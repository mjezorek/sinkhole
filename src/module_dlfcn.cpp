#include "sinkhole.hpp"
#include "module.hpp"

#include <dlfcn.h>

using namespace Sinkhole::Modules;

ModuleHandle Symbol::Open(const std::string &file)
{
	std::string real_file = "../lib/" + file;
	ModuleHandle handle = dlopen(real_file.c_str(), RTLD_LAZY);
	if (handle == NULL)
		throw ModuleException(dlerror());
	return handle;
}

void Symbol::Close(ModuleHandle handle)
{
	if (dlclose(handle))
		throw ModuleException(dlerror());
}

void *Symbol::GetSymbol(ModuleHandle handle, const std::string &name)
{
	dlerror();
	void *symbol = dlsym(handle, name.c_str());
	const char *error = dlerror();
	if (error != NULL)
		throw ModuleException(error);
	return symbol;
}

