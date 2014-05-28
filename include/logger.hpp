#ifndef LOGGER_H
#define LOGGER_H

namespace Sinkhole
{
	enum LogLevel
	{
		LOG_ERROR = 3,
		LOG_NORMAL = -1,
		LOG_INFORMATIONAL = 6,
		LOG_DEBUG = 7
	};

	class Log
	{
		LogLevel level;
		std::string module;
		std::string source;
		std::string action;
		std::stringstream buf;
	 public:
		Log(LogLevel l);
		Log(const std::string &m, const std::string &s, const std::string &a);
		~Log();

		template<typename T> Log &operator<<(const T &val)
		{
			this->buf << val;
			return *this;
		}
	};
}

#endif // LOGGER_H

