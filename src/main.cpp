#include "sinkhole.hpp"
#include "conf.hpp"
#include "config.h"
#include "network.hpp"
#include "io.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "threads.hpp"
#include "timers.hpp"
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

using namespace Sinkhole;

static std::string configuration_file = "sinkhole.conf";

static void parse_args(int argc, char **argv)
{
	option longopts[] = {
		{ "config", required_argument, NULL, 'c' },
		{ "debug", no_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "nofork",  no_argument, NULL, 'n' },
		{ "version", no_argument, NULL, 'v' },
		{ 0, 0, 0, 0 }
	};

	for (int i; (i = getopt_long_only(argc, argv, "c:dhnv", longopts, NULL)) != -1;)
	{
		switch (i)
		{
			case 'c':
				configuration_file = optarg;
				break;
			case 'd':
				debug = true;
				break;
			case 'h':
				std::cout << "Usage: " << argv[0] << " [OPTIONS]" << std::endl << std::endl;
				std::cout << "  Available options:" << std::endl;
				std::cout << "    --config filename   Use filename as the configuration file" << std::endl;
				std::cout << "    --debug             Enable debug mode" << std::endl;
				std::cout << "    --help              Shows this help" << std::endl;
				std::cout << "    --nofork            Don't daemonize" << std::endl;
				std::cout << "    --version           Print version and exit" << std::endl;
				exit(0);
			case 'n':
				nofork = true;
				break;
			case 'v':
				static const char *const COMPILED_TIME =  __DATE__ " at " __TIME__;
				std::cout << PACKAGE_NAME << " version " << PACKAGE_VERSION << std::endl;
				std::cout << "   compiled " << COMPILED_TIME << std::endl;
				exit(0);
			default:
				std::cout << "Unknown parameter '" << argv[optind - 1] << "'" << std::endl;
				exit(0);
				std::cout << "Usage: " << argv[0] << " [ --config <config> ] [ --debug ] [ --help ] [ --nofork ] [ --version ]" << std::endl; 
		}
	}
}

static void set_chdir(char *command)
{
	char *p = strrchr(command, '/');
	if (p == NULL)
		return;
	*p = 0;
	if (strcmp(command, "."))
		chdir(command);
	*p = '/';
}

static void on_start()
{
	try
	{
		ConfigurationBlock &sinkhole = Config->GetBlock("sinkhole");

		std::string uid, gid;
		struct passwd *u = NULL;
		struct group *g = NULL;

		try
		{
			uid = sinkhole.GetValue("runuser");

			errno = 0;
			u = getpwnam(uid.c_str());
			if (u == NULL)
				throw Exception("Unable to setuid to " + uid + ": " + LastError());
		}
		catch (const ConfigException &) { }

		try
		{
			gid = sinkhole.GetValue("rungroup");

			errno = 0;
			g = getgrnam(gid.c_str());
			if (g == NULL)
				throw Exception("Unable to setgid to " + gid + ": " + LastError());
		}
		catch (const ConfigException &) { }

		try
		{
			std::string chroot_dir = sinkhole.GetValue("chroot");
			if (chroot(chroot_dir.c_str()) == -1)
				throw Exception("Unable to chroot to " + chroot_dir + ": " + LastError());
			else
				Log(LOG_INFORMATIONAL) << "Jailed into " << chroot_dir;
		}
		catch (const ConfigException &) { }

		if (u != NULL && setuid(u->pw_uid) == -1)
			throw Exception("Unable to setuid to " + uid + ": " + LastError());

		if (g != NULL && setgid(g->gr_gid) == -1)
			throw Exception("Unable to setgid to " + gid + ": " + LastError());
	}
	catch (const ConfigException &) { }
}

class SignalKill : public Signal
{
 public:
	SignalKill(int signum) : Signal(signum) { }

	void OnSignal()
	{
		Log(LOG_INFORMATIONAL) << "Received " << (this->signal == SIGKILL ? "SIGKILL" : "SIGINT") << " - shutting down";
		shutting_down = true;
	}
};

int main(int argc, char **argv)
{
	SignalKill signal_sigterm(SIGTERM), signal_sigint(SIGINT);

	parse_args(argc, argv);
	if (argc > 0)
		set_chdir(argv[0]);

	Log(LOG_INFORMATIONAL) << "Sinkhole starting up.";

	try
	{
		Config = new Configuration(configuration_file);
		SocketEngine::Init();
		ThreadEngine::Init();
		Config->Process();
		on_start();
	}
	catch (const Exception &ex)
	{
		Log(LOG_ERROR) << ex.GetReason();
		return -1;
	}

	if (nofork == false && fork() > 0)
		return 0;

	int errorcode = 0;
	try
	{
		while (shutting_down == false)
		{
			SocketEngine::Process();
			ThreadEngine::Process();
			TimerManager::Process();
			Signal::Process();
		}
	}
	catch (const Exception &ex)
	{
		Log(LOG_ERROR) << ex.GetReason();
		errorcode = -1;
	}

	try
	{
		Modules::UnloadAll();
		ThreadEngine::Shutdown();
		SocketEngine::Shutdown();
	}
	catch (const Exception &ex)
	{
		Log(LOG_ERROR) << ex.GetReason();
		errorcode = -1;
	}

	return errorcode;
}

