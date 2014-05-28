#ifndef PIPE_H
#define PIPE_H

namespace Sinkhole
{
	struct PipeData
	{
		int rfd, wfd;

		PipeData();
	};

	class Pipe : public Socket
	{
	 	/** The FD of the write pipe
		 * this->Sock is the readfd
		 */
		int WriteFD;
 	 public:
		/** Default constructor
		 * @param d Pipe data for r/w sockets
		 */
		Pipe(PipeData d);

	 	/** Destructor, closes WriteFD
		 */
	 	~Pipe();

		/** Allocates and returns a new Pipe
		 * @return the new Pipe
		 */
		static Pipe *Create();

		/** Called when data is to be read
		 */
		bool ProcessRead();

		/** Called when data can be written
		 */
		bool ProcessWrite();

		/** Called when this pipe needs to be woken up
		 */
		void Notify();

		/** Should be overloaded to do something useful
		 */
		virtual void OnNotify() = 0;
	};
}

#endif // PIPE_H

