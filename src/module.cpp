#include "sinkhole.hpp"
#include "logger.hpp"
#include "module.hpp"

using namespace Sinkhole;

template<typename T> static T function_cast(void *symbol)
{
	union
	{
		void *symbol;
		T function;
	} cast;
	cast.symbol = symbol;
	return cast.function;
}

std::vector<Modules::Module *> Modules::modules;

void Modules::Load(const std::string &modname)
{
	ModuleHandle handle = Modules::Symbol::Open(modname);
	Module *(*init)(const std::string &) = function_cast<Module *(*)(const std::string &)>(Modules::Symbol::GetSymbol(handle, "ModuleInit"));
	Module *newmod = init(modname);
	newmod->handle = handle;
}

void Modules::Unload(Module *m)
{
	ModuleHandle handle = m->handle;

	void (*destroy)(Module *) = function_cast<void (*)(Module *)>(Modules::Symbol::GetSymbol(m->handle, "ModuleDestroy"));
	destroy(m);

	Modules::Symbol::Close(handle);
}

void Modules::UnloadAll()
{
	for (unsigned i = modules.size(); i > 0; --i)
	{
		std::string mname = modules[i - 1]->name;

		try
		{
			Unload(modules[i - 1]);
		}
		catch (const ModuleException &ex)
		{
			Log(LOG_ERROR) << "Error unloading module " << mname << ": " << ex.GetReason();
		}
	}
}

using namespace Modules;

Module::Module(const std::string &modname) : name(modname)
{
	modules.push_back(this);
}

Module::~Module()
{
	std::vector<Module *>::iterator it = std::find(modules.begin(), modules.end(), this);
	if (it != modules.end())
		modules.erase(it);
}

