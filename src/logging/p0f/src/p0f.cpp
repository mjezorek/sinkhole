#include "sinkhole.hpp"
#include "conf.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "../p0f/include/p0f-query.h"
#include <errno.h>

using namespace Sinkhole::Modules;

class P0fSocket : public Sinkhole::Socket
{
	std::string sock;
	Sinkhole::sockaddrs source, dest;
	bool connected;

	bool SendQuery()
	{
		p0f_query p;
		p.magic = QUERY_MAGIC;
		p.id = 0;
		p.type = QTYPE_FINGERPRINT;
		p.src_ad = this->source.sa4.sin_addr.s_addr;
		p.dst_ad = this->dest.sa4.sin_addr.s_addr;
		p.src_port = this->source.port();
		p.dst_port = this->dest.port();

		int len = send(this->fd, &p, sizeof(p), 0);
		if (len == sizeof(p))
			return true;
		return false;
	}

 public:
	P0fSocket(const std::string &s, const Sinkhole::sockaddrs &so, const Sinkhole::sockaddrs &de) : Sinkhole::Socket(AF_UNIX, SOCK_STREAM), sock(s), source(so), dest(de)
	{
		Sinkhole::sockaddrs addr;
		addr.un.sun_family = AF_UNIX;
		strncpy(addr.un.sun_path, this->sock.c_str(), sizeof(addr.un.sun_path));
		addr.un.sun_path[sizeof(addr.un.sun_path) - 1] = 0;

		this->connected = !connect(this->fd, &addr.sa, addr.size());
		if (!this->connected && errno != EINPROGRESS)
			throw Sinkhole::SocketException(Sinkhole::LastError());

		Sinkhole::SocketEngine::ModifySocket(this, this->flags | Sinkhole::SF_WANT_WRITE);
	}

	bool ProcessRead()
	{
		p0f_response r;
		int len = recv(this->fd, &r, sizeof(r), 0);
		if (len != sizeof(r))
			return false;

		if (r.magic != QUERY_MAGIC || r.type == RESP_BADQUERY)
			return false;
		else if (r.type == RESP_NOMATCH || !r.genre[0])
			return false;

		Sinkhole::Log("p0f", this->source.addr(), "CONNECT") << r.genre << " " << r.detail;

		return false;
	}

	bool ProcessWrite()
	{
		if (!this->connected)
		{
			int optval = 0;
			socklen_t optlen = sizeof(optval);
			if (!getsockopt(this->fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) && !optval)
			{
				this->connected = true;
				return true;
			}

			return false;
		}

		Sinkhole::SocketEngine::ModifySocket(this, this->flags & ~Sinkhole::SF_WANT_WRITE);
		return this->SendQuery();
	}
};

class P0fModule : public Module
{
	std::string socket;

 public:
	P0fModule(const std::string &modname) : Module(modname)
	{
		try
		{
			this->socket = Sinkhole::Config->GetBlock("p0f").GetValue("socket");
		}
		catch (const Sinkhole::ConfigException &) { }
	}

	void OnClientAccept(Sinkhole::Socket *sock)
	{
		if (this->socket.empty())
			return;

		try
		{
			Sinkhole::sockaddrs &peer = sock->GetPeer();
			if (peer.sa.sa_family != AF_INET)
				return;

			new P0fSocket(this->socket, peer, sock->GetSock());
		}
		catch (const Sinkhole::SocketException &ex)
		{
			Sinkhole::Log(Sinkhole::LOG_DEBUG) << "Unable to connect to p0f: " << ex.GetReason();
		}
	}
};

MODULE_INIT(P0fModule)

