#ifndef FTP_DIRECTORY_H
#define FTP_DIRECTORY_H

#include <sys/types.h>
#include <dirent.h>

FTP_NAMESPACE_BEGIN

class FTPFile
{
 public:
	mode_t mode;
	nlink_t links;
	uid_t uid;
	gid_t gid;
	off_t size;
	time_t modified;
	std::string name;

	std::string GetListInfo();
};

class FTPDirectory
{
	std::deque<std::string> dirs;
	std::string root;

	bool IsDirectory(const std::string &path);

 public:
	FTPDirectory();
	~FTPDirectory();
	
	void Init(const std::string &dir);
	std::string GetCWD() const;
	void Chdir(const std::string &name);
	void Mkdir(const std::string &name);
	void Rm(const std::string &name);
	std::string GetRelativePath(const std::string &name);
	std::vector<FTPFile> FileList(const std::string &path);
	std::vector<std::string> NameList(const std::string &path);
};

FTP_NAMESPACE_END

#endif

