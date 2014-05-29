#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "string.hpp"
#include "include/http.hpp"
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

using namespace Sinkhole::Protocol::HTTP;

HTTPRequest::HTTPRequest(ClientSocket *c, const std::string &d) : con(c)
{
	this->aio = NULL;

	std::vector<std::string> data;
	{
		sepstream sep(d, '\n');
		std::string buf;
		while (sep.GetToken(buf))
		{
			Sinkhole::strip(buf);
			data.push_back(buf);
		}
	}

	if (data.empty())
		throw HTTPException("Malformed request");
	
	{
		std::vector<std::string> tokens;
		{
			sepstream sep(data[0], ' ');
			std::string buf;
			while (sep.GetToken(buf))
				tokens.push_back(buf);
		}
		if (tokens.size() < 3)
			throw HTTPException("Malfored request");

		if (tokens[0] == "GET")
			this->method = HTTP_METHOD_GET;
		else if (tokens[0] == "POST")
			this->method = HTTP_METHOD_POST;
		else
			throw HTTPException("Unknown method " + tokens[0]);
	
		this->path = tokens[1];

		if (tokens[2] == "HTTP/1.0")
			this->protocol = HTTP_PROTO_10;
		else if (tokens[2] == "HTTP/1.1")
			this->protocol = HTTP_PROTO_11;
		else
			throw HTTPException("Unknown protocol " + tokens[2]);

		Sinkhole::Log(this->con->server->name, this->con->GetIP(), "REQUEST") << data[0];
	}

	for (unsigned i = 1, j = data.size(); i < j; ++i)
	{
		size_t colon = data[i].find(':');
		if (data.empty() || colon == std::string::npos)
			break;

		std::string key = data[i].substr(0, colon);
		std::string value = data[i].substr(colon + 2);

		if (key == "Host")
			Sinkhole::Log(this->con->server->name, this->con->GetIP(), "HOST") << value;
		else if (key == "User-Agent")
			Sinkhole::Log(this->con->server->name, this->con->GetIP(), "USERAGENT") << value;
		else if (key == "Referer") 
			Sinkhole::Log(this->con->server->name, this->con->GetIP(), "REFERER") << value;

		this->attributes[key] = value;
	}
}

HTTPRequest::~HTTPRequest()
{
	if (this->aio)
		this->aio->Cancel();
}

void HTTPRequest::Read(const std::string &)
{
}

void HTTPRequest::Process()
{
	HTTPHeader reply;
	int file = -1;
	reply.SetCode(HTTP_CODE_INTERNAL_ERROR);
	reply.SetProtocol(this->protocol);

	HTTPAction *a = this->con->connectclass->action;
	HTTPVhost *v = a;
	for (unsigned i = 0, j = a->vhosts.size(); i < j; ++i)
	{
		HTTPVhost &vhost = a->vhosts[i];
		if (vhost.name == this->path)
		{
			v = &vhost;
			break;
		}
	}

	if (v->type == HTTPVhost::TYPE_SERVE)
	{
		struct stat s;

		file = open(v->data.c_str(), O_RDONLY | O_NOATIME | O_NONBLOCK);
		if (file == -1 || fstat(file, &s) == -1)
		{
			if (errno != EACCES)
				reply.SetCode(HTTP_CODE_NOT_FOUND);
			else
				reply.SetCode(HTTP_CODE_FORBIDDEN);
		}
		else if (!S_ISREG(s.st_mode))
			reply.SetCode(HTTP_CODE_FORBIDDEN);
		else
		{
			reply.SetCode(HTTP_CODE_OK);
			reply.SetAttribute("X-Malware-Sinkhole","malware sinkhole");
			reply.SetAttribute("Content-Length", stringify(s.st_size));
			this->aio = new AIO(this->con, file);
		}
	}
	else if (v->type == HTTPVhost::TYPE_ROOT)
	{
		struct stat s;
		std::string want_path = v->data + "/" + Sinkhole::replace(this->path, "..", "");

		file = open(want_path.c_str(), O_RDONLY | O_NOATIME | O_NONBLOCK);
		if (file == -1 || fstat(file, &s) == -1)
		{
			if (errno != EACCES)
				reply.SetCode(HTTP_CODE_NOT_FOUND);
			else
				reply.SetCode(HTTP_CODE_FORBIDDEN);
		}
		else if (!S_ISREG(s.st_mode))
			reply.SetCode(HTTP_CODE_FORBIDDEN);
		else
		{
			reply.SetCode(HTTP_CODE_OK);
			reply.SetAttribute("Content-Length", stringify(s.st_size));
			this->aio = new AIO(this->con, file);
		}
	}

	reply.SetAttribute("Server", a->server_name);
	reply.SetAttribute("Connection", "close");
	reply.SetAttribute("Content-Type", "text/html");

	std::string header_data = reply.ToString();
	this->con->Write(header_data.c_str(), header_data.length() + 1);

	if (this->aio == NULL)
	{
		if (file != -1)
			close(file);
		this->con->flags |= SF_DEAD;
	}
}

