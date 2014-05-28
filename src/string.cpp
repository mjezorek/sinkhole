#include "sinkhole.hpp"
#include "string.hpp"
#include <stdarg.h>

using namespace Sinkhole;

sepstream::sepstream(const std::string &source, char seperator) : tokens(source), sep(seperator)
{
	last_starting_position = n = tokens.begin();
}

bool sepstream::GetToken(std::string &token)
{
	std::string::iterator lsp = last_starting_position;

	while (n != tokens.end())
	{
		if (*n == sep || n + 1 == tokens.end())
		{
			last_starting_position = n + 1;
			token = std::string(lsp, n + 1 == tokens.end() ? n + 1 : n);

			while (token.length() && token.rfind(sep) == token.length() - 1)
				token.erase(token.end() - 1);

			++n;

			return true;
		}

		++n;
	}

	token.clear();
	return false;
}

const std::string sepstream::GetRemaining()
{
	return std::string(n, tokens.end());
}

bool sepstream::StreamEnd()
{
	return n == tokens.end();
}

void Sinkhole::strip(std::string &string)
{
	while (!string.empty() && isspace(string[0]))
		string.erase(string.begin());
	while (!string.empty() && isspace(string[string.length() - 1]))
		string.erase(string.length() - 1);
}

bool less_ci::operator()(const std::string &s1, const std::string &s2) const
{
	size_t min = s1.length() < s2.length() ? s1.length() : s2.length();
	for (size_t i = 0; i < min; ++i)
	{
		register char c1 = tolower(s1[i]), c2 = tolower(s2[i]);
		if (c1 > c2)
			return false;
		else if (c1 < c2)
			return true;
	}

	return s1.length() < s2.length();
}

bool Sinkhole::match(const std::string &str, const std::string &pattern, bool case_sensitive)
{
	size_t s = 0, m = 0, str_len = str.length(), pattern_len = pattern.length();

	while (s < str_len && m < pattern_len && pattern[m] != '*')
	{
		char string = str[s], wild = pattern[m];
		if (case_sensitive)
		{
			if (wild != string && wild != '?')
				return false;
		}
		else
		{
			if (tolower(wild) != tolower(string) && wild != '?')
				return false;
		}

		++m;
		++s;
	}

	size_t sp = std::string::npos, mp = std::string::npos;
	while (s < str_len)
	{
		char string = str[s], wild = pattern[m];
		if (wild == '*')
		{
			if (++m == pattern_len)
				return 1;

			mp = m;
			sp = s + 1;
		}
		else if (case_sensitive)
		{
			if (wild == string || wild == '?')
			{
				++m;
				++s;
			}
			else
			{
				m = mp;
				s = sp++;
			}
		}
		else
		{
			if (tolower(wild) == tolower(string) || wild == '?')
			{
				++m;
				++s;
			}
			else
			{
				m = mp;
				s = sp++;
			}
		}
	}

	if (pattern[m] == '*')
		++m;

	return m == pattern_len;
}

std::string Sinkhole::printf(const char *format, ...)
{
	char format_buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(format_buffer, sizeof(format_buffer), format, args);
	va_end(args);

	return format_buffer;
}

std::string Sinkhole::replace(const std::string &string, const std::string &targ, const std::string &what)
{
	std::string new_string = string;
	std::string::size_type pos = new_string.find(targ), orig_length = targ.length(), repl_length = what.length();
	while (pos != std::string::npos)
	{
		new_string = new_string.substr(0, pos) + what + new_string.substr(pos + orig_length);
		pos = new_string.find(targ, pos + repl_length);
	}
	return new_string;
}

