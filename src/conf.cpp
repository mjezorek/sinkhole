#include "sinkhole.hpp"
#include "conf.hpp"
#include "logger.hpp"
#include "module.hpp"
#include "string.hpp"

using namespace Sinkhole;

Configuration *Sinkhole::Config = NULL;

ConfigurationFile::ConfigurationFile(const std::string &n, bool e) : name(n), executable(e), fp(NULL)
{
}

ConfigurationFile::~ConfigurationFile()
{
	this->Close();
}

const std::string &ConfigurationFile::GetName() const
{
	return this->name;
}

bool ConfigurationFile::IsOpen() const
{
	return this->fp != NULL;
}

bool ConfigurationFile::Open()
{
	this->Close();
	this->fp = (this->executable ? popen(this->name.c_str(), "r") : fopen(("../etc/" + this->name).c_str(), "r"));
	return this->IsOpen();
}

void ConfigurationFile::Close()
{
	if (this->fp != NULL)
	{
		if (this->executable)
			pclose(this->fp);
		else
			fclose(this->fp);
		this->fp = NULL;
	}
}

bool ConfigurationFile::End() const
{
	return !this->IsOpen() || feof(this->fp);
}

std::string ConfigurationFile::Read() const
{
	std::string ret;
	char buf[1024];

	while (fgets(buf, sizeof(buf), this->fp) != NULL)
	{
		char *nl = strchr(buf, '\n');
		if (nl != NULL)
		{
			*nl = 0;
			ret = buf;
			break;
		}
		else if (!this->End())
		{
			ret += buf;
			continue;
		}
	}

	return ret;
}

ConfigurationBlock::ConfigurationBlock(const std::string &n) : name(n)
{
}

const std::string &ConfigurationBlock::GetValue(const std::string &iname)
{
	if (this->items.count(iname) == 0)
		throw ConfigException(this->name + " has no value named " + iname);
	return this->items[iname];
}

bool ConfigurationBlock::GetBool(const std::string &iname)
{
	std::string item = this->GetValue(iname);
	return (item == "yes" || item == "true" || item == "1");
}

int ConfigurationBlock::CountBlock(const std::string &iname)
{
	return this->blocks.count(iname);
}

ConfigurationBlock &ConfigurationBlock::GetBlock(const std::string &iname, int num)
{
	ConfigurationBlock::blocks_map::iterator it = this->blocks.find(iname);
	if (it == this->blocks.end())
		throw ConfigException(this->name + " has no block named " + iname);
	else if (num >= this->CountBlock(iname))
		throw ConfigException(this->name + ": out of range access to block " + iname);
	ConfigurationBlock::blocks_map::iterator it_end = this->blocks.upper_bound(iname);
	for (int count = 0; it != it_end; ++it, ++count)
	{
		if (count != num)
			continue;
		return it->second;
	}

	throw ConfigException(this->name + ": " + iname + " doesn't have block number " + stringify(num));
}
	 
ConfigurationBlock::ConfigurationBlock()
{
}

Configuration::Configuration(const std::string &file)
{
	ConfigurationFile f(file);
	this->Read(f);
}

Configuration::~Configuration()
{
}

