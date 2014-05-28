#ifndef STRING_H
#define STRING_H

#include <string.h>

namespace Sinkhole
{
	/** sepstream allows for splitting token seperated lists.
	 * Each successive call to sepstream::GetToken() returns
	 * the next token, until none remain, at which point the method returns
	 * an empty string.
	 */
	class sepstream
	{
	 private:
		/** Original string.
		 */
		std::string tokens;
		/** Last position of a seperator token
		 */
		std::string::iterator last_starting_position;
		/** Current string position
		 */
		std::string::iterator n;
		/** Seperator value
		 */
		char sep;
	 public:
		/** Create a sepstream and fill it with the provided data
		 */
		sepstream(const std::string &source, char seperator);
		virtual ~sepstream() { }

		/** Fetch the next token from the stream
		 * @param token The next token from the stream is placed here
		 * @return True if tokens still remain, false if there are none left
		 */
		virtual bool GetToken(std::string &token);

		/** Fetch the entire remaining stream, without tokenizing
		 * @return The remaining part of the stream
		 */
		virtual const std::string GetRemaining();

		/** Returns true if the end of the stream has been reached
		 * @return True if the end of the stream has been reached, otherwise false
		 */
		virtual bool StreamEnd();
	};

	template<typename T> std::string stringify(const T &x)
	{
		std::ostringstream stream;
		stream << x;
		return stream.str();
	}

	/** Remove beginning and trailing whitespaces from the string
	 * @param The string
	 */
	void strip(std::string &string);

	class less_ci
	{
	 public:
		/** Compare two std::strings with case insensitivity
		 * @param s1 The first string
		 * @param s2 The second string
		 * @return true if s1 < s2, else false
		 */
		bool operator()(const std::string &s1, const std::string &s2) const;
	};

	/** Perform a wildcard match.
	 * @param str The string 
	 * @param pattern The pattern to match against
	 * @param case_sensitive true to match with case sensitivity
	 * @return true on a successful match
	 */
	bool match(const std::string &str, const std::string &pattern, bool case_sensitive = false);

	/** Creates a string from a printf() format
	 * @param format The format
	 * @return The formatted string
	 */
	std::string printf(const char *format, ...);

	/** Replace all of targ in string with what
	 * @param string The original string
	 * @param targ Substring to be replaced
	 * @param what What to replace targ with
	 * @return The new string
	 */
	std::string replace(const std::string &string, const std::string &targ, const std::string &what);
}

#endif // STRING_H

