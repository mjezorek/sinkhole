#include "sinkhole.hpp"
#include "conf.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "network.hpp"
#include "io.hpp"
#include "timers.hpp"
#include "include/dns.hpp"
#include "include/class.hpp"
#include "include/datasocket.hpp"
#include "include/request.hpp"

using namespace Sinkhole::Modules;
using namespace Sinkhole::Protocol::DNS;

std::vector<DNSServer *> Sinkhole::Protocol::DNS::servers;

Sinkhole::sockaddrs Sinkhole::Protocol::DNS::resolver;
int Sinkhole::Protocol::DNS::resolver_timeout = 5;

bool Sinkhole::Protocol::DNS::resolve_clients = false;

class ClientResolver : public DNSRequest
{
 public:
	ClientResolver(const std::string &h, QueryType t) : DNSRequest(h, t)
	{
	}

	void OnResult(const DNSPacket &p)
	{
		if (p.answers.empty())
			return;

		const ResourceRecord &rr = p.answers[0];

		Sinkhole::Log("dns/resolver", rr.name, "RESOLVE") << rr.rdata;
	}
};

class ProtocolDNS : public Module
{
 public:
	ProtocolDNS(const std::string &modname) : Module(modname)
	{
		if (Sinkhole::Config->CountBlock("resolver") > 0)
		{
			Sinkhole::ConfigurationBlock &r = Sinkhole::Config->GetBlock("resolver", 0);

			try
			{
				std::string resolver_host = r.GetValue("nameserver");
				int resolver_port = atoi(r.GetValue("port").c_str());
				bool resolver_ipv6 = r.GetBool("ipv6");
				resolver.pton(resolver_ipv6 ? AF_INET6 : AF_INET, resolver_host, resolver_port);

				resolver_timeout = atoi(r.GetValue("timeout").c_str());

				resolve_clients = r.GetBool("resolve_clients");
			}
			catch (const Sinkhole::SocketException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading resolver block: " << ex.GetReason();
			}
			catch (const Sinkhole::ConfigException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading resolver block: " << ex.GetReason();
			}
		}

		for (int i = 0, j = Sinkhole::Config->CountBlock("dns"); i < j; ++i)
		{
			Sinkhole::ConfigurationBlock &b = Sinkhole::Config->GetBlock("dns", i);

			try
			{
				std::string name = b.GetValue("name");

				DNSServer *s = DNSServer::Find(name);
				if (s == NULL)
				{
					s = new DNSServer(name);
					Sinkhole::Log(Sinkhole::LOG_INFORMATIONAL) << "DNS server " << name << " starting up";
				}

				for (int k = 0, l = b.CountBlock("listen"); k < l; ++k)
				{
					Sinkhole::ConfigurationBlock &lb = b.GetBlock("listen", k);

					try
					{
						std::string addr = lb.GetValue("addr");
						int port = atoi(lb.GetValue("port").c_str());
						if (port < 0 || port > 65535)
							throw Sinkhole::ConfigException("Invalid port");
						bool ipv6 = lb.GetBool("ipv6");

						s->listeners.push_back(new DNSDataSocket(s, addr, port, ipv6));
					}
					catch (const Sinkhole::SocketException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error binding to address " << lb.GetValue("addr") << ":" << lb.GetValue("port") << ": " << ex.GetReason();
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading listen block number " << k << " for " << name << ": " << ex.GetReason();
					}
				}

				for (int k = 0, l = b.CountBlock("class"); k < l; ++k)
				{
					Sinkhole::ConfigurationBlock &cb = b.GetBlock("class", k);

					try
					{
						DNSClass dnsclass;

						for (int m = 0, n = cb.CountBlock("source"); m < n; ++m)
						{
							try
							{
								Sinkhole::ConfigurationBlock &cbs = cb.GetBlock("source", m);
								Sinkhole::cidr range(cbs.GetValue("addr"));
								dnsclass.sources.push_back(range);
							}
							catch (const Sinkhole::ConfigException &ex)
							{
								Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading source block number " << m << " for class " << k << "for " << name << ": " << ex.GetReason();
							}
						}

						for (int m = 0, n = cb.CountBlock("zone"); m < n; ++m)
						{
							Sinkhole::ConfigurationBlock &zb = cb.GetBlock("zone", m);

							try
							{
								std::string zname = zb.GetValue("name");

								Sinkhole::ConfigurationBlock &soa = zb.GetBlock("soa");
								std::string mname = soa.GetValue("mname");
								std::string rname = soa.GetValue("rname");
								unsigned int serial = atol(soa.GetValue("serial").c_str());
								unsigned int refresh = atol(soa.GetValue("refresh").c_str());
								unsigned int retry = atol(soa.GetValue("retry").c_str());
								unsigned int expire = atol(soa.GetValue("expire").c_str());
								unsigned int minimum = atol(soa.GetValue("minimum").c_str());

								DNSZone zone;

								zone.name = zname;
								zone.soa.mname = mname;
								zone.soa.rname = rname;
								zone.soa.serial = serial;
								zone.soa.refresh = refresh;
								zone.soa.retry = retry;
								zone.soa.expire = expire;
								zone.soa.minimum = minimum;

								for (int o = 0, p = zb.CountBlock("record"); o < p; ++o)
								{
									Sinkhole::ConfigurationBlock &record = zb.GetBlock("record", o);

									try
									{
										std::string rname = record.GetValue("name");
										unsigned int record_ttl = refresh;
										try { record_ttl = atoi(record.GetValue("ttl").c_str()); }
										catch (const Sinkhole::ConfigException &) { }
										std::string rclass = record.GetValue("class");
										std::string host = record.GetValue("host");

										QueryType type;
										if (rclass == "A")
											type = DNS_QUERY_A;
										else if (rclass == "AAAA")
											type = DNS_QUERY_AAAA;
										else if (rclass == "CNAME")
											type = DNS_QUERY_CNAME;
										else if (rclass == "PTR")
											type = DNS_QUERY_PTR;
										else if (rclass == "NS")
											type = DNS_QUERY_NS;
										else
											throw Sinkhole::ConfigException("Invalid query type " + rclass);

										DNSRecord record;
										record.type = type;
										record.result = host;
										record.ttl = record_ttl;

										zone.AddRecord(rname, record);
									}
									catch (const Sinkhole::ConfigException &ex)
									{
										Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading record block number " << o << " in class " << k <<  " for " << name << ": " << ex.GetReason();
									}
								}

								dnsclass.zones.push_back(zone);
							}
							catch (const Sinkhole::ConfigException &ex)
							{
								Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading zone block number " << m << " in class " << n << " for " << name << ": " << ex.GetReason();
							}
						}

						s->classes.push_back(dnsclass);
					}
					catch (const Sinkhole::ConfigException &ex)
					{
						Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading class block number " << k << " for " << name << ": " << ex.GetReason();
					}
				}
			}
			catch (const Sinkhole::ConfigException &ex)
			{
				Sinkhole::Log(Sinkhole::LOG_ERROR) << "Error loading dns block " << i << ": " << ex.GetReason();
			}
		}
	}

	~ProtocolDNS()
	{
		for (unsigned i = servers.size(); i > 0; --i)
			delete servers[i - 1];
	}

	void OnClientAccept(Sinkhole::Socket *sock)
	{
		if (!resolver() || resolve_clients == false)
			return;

		Sinkhole::sockaddrs &client_addr = sock->GetSock();
		
		ClientResolver *request = new ClientResolver(client_addr.addr(), DNS_QUERY_PTR);
		request->Dispatch();
	}
};

MODULE_INIT(ProtocolDNS)