void Configuration::Read(ConfigurationFile &file)
{
	if (file.Open() == false)
		throw ConfigException("Unable to open " + file.GetName());

	std::stack<ConfigurationBlock *> block_stack;
	std::string word_buffer;
	bool in_string = false, in_word = false, in_comment = false;

	int line_number = 0;
	while (!file.End())
	{
		std::string buf = file.Read();
		std::string item_name;
		if (in_string)
			throw ConfigException("Newline in string: " + file.GetName() + ":" + stringify(line_number));
		else if (!item_name.empty())
			throw ConfigException("Stray newline: " + file.GetName() + ":" + stringify(line_number));
		++line_number;
		for (unsigned i = 0, j = buf.length(); i < j; ++i)
		{
			if (in_string)
			{
				if (buf[i] == '"')
					in_string = in_word = false;
				else
					word_buffer += buf[i];
			}
			else if (!in_string && (buf[i] == '#' || (buf[i] == '/' && (i + 1 < j && buf[i + 1] == '/'))))
				break;
			else if (!in_string && buf[i] == '/' && i + 1 < j && buf[i + 1] == '*')
			{
				in_comment = true;
				++i;
				continue;
			}
			else if (in_comment)
			{
				if (buf[i] == '*' && i + 1 < j && buf[i + 1] == '/')
				{
					in_comment = false;
					++i;
				}
				continue;
			}
			else if (buf[i] == '\r')
				continue;
			else if (buf[i] == ' ' || buf[i] == '\t')
				in_word = false;
			else if (buf[i] == ';')
			{
				if (block_stack.empty())
					throw ConfigException("Stray ':': " + file.GetName() + ":" + stringify(line_number));
				else if (item_name.empty())
					throw ConfigException("Entry without a name: " + file.GetName() + ":" + stringify(line_number));

				ConfigurationBlock *block = block_stack.top();
				block->items.insert(std::make_pair(item_name, word_buffer));
				item_name.clear();
				word_buffer.clear();
			}
			else if (buf[i] == '{')
			{
				if (word_buffer.empty())
					throw ConfigException("Section without a name or unexpected '{': " + file.GetName() + ":" + stringify(line_number));
				in_word = false;

				if (block_stack.empty())
				{
					ConfigurationBlock::blocks_map::iterator it = this->blocks.insert(std::make_pair(word_buffer, ConfigurationBlock(word_buffer)));
					block_stack.push(&it->second);
				}
				else
				{
					ConfigurationBlock *b = block_stack.top();
					ConfigurationBlock::blocks_map::iterator it = b->blocks.insert(std::make_pair(word_buffer, ConfigurationBlock(word_buffer)));
					block_stack.push(&it->second);
				}
				word_buffer.clear();
			}
			else if (buf[i] == '}')
			{
				if (block_stack.empty())
					throw ConfigException("Stray '}': " + file.GetName() + ":" + stringify(line_number));
				block_stack.pop();
			}
			else if (buf[i] == '=')
			{
				item_name = word_buffer;
				word_buffer.clear();
			}
			else if (buf[i] == '"')
			{
				in_string = true;
			}
			else
			{
				in_word = true;
				word_buffer += buf[i];
			}
		}
	}

	file.Close();

	for (int i = 0, j = this->CountBlock("include"); i < j; ++i)
	{
		ConfigurationBlock &b = this->GetBlock("include", i);

		std::string name;
		try { name = b.GetValue("name"); }
		catch (const ConfigException &) { continue; }

		bool executable = false;
		try { executable = b.GetBool("executable"); }
		catch (const ConfigException &) { }

		ConfigurationFile conf(name, executable);
		this->Read(conf);
	}
}

void Configuration::Process()
{
	for (int i = 0, j = this->CountBlock("module"); i < j; ++i)
	{
		ConfigurationBlock &b = this->GetBlock("module", i);

		try
		{
			Modules::Load(b.GetValue("name"));
		}
		catch (const Modules::ModuleException &ex)
		{
			Log(LOG_ERROR) << "Unable to load module " << b.GetValue("name") << ": " << ex.GetReason();
		}
		catch (const ConfigException &) { }
	}
}

int Configuration::CountBlock(const std::string &name)
{
	return this->blocks.count(name);
}

ConfigurationBlock &Configuration::GetBlock(const std::string &name, int num)
{
	ConfigurationBlock::blocks_map::iterator it = this->blocks.find(name);
	if (it == this->blocks.end())
		throw ConfigException("There is no block named " + name);
	else if (num >= this->CountBlock(name))
		throw ConfigException("Out of range access to block " + name);
	ConfigurationBlock::blocks_map::iterator it_end = this->blocks.upper_bound(name);
	for (int count = 0; it != it_end; ++it, ++count)
	{
		if (count != num)
			continue;
		return it->second;
	}

	throw ConfigException("There is no block number " + stringify(num));
}

