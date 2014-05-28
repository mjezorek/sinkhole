#ifndef IO_H
#define IO_H

namespace Sinkhole
{
	class SocketException : public Exception
	{
	 public:
		SocketException(const std::string &r) : Exception(r) { }
	};

	enum SocketFlag
	{
		SF_DEAD = 1,
		SF_WANT_READ = 2,
		SF_WANT_WRITE = 4
	};

	class Socket
	{
		friend class SocketEngine;

	 protected:
		/* Socket data */
		int fd;
		sockaddrs source_info, peer_info;

	 public:
		short flags;

		/** Constructor
		 * @param fd The already existing fd this socket should represent.
		 */
 		Socket(int sfd);

		/** Constructor
		 * @param domain AF_INET/AF_INET6
		 * @param stype SOCK_STREAM etc
		 */
		Socket(int sdomain, int stype);

		/** Default destructor
		 */
		virtual ~Socket();

		/** Get a sockaddr representing the source of the connection (us)
		 * @return A sockaddr structure
		 */
		sockaddrs &GetSock();

		/** Get a sockaddr represending the peer of this connection
		 * @return A sockaddr structure
		 */
		sockaddrs &GetPeer();

		/** Called when there is something to be received for this socket
		 * @return true on success, false to drop this socket
		 */
		virtual bool ProcessRead();

		/** Called when the socket is ready to be written to
		 * @return true on success, false to drop this socket
		 */
		virtual bool ProcessWrite();
		
		/** Called when the socket has an error
		 */
		virtual void ProcessError();
	};

	class Listener : public Socket
	{
	 public:
		Listener(const std::string &bindip, int port, int type, int stype = SOCK_STREAM);

		bool ProcessRead();

		virtual Socket *OnAccept(int fd);
	};

	class SocketEngine
	{
		/* Map of sockets */
		static std::map<int, Socket *> sockets;

	 public:
		static void Init();

		static void Shutdown();

		static void AddSocket(Socket *s);

		static void DelSocket(Socket *s);

		static void ModifySocket(Socket *s, short flags);

		static void Process();
	};
}

#endif // IO_H
