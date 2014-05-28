#include "sinkhole.hpp"
#include "network.hpp"
#include "string.hpp"
#include "include/ftp.hpp"
#include "include/directory.hpp"
#include <unistd.h>
using namespace Sinkhole::Protocol::FTP;

std::string FTPFile::GetListInfo()
{
	std::string perms;

	if (S_ISDIR(this->mode))
		perms += "d";
	else
		perms += "-";
	
	int permissions[][3] = {
		{ S_IRUSR, S_IWUSR, S_IXUSR },
		{ S_IRGRP, S_IWGRP, S_IXGRP },
		{ S_IROTH, S_IWOTH, S_IXOTH }
	};

	for (int i = 0; i < 3; ++i)
	{
		if (this->mode & permissions[i][0])
			perms += "r";
		else
			perms += "-";

		if (this->mode & permissions[i][1])
			perms += "w";
		else
			perms += "-";

		if (this->mode & permissions[i][2])
			perms += "x";
		else
			perms += "-";
	}

	char *modified_time = ctime(&this->modified);
	char *p = strchr(modified_time, '\n');
	if (p)
		*p = 0;

	return Sinkhole::printf("%s %4d %d %8d %12d %s %s\n", perms.c_str(), this->links, this->uid, this->gid, this->size, modified_time, this->name.c_str());
}

FTPDirectory::FTPDirectory()
{
}

FTPDirectory::~FTPDirectory()
{
}

bool FTPDirectory::IsDirectory(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode))
		return true;
	return false;
}

void FTPDirectory::Init(const std::string &dir)
{
	if (!this->IsDirectory(dir))
		throw FTPException("Unable to open " + dir);
	this->root = dir;
}

std::string FTPDirectory::GetCWD() const
{
	std::string cwd = this->root;
	for (unsigned i = 0; i < this->dirs.size(); ++i)
		cwd += "/" + this->dirs[i];
	return cwd;
}

void FTPDirectory::Chdir(const std::string &name)
{
	std::deque<std::string> dirs_copy = this->dirs;
	Sinkhole::sepstream sep(name, '/');
	std::string token;
	while (sep.GetToken(token))
		if (token == "..")
		{
			if (!dirs_copy.empty())
				dirs_copy.pop_back();
		}
		else if (!token.empty() && token != ".")
			dirs_copy.push_back(token);

	std::string new_dir = this->root;
	for (unsigned i = 0; i < dirs_copy.size(); ++i)
		new_dir += "/" + dirs_copy[i];

	if (!this->IsDirectory(new_dir))
		throw FTPException("Unable to open " + new_dir);
	this->dirs = dirs_copy;
}

void FTPDirectory::Mkdir(const std::string &name)
{
	if (name.find("..") != std::string::npos)
		throw FTPException("Invalid filename");

	std::string new_dir = this->GetCWD() + "/" + name;
	if (mkdir(new_dir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1)
		throw FTPException(Sinkhole::LastError());
}

void FTPDirectory::Rm(const std::string &name)
{
	if (name.find("..") != std::string::npos)
		throw FTPException("Invalid filename");
	
	std::string new_path = this->GetCWD() + "/" + name;
	struct stat s;
	if (stat(new_path.c_str(), &s) == -1)
		throw FTPException(Sinkhole::LastError());
	else if (S_ISDIR(s.st_mode))
	{
		if (rmdir(new_path.c_str()) == -1)
			throw FTPException(Sinkhole::LastError());
	}
	else
	{
		if (unlink(new_path.c_str()) == -1)
			throw FTPException(Sinkhole::LastError());
	}
}

std::string FTPDirectory::GetRelativePath(const std::string &name)
{
	std::deque<std::string> dirs_copy = this->dirs;
	Sinkhole::sepstream sep(name, '/');
	std::string token;
	while (sep.GetToken(token))
		if (token == "..")
		{
			if (!dirs_copy.empty())
				dirs_copy.pop_back();
		}
		else if (!token.empty() && token != ".")
			dirs_copy.push_back(token);

	std::string new_dir = this->root;
	for (unsigned i = 0; i < dirs_copy.size(); ++i)
		new_dir += "/" + dirs_copy[i];
	return new_dir;
}

std::vector<FTPFile> FTPDirectory::FileList(const std::string &path)
{
	std::string real_path = this->GetRelativePath(path);
	std::vector<FTPFile> files;
	struct stat s;

	if (stat(real_path.c_str(), &s))
		throw FTPException("Unable to open " + real_path + ": " + Sinkhole::LastError());
	
	if (S_ISDIR(s.st_mode))
	{
		DIR *dirp = opendir(real_path.c_str());
		if (dirp == NULL)
			throw FTPException("Unable to open " + real_path + ": " + Sinkhole::LastError());
	
		for (dirent *dir; (dir = readdir(dirp)) != NULL;)
		{
			if (dir->d_name[0] == '.')
				continue;

			if (stat((real_path + dir->d_name).c_str(), &s))
				continue;

			FTPFile file;
			file.mode = s.st_mode;
			file.links = s.st_nlink;
			file.uid = s.st_uid;
			file.gid = s.st_gid;
			file.size = s.st_size;
			file.modified = s.st_mtime;
			file.name = dir->d_name;

			files.push_back(file);
		}
	
		closedir(dirp);
	}
	else
	{
		FTPFile file;
		file.mode = s.st_mode;
		file.links = s.st_nlink;
		file.uid = s.st_uid;
		file.gid = s.st_gid;
		file.size = s.st_size;
		file.modified = s.st_mtime;
		file.name = path;

		files.push_back(file);
	}

	return files;
}

std::vector<std::string> FTPDirectory::NameList(const std::string &path)
{
	std::string real_path = this->GetRelativePath(path);
	std::vector<std::string> files;
	struct stat s;

	if (stat(real_path.c_str(), &s))
		throw FTPException("Unable to open " + real_path + ": " + Sinkhole::LastError());
	else if (!S_ISDIR(s.st_mode))
		throw FTPException(real_path + " is not a directory");

	DIR *dirp = opendir(real_path.c_str());
	if (dirp == NULL)
		throw FTPException("Unable to open " + real_path + ": " + Sinkhole::LastError());
	
	for (dirent *dir; (dir = readdir(dirp)) != NULL;)
	{
		if (dir->d_name[0] == '.')
			continue;

		files.push_back(dir->d_name);
	}
	
	closedir(dirp);

	return files;
}

