#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "threads.hpp"
#include "pipe.hpp"
#include "include/http.hpp"

#include <aio.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
using namespace Sinkhole::Protocol::HTTP;

static std::set<AIO *> requests;

class SigUSR1 : public Sinkhole::Signal
{
 public:
	SigUSR1() : Signal(SIGUSR1)
	{
	}

	void OnSignal()
	{
		for (std::set<AIO *>::iterator it = requests.begin(), it_end = requests.end(); it != it_end;)
		{
			AIO *a = *it;
			++it;

			int err = aio_error(a);
			if (err == EINPROGRESS)	
				continue;
			else if (err == ECANCELED)
				delete a;
			else if (err == 0)
			{
				int len = aio_return(a);
				if (len > 0)
				{
					a->aio_offset += len;
					a->OnRead(a->read_buffer, len);
					aio_read(a);
				}
				else if (len == -1)
					a->OnError();
				else
					delete a;
			}
		}
	}
} aio_sighandler;

AIO::AIO(ClientSocket *c, int fd)
{
	this->con = c;
	this->aio_fildes = fd;
	this->aio_offset = 0;
	this->aio_lio_opcode = LIO_READ;
	this->aio_reqprio = 0;
	this->aio_buf = this->read_buffer;
	this->aio_nbytes = sizeof(this->read_buffer);
	memset(&this->aio_sigevent, 0, sizeof(this->aio_sigevent));
	this->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	this->aio_sigevent.sigev_signo = SIGUSR1;

	aio_read(this);

	requests.insert(this);
}

AIO::~AIO()
{
	requests.erase(this);

	close(this->aio_fildes);
	this->con->flags |= SF_DEAD;
}

void AIO::Cancel()
{
	aio_cancel(this->aio_fildes, this);
}

void AIO::OnRead(const char *data, int len)
{
	this->con->Write(data, len);
}

void AIO::OnError()
{
}

