#ifndef MODULE_H
#define MODULE_H

#define MODULE_INIT(x) \
	extern "C" \
	{ \
		Sinkhole::Modules::Module *ModuleInit(const std::string &modname) \
		{ \
			return new x(modname); \
		} \
		void ModuleDestroy(Sinkhole::Modules::Module *m) \
		{ \
			x *rm = dynamic_cast<x *>(m); \
			delete rm; \
		} \
	}

#define FOREACH_MOD(x) \
	{ \
		using namespace Sinkhole::Modules; \
		for (std::vector<Module *>::iterator _i = modules.begin(), _i_end = modules.end(); _i != _i_end; ++_i) \
		{ \
			try \
			{ \
				(*_i)->x; \
			} \
			catch (const ModuleException &ex) \
			{ \
				Log(LOG_INFORMATIONAL) << "Exception caught: " << ex.GetReason(); \
			} \
		} \
	}

#define FOREACH_RESULT(x, y) \
	{ \
		using namespace Sinkhole::Modules; \
		for (std::vector<Module *>::iterator _i = modules.begin(), _i_end = modules.end(); _i != _i_end; ++_i) \
		{ \
			try \
			{ \
				x = (*_i)->y; \
			} \
			catch (const ModuleException &ex) \
			{ \
				Log(LOG_INFORMATIONAL) << "Exception caught: " << ex.GetReason(); \
			} \
		} \
	}

namespace Sinkhole
{
	class Configuration;
	class Socket;

	namespace Modules
	{
		typedef void* ModuleHandle;

		class ModuleException : public Exception
		{
		 public:
			ModuleException(const std::string &r) : Exception(r) { }
		};

		class Module
		{
		 public:
		 	std::string name;
			ModuleHandle handle;

			Module(const std::string &modname);
			virtual ~Module();

			/** Called when a connection is accepted
			 * @param s The connection
			 */
			virtual void OnClientAccept(Socket *) { }

			/** Called when there is something useful to log
			 * @param target The name of the logging resource wanted
			 * @param module The module name logging the data
			 * @param source The source of the data, protocol dependant
			 * @param action Arbitrary action, protocol dependant
			 * @param data Miscellaneous data
			 */
			virtual void OnLog(const std::string &, const std::string &, const std::string &, const std::string &, const std::string &) { }
		};

		namespace Symbol
		{
			extern ModuleHandle Open(const std::string &file);
			extern void Close(ModuleHandle handle);
			extern void *GetSymbol(ModuleHandle handle, const std::string &name);
		}

		extern std::vector<Module *> modules;
		extern void Load(const std::string &modname);
		extern void Unload(Module *m);
		extern void UnloadAll();
	}
}

#endif // MODULE_H

