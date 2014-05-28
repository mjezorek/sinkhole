#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "threads.hpp"
#include "pipe.hpp"
#include "include/ftp.hpp"
#include "include/aio.hpp"
#include "include/client.hpp"
#include "include/command.hpp"

#include <aio.h>
#include <errno.h>
#include <unistd.h>

using namespace Sinkhole::Protocol::FTP;

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
				int ret = aio_return(a);
				if (ret <= -1)
				{
					a->OnError();
					delete a;
				}
				else
					a->OnReturn(ret);
			}
		}
	}
} aio_sighandler;

AIO::AIO(int fd)
{
	this->aio_fildes = fd;
	this->aio_offset = 0;
	this->aio_lio_opcode = LIO_NOP;
	this->aio_reqprio = 0;
	this->aio_buf = NULL;
	this->aio_nbytes = 0;
	memset(&this->aio_sigevent, 0, sizeof(this->aio_sigevent));
	this->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	this->aio_sigevent.sigev_signo = SIGUSR1;

	requests.insert(this);
}

AIO::~AIO()
{
	requests.erase(this);
}

void AIO::Cancel()
{
	aio_cancel(this->aio_fildes, this);
}

void AIO::OnError()
{
}

AIORead::AIORead(FTPClient *cl, Command *c, int fd) : AIO(fd), command(c), client(cl)
{
	cl->read_request = this;

	this->aio_lio_opcode = LIO_READ;

	this->aio_buf = this->read_buffer;
	this->aio_nbytes = sizeof(this->read_buffer);

	aio_read(this);
}

AIORead::~AIORead()
{
	if (this->client != NULL)
	{
		this->client->read_request = NULL;
		this->command->Finish(this->client);
	}

	close(this->aio_fildes);
}

void AIORead::OnReturn(int ret)
{
	if (ret == 0)
		delete this;
	else
	{
		this->aio_offset += ret;
		this->command->OnData(this->client, this->read_buffer, ret);
		aio_read(this);
	}
}

AIOWrite::AIOWrite(int fd, const char *data, int len) : AIO(fd)
{
	this->aio_lio_opcode = LIO_WRITE;

	this->aio_buf = new char[len];
	memcpy(const_cast<void *>(this->aio_buf), data, len);
	this->aio_nbytes = len;
}

AIOWrite::~AIOWrite()
{
	volatile const char *aio_data = static_cast<volatile const char *>(this->aio_buf);
	delete [] aio_data;
}

void AIOWrite::OnReturn(int ret)
{
	if (this->aio_nbytes - ret == 0)
		delete this;
	else
	{
		volatile char *aio_data = static_cast<volatile char *>(this->aio_buf);
		aio_data += ret;

		this->aio_buf = aio_data;
		this->aio_nbytes -= ret;
		aio_write(this);
	}
}

