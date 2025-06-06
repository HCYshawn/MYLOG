/*
    通用功能类，与业务无关的功能实现
    1. 获取系统时间
    2. 获取文件大小
    3. 创建目录
    4. 获取文件所在目录
*/

#ifndef __M_UTIL_H__
#define __M_UTIL_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <cassert>
#include <sys/stat.h>

namespace logsys
{
    namespace util
    {
        class Date
        {
        public:
            static size_t now() { return (size_t)time(nullptr); }
        };
        class File
        {
        public:
            // 获取文件大小
            static bool exists(const std::string &name)
            {
                struct stat st;
                return (stat(name.c_str(), &st) == 0);
            }

            //
            static std::string path(const std::string &name)
            {
                if (name.empty())
                {
                    return ".";
                }
                size_t pos = name.find_last_of("/\\");
                if (pos == std::string::npos)
                {
                    return ".";
                }
                return name.substr(0, pos + 1);
            }

            //
            static void create_directory(const std::string &path)
            {
                size_t pos = 0;
                size_t idx = 0;
                while (idx < path.size())
                {
                    pos = path.find_first_of("/\\", idx);
                    if (pos == std::string::npos)
                    {
                        mkdir(path.c_str(), 0755);
                        return;
                    }
                    std::string parent_dir = path.substr(0, pos + 1);
                    if (exists(parent_dir))
                    {
                        idx = pos + 1;
                        continue;
                    }
                    mkdir(parent_dir.c_str(), 0755);
                    idx = pos + 1;
                }
            }
        };
    }
}

#endif