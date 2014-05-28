#include "sinkhole.hpp"
#include "conf.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "threads.hpp"
#include "include/mysql.hpp"
#include "include/query.hpp"

using namespace Sinkhole::Modules;
using namespace Sinkhole::Logging::MySQL;

std::vector<MySQLConnection *> Sinkhole::Logging::MySQL::connections;

MySQLConnection *MySQLConnection::Find(const std::string &name)
{
	for (unsigned j = connections.size(); j > 0; --j)
		if (connections[j - 1]->name == name)
			return connections[j - 1];
	return NULL;
}

MySQLConnection::MySQLConnection(const std::string &n, const std::string &s, int p, const std::string &d, const std::string &u, const std::string &pa)
 : server(s), port(p), database(d), username(u), password(pa), name(n)
{
	this->sql = NULL;
	this->Connect();
	ThreadEngine::Start(this);

	connections.push_back(this);
}

MySQLConnection::~MySQLConnection()
{
	this->SetExitState();
	this->Wakeup();
	this->Join();

	if (this->sql != NULL)
	{
		this->SQLMutex.Lock();
		mysql_close(this->sql);
		this->SQLMutex.Unlock();
	}
	
	std::vector<MySQLConnection *>::iterator it = std::find(connections.begin(), connections.end(), this);
	if (it != connections.end())
		connections.erase(it);
}

void MySQLConnection::Connect()
{
	this->SQLMutex.Lock();
	this->sql = mysql_init(sql);

	const unsigned int timeout = 1;
	mysql_options(this->sql, MYSQL_OPT_CONNECT_TIMEOUT, reinterpret_cast<const char *>(&timeout));

	bool connect = mysql_real_connect(this->sql, this->server.c_str(), this->username.c_str(), this->password.c_str(), this->database.c_str(), this->port, NULL, 0);

	std::string error = !connect ? mysql_error(this->sql) : "";
	this->SQLMutex.Unlock();

	if (!error.empty())
		throw MySQLException("Unable to connect to MySQL: " + error);
}

std::string MySQLConnection::Escape(const std::string &value)
{
	char buffer[1024];
	mysql_real_escape_string(this->sql, buffer, value.c_str(), value.length());
	return buffer;
}

void MySQLConnection::Execute(Query &query)
{
	this->Lock();
	this->QueryRequests.push_back(query);
	this->Unlock();
	this->Wakeup();
}

void MySQLConnection::ExecuteBlocking(Query &query)
{
	std::string query_string, error;

	this->SQLMutex.Lock();

	try
	{
		query_string = query.BuildQuery(this);
	}
	catch (const MySQLException &ex)
	{
		error = ex.GetReason();
	}
	if (error.empty())
	{
		try
		{
			if (!this->sql || mysql_ping(this->sql))
				this->Connect();
		}
		catch (const MySQLException &ex)
		{
			error = ex.GetReason();
		}
		if (error.empty())
			if (mysql_real_query(this->sql, query_string.c_str(), query_string.length()))
				error = mysql_error(sql);
	}

	this->SQLMutex.Unlock();

	if (!error.empty())
	{
		Sinkhole::Log(Sinkhole::LOG_ERROR) << error;
		throw MySQLException(error);
	}
}

void MySQLConnection::Run()
{
	this->Lock();

	while (!this->GetExitState())
	{
		if (!QueryRequests.empty())
		{
			Query &q = QueryRequests.front();
			this->Unlock();

			try
			{
				this->ExecuteBlocking(q);
			}
			catch (const MySQLException &) { }

			this->Lock();
			if (!QueryRequests.empty() && QueryRequests.front() == q)
				QueryRequests.pop_front();
		}
		else
		{
			this->Wait();
		}
	}

	this->Unlock();
}

class LoggingMySQL : public Module
{
	std::set<std::string> existingTables;

 public:
	LoggingMySQL(const std::string &modname) : Module(modname)
	{
		for (int i = 0, j = Sinkhole::Config->CountBlock("mysql"); i < j; ++i)
		{
			Sinkhole::ConfigurationBlock &b = Sinkhole::Config->GetBlock("mysql", i);

			try
			{
				std::string name = b.GetValue("name");
				std::string server = b.GetValue("server");
				int port = atoi(b.GetValue("port").c_str());
				if (port <= 0 || port > 65535)
					port = 3306;
				std::string database = b.GetValue("database");
				std::string username = b.GetValue("username");
				std::string password = b.GetValue("password");

				new MySQLConnection(name, server, port, database, username, password);
			}
			catch (const Sinkhole::ConfigException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading mysql block: " << ex.GetReason();
			}
			catch (const MySQLException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Unable to connect to SQL server " << b.GetValue("name") << ": " << ex.GetReason();
			}
		}
	}

	~LoggingMySQL()
	{
		for (unsigned j = connections.size(); j > 0; --j)
			delete connections[j - 1];
		connections.clear();
	}

	void OnLog(const std::string &target, const std::string &module, const std::string &source, const std::string &action, const std::string &data)
	{
		for (unsigned j = connections.size(); j > 0; --j)
		{
			MySQLConnection *con = connections[j - 1];

			if (con->name == target)
			{
				size_t sl = module.find('/');
				std::string transport = module.substr(0, sl != std::string::npos ? sl : module.length());
				if (this->existingTables.find(transport) == this->existingTables.end())
				{
					this->existingTables.insert(transport);
					Query q("CREATE TABLE IF NOT EXISTS `" + transport + "` (`time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,"
							"`source` varchar(255), `action` varchar(255), `data` text)");
					con->Execute(q);
				}

				Query q("INSERT INTO `" + transport + "` (source, action, data) VALUES(?, ?, ?)");
				q.setString(0, source);
				q.setString(1, action);
				q.setString(2, data);

				con->Execute(q);
				break;
			}
		}
	}
};

MODULE_INIT(LoggingMySQL)

