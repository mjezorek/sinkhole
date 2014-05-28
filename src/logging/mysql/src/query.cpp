#include "sinkhole.hpp"
#include "string.hpp"
#include "threads.hpp"
#include "include/mysql.hpp"
#include "include/query.hpp"

using namespace Sinkhole::Logging::MySQL;

Query::Query(const std::string &q) : query(q)
{
}

bool Query::operator==(const Query &other) const
{
	return this->query == other.query;
}

void Query::setString(unsigned index, const std::string &value)
{
	this->parameters[index] = value;
}

void Query::setInteger(unsigned index, int value)
{
	this->parameters[index] = Sinkhole::stringify(value);
}

void Query::unset(unsigned index)
{
	this->parameters.erase(index);
}

std::string Query::BuildQuery(MySQLConnection *con) const
{
	std::string tquery;

	for (unsigned i = 0, j = this->query.length(), c = 0; i < j; ++i)
	{
		if (this->query[i] != '?')
			tquery += this->query[i];
		else
		{
			std::map<unsigned, std::string>::const_iterator it = this->parameters.find(c++);
			if (it == this->parameters.end())
				throw MySQLException("Not enough parameters set");
			tquery += "'";
			tquery += con->Escape(it->second);
			tquery += "'";
		}
	}

	return tquery;
}

