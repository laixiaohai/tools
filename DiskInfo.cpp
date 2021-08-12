#include "DiskInfo.h"

#include <wchar.h>
#include<stdio.h>


DiskInfo::DiskInfo() : m_childDir(false), m_limit(0)
{
	ReadDiskSubareaInfo();
}

DiskInfo::~DiskInfo()
{
}


// 读取磁盘分区信息
void DiskInfo::ReadDiskSubareaInfoW()
{
	typeWStr diskSubareaList;

	wchar_t *szBuffer = new wchar_t[100];
	GetLogicalDriveStrings(100, szBuffer);

	while (*szBuffer != '\0')
	{
		diskSubareaList.push_back(szBuffer);

		szBuffer += wcslen(szBuffer) + 1;
	}

	m_diskSubareaListW = diskSubareaList;

	return;
}

void DiskInfo::ReadDiskSubareaInfo()
{
	typeStr diskSubareaList;

	char *szBuffer = new char[100];
	GetLogicalDriveStringsA(100, szBuffer);

	while (*szBuffer != '\0')
	{
		diskSubareaList.push_back(szBuffer);

		szBuffer += strlen(szBuffer) + 1;
	}

	m_diskSubareaList = diskSubareaList;

	return;
}

// 获取磁盘分区信息的列表
DiskInfo::typeStr &DiskInfo::GetDiskSubareaList()
{
	return m_diskSubareaList;
}

// 获取磁盘分区信息的列表
DiskInfo::typeWStr &DiskInfo::GetDiskSubareaListW()
{
	return m_diskSubareaListW;
}

// 查找目录下的文件和目录地址
int DiskInfo::Find(string sPath, string sFext /* = "" */)
{
	const char c_pathSeparator = '\\';
	const string s_pathSeparator = "\\";

	m_contentFile.clear();
	m_contentDir.clear();
	m_content.clear();

	string path = sPath;
	if (path.size() > 0 && c_pathSeparator != path[path.size() - 1])
	{
		path.append(s_pathSeparator);
	}

	m_contentDir.push_back(path);

	string fext = sFext;
	if (0 == fext.compare("*") || 0 == fext.compare("*.*"))
	{
		fext = "";
	}

	//string file = fext;
	string file = "*";

	string s = path + file;

	WIN32_FIND_DATAA fileinfo = { 0 };
	HANDLE handle = FindFirstFileA(s.c_str(), &fileinfo);

	if (NULL != handle && INVALID_HANDLE_VALUE != handle)
	{
		do
		{
			if (Limit()) break; //--limit test

			if ('.' != fileinfo.cFileName[0])   //--skip./..
			{
				if ((FILE_ATTRIBUTE_DIRECTORY & fileinfo.dwFileAttributes) == FILE_ATTRIBUTE_DIRECTORY) //--目录
				{
					if (!Limit()/*&& 0 == fext.size()*/) /*--limit test*/
					{
						m_contentDir.push_back(path + fileinfo.cFileName + s_pathSeparator);
					}

					if (GetFindChildDir())
					{
						DiskInfo o;
						o.SetFindChildDir(GetFindChildDir());
						o.Find(path + fileinfo.cFileName, fext);

						//--dir
						typeStr o_dir = o.ContentDir();
						for (typeStr::iterator it_dir = o_dir.begin(); o_dir.end() != it_dir && !Limit(); /*--limit test*/it_dir++)
						{
							m_contentDir.push_back(*it_dir);
						}

						//--file
						typeStr o_file = o.ContentFile();
						for (typeStr::iterator it_file = o_file.begin(); o_file.end() != it_file && !Limit(); /*--limit test*/it_file++)
						{
							m_contentFile.push_back(*it_file);
						}
					}
				}
				else
				{
					if (!Limit()    /*--limit test*/ && (0 == fext.size() || Match(fext, fileinfo.cFileName)))
					{
						m_contentFile.push_back(path + fileinfo.cFileName);
					}
				}
			}
		} while (FindNextFileA(handle, &fileinfo));

		FindClose(handle);
	}

	//--dir
	for (typeStr::iterator it_dir = m_contentDir.begin(); m_contentDir.end() != it_dir; it_dir++)
	{
		m_content.push_back(*it_dir);
	}
	//--file
	for (typeStr::iterator it_file = m_contentFile.begin(); m_contentFile.end() != it_file; it_file++)
	{
		m_content.push_back(*it_file);
	}

	return Count();
}


// 获取目录下文件和目录的总数
int DiskInfo::Count()
{
	return m_contentDir.size() + m_contentFile.size();
}


// 获取目录下所有文件和目录的地址
DiskInfo::typeStr &DiskInfo::Content()
{
	return m_content;
}


// 获取目录下所有目录的地址
DiskInfo::typeStr &DiskInfo::ContentDir()
{
	return m_contentDir;
}

// 获取目录下所有文件的地址
DiskInfo::typeStr &DiskInfo::ContentFile()
{
	return m_contentFile;
}

// 设置是否查找子目录
void DiskInfo::SetFindChildDir(bool bFind)
{
	m_childDir = bFind;
	return;
}

// 获取是否查找子目录
bool DiskInfo::GetFindChildDir()
{
	return m_childDir;
}

// 判断s是否为目录
bool DiskInfo::Dir(string s)
{
	return (FILE_ATTRIBUTE_DIRECTORY == GetFileAttributesA(s.c_str()));
}

// 设置查找文件数目上限
void DiskInfo::SetLimit(unsigned int iLimit)
{
	m_limit = iLimit;
	return;
}

// 获取查找文件数目上限
int DiskInfo::GetLimit()
{
	return m_limit;
}

// 文件匹配检测
int DiskInfo::Match(string sFext, string sFile)
{
	string fext = Uppercase(sFext);
	string file = Uppercase(sFile);

	int pos = file.find_last_of('.');
	if (string::npos != pos) file = file.substr(pos);

	return (string::npos != fext.find(file));
}

// 判断是否超过查找的文件数目上限
bool DiskInfo::Limit()
{
	if (m_limit)
	{
		return Count() == GetLimit();
	}
	else
	{
		return false;
	}
}

//--转换s到大写字母
string DiskInfo::Uppercase(string s)
{
	const char aAzZ = 'z' - 'Z';
	string rs;

	for (string::iterator it = s.begin(); s.end() != it; it++)
	{
		if ('a' <= *it && *it <= 'z') rs.append(1, *it - aAzZ);
		else rs.append(1, *it);
	}

	return rs;
}
