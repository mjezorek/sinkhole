#ifndef MYSQL_QUERY_H
#define MYSQL_QUERY_H

MYSQL_NAMESPACE_BEGIN

class Query
{
	std::string query;
	std::map<unsigned, std::string> parameters;

 public:
	Query(const std::string &q);
	bool operator==(const Query &other) const;
	void setString(unsigned index, const std::string &value);
	void setInteger(unsigned index, int value);
	void unset(unsigned index);
	std::string BuildQuery(MySQLConnection *con) const;
};

MYSQL_NAMESPACE_END

#endif // MYSQL_QUERY_H

