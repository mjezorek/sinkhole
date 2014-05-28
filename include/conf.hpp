#ifndef CONF_H
#define CONF_H

namespace Sinkhole
{
	class ConfigException : public Exception
	{
	 public:
		ConfigException(const std::string &r) : Exception(r) { }
	};

	class ConfigurationFile
	{
		std::string name;
		bool executable;
		FILE *fp;
	 public:
		ConfigurationFile(const std::string &n, bool e = false);
		~ConfigurationFile();

		const std::string &GetName() const;

		bool IsOpen() const;
		bool Open();
		void Close();
		bool End() const;
		std::string Read() const;
	};

	struct ConfigurationBlock
	{
		typedef std::map<std::string, std::string> item_map;
		typedef std::multimap<std::string, ConfigurationBlock> blocks_map;

		std::string name;
		item_map items;
		blocks_map blocks;

		ConfigurationBlock(const std::string &n);
		const std::string &GetValue(const std::string &iname);
		bool GetBool(const std::string &iname);
		int CountBlock(const std::string &iname);
		ConfigurationBlock &GetBlock(const std::string &iname, int num = 0);
	 private:
	 	ConfigurationBlock();
	};

	class Configuration
	{
		ConfigurationBlock::blocks_map blocks;
	 public:
		Configuration(const std::string &file);
		~Configuration();
		void Read(ConfigurationFile &file);
		void Process();
		int CountBlock(const std::string &name);
		ConfigurationBlock &GetBlock(const std::string &name, int num = 0);
	};

	extern Configuration *Config;
}

#endif // CONF_H
