#include "sinkhole.hpp"
#include "module.hpp"
#include "string.hpp"
#include "timers.hpp"
#include "include/flatfile.hpp"

using namespace Sinkhole::Modules;
using namespace Sinkhole::Logging::Flatfile;

WriteRequest::WriteRequest(FlatfileConfiguration *f, const std::string &b) : ffconf(f), buffer(b)
{
	this->buffer += "\n";
	this->aio_fildes = f->fd;
	this->aio_lio_opcode = LIO_WRITE;
	this->aio_reqprio = 0;
	this->aio_buf = const_cast<char *>(this->buffer.c_str());
	this->aio_nbytes = this->buffer.length();
	memset(&this->aio_sigevent, 0, sizeof(this->aio_sigevent));
}

void WriteRequest::Dispatch()
{
	if (aio_write(this) == -1)
		throw Sinkhole::Exception(Sinkhole::LastError());
}

