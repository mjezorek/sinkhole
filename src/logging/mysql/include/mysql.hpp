#ifndef MYSQL_MYSQL_H
#define MYSQL_MYSQL_H

#define NO_CLIENT_LONG_LONG
#include <mysql/mysql.h>

#define MYSQL_NAMESPACE_BEGIN namespace Sinkhole { namespace Logging { namespace MySQL {
#define MYSQL_NAMESPACE_END } } }

MYSQL_NAMESPACE_BEGIN

class MySQLException : public Exception
{
 public:
	MySQLException(const std::string &r) : Exception(r) { }
};

class Query;

class MySQLConnection : public Sinkhole::Thread, public Sinkhole::Condition
{
	std::string server;
	int port;
	std::string database;
	std::string username;
	std::string password;

 public:
	std::string name;
	MYSQL *sql;
	Sinkhole::Mutex SQLMutex;
	std::deque<Query> QueryRequests;

	static MySQLConnection *Find(const std::string &name);

	MySQLConnection(const std::string &n, const std::string &s, int p, const std::string &d, const std::string &u, const std::string &pa);
	~MySQLConnection();
	
	void Connect();

	std::string Escape(const std::string &value);
	void Execute(Query &query);
	void ExecuteBlocking(Query &query);

	void Run();
};

extern std::vector<MySQLConnection *> connections;

MYSQL_NAMESPACE_END

#endif // MYSQL_MYSQL_H

