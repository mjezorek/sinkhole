#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "include/http.hpp"

using namespace Sinkhole::Protocol::HTTP;

struct CodeDescription
{
	HTTPCode code;
	std::string desc;
} CodeDescriptions[] = {
	{ HTTP_CODE_OK, "OK" },
	{ HTTP_CODE_BAD_REQUEST, "Bad Request" },
	{ HTTP_CODE_UNAUTHORIZED, "Unauthorized" },
	{ HTTP_CODE_FORBIDDEN, "Forbidden" },
	{ HTTP_CODE_NOT_FOUND, "Not Found" },
	{ HTTP_CODE_INTERNAL_ERROR, "Internal Error" },
	{ HTTP_CODE_INTERNAL_ERROR, "" }
};

HTTPHeader::HTTPHeader()
{
	this->code = HTTP_CODE_INTERNAL_ERROR;
	this->protocol = HTTP_PROTO_11;
}

void HTTPHeader::SetCode(HTTPCode c)
{
	this->code = c;
}

void HTTPHeader::SetProtocol(HTTPProtocol p)
{
	this->protocol = p;
}

void HTTPHeader::SetAttribute(const std::string &attr, const std::string &value)
{
	this->attributes[attr] = value;
}

std::string HTTPHeader::ToString()
{
	std::string ret;
	
	if (this->protocol == HTTP_PROTO_10)
		ret += "HTTP/1.0";
	else if (this->protocol == HTTP_PROTO_11)	
		ret += "HTTP/1.1";
	else
		throw HTTPException("Unrecognized protocol");
	
	ret += " ";

	ret += Sinkhole::stringify(this->code);

	ret += " ";
	
	for (int i = 0; !CodeDescriptions[i].desc.empty(); ++i)
		if (CodeDescriptions[i].code == this->code)
		{
			ret += CodeDescriptions[i].desc;
			break;
		}
	
	ret += "\r\n";

	for (std::map<std::string, std::string>::iterator it = this->attributes.begin(), it_end = this->attributes.end(); it != it_end; ++it)
		ret += it->first + ": " + it->second + "\r\n";
	
	ret += "\r\n";
	
	return ret;
}

