#ifndef NETWORK_H
#define NETWORK_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace Sinkhole
{
	/** Retrieve a pointer to the network buffer for recv() etc to read into.
	 * @param The pointer and the size it points to
	 */
	extern std::pair<char *, size_t> NetBuffer();

	/** A sockaddr union used to combine IPv4 and IPv6 sockaddrs
	 */
	union sockaddrs
	{
		sockaddr sa;
		sockaddr_in sa4;
		sockaddr_in6 sa6;
		sockaddr_un un;

		/** Construct the object, sets everything to 0
		 */
		sockaddrs();

		/** Memset the object to 0
		 */
		void clear();

		/** Get the size of the sockaddr we represent
		 * @return The size
		 */
		size_t size() const;

		/** Get the port represented by this addr
		 * @return The port, or -1 on fail
		 */
		int port() const;

		/** Get the address represented by this addr
		 * @return The address
		 */
		std::string addr() const;

		/** Check if this sockaddr has data in it
		 */
		bool operator()() const;

		/** Compares this sockaddr with another. Compares address type, port, and address
		 * @return true if they are the same
		 */
		bool operator==(const sockaddrs &other) const;
		/* The same as above but not */
		inline bool operator!=(const sockaddrs &other) const { return !(*this == other); }

		/** Compares this sockaddr with another.
		 * @return true if this is less than the other
		 */
		bool operator<(const sockaddrs &other) const;

		/** The equivalent of inet_pton
		 * @param type AF_INET or AF_INET6
		 * @param address The address to place in the sockaddr structures
		 * @param pport An option port to include in the  sockaddr structures
		 * @throws A socket exception if given invalid IPs
		 */
		void pton(int type, const std::string &address, int pport = 0);

		/** The equivalent of inet_ntop
		 * @param type AF_INET or AF_INET6
		 * @param address The in_addr or in_addr6 structure
		 * @throws A socket exception if given an invalid structure
		 */
		void ntop(int type, const void *src);
	};

	class cidr
	{
		static const unsigned char AF_INET_LEN;
		static const unsigned char AF_INET6_LEN;
		sockaddrs addr;
		std::string cidr_ip;
		unsigned char cidr_len;
	 public:
 		cidr(const std::string &ip);
		cidr(const std::string &ip, unsigned char len);
		std::string mask() const;
		bool match(sockaddrs &other);
		bool operator==(const cidr &other) const;
		/* The same as above but not */
		inline bool operator!=(const cidr &other) const { return !(*this == other); }
		bool operator<(const cidr &other) const;
	};
}

#endif

