/*
    日志落地模块的实现
    1. 抽象落地基类
    2. 派生子类（根据不同的落地方向进行派生）
    3. 使用工厂模式进行创建与表示的分离
*/
#ifndef __M_SINK_H__
#define __M_SINK_H__

#include "util.hpp"
#include <fstream>
#include <memory>
#include <cassert>

namespace logsys
{
    class LogSink
    {
    public:
        using ptr = std::shared_ptr<LogSink>;
        LogSink() {}
        virtual ~LogSink() {}
        virtual void log(const char *data, size_t len) = 0;
    };

    class StdoutSink : public LogSink
    {
    public:
        // 将日志消息写入至标准输出
        void log(const char *data, size_t len)
        {
            std::cout.write(data, len);
        }
    };

    // 落地方向：指定文件
    class FileSink : public LogSink
    {
    public:
        // 构造时传入文件名并打开文件，将操作句柄管理起来
        FileSink(const std::string &pathname)
            : _pathname(pathname)
        {
            // 1. 创建日志文件所在的目录
            util::File::create_directory(util::File::path(pathname));
            // 2. 创建并打开日志文件
            _ofs.open(_pathname, std::ios::binary | std::ios::app);

            assert(_ofs.is_open());
        }

        void log(const char *data, size_t len)
        {
            _ofs.write(data, len);
            assert(_ofs.good());
        }

    private:
        std::string _pathname;
        std::ofstream _ofs;
    };

    // 落地方向：滚动文件（以大小进行滚动）
    class RollBySizeSink : public LogSink
    {
    public:
        // 构造时传入文件名，并打开文件，将操作句柄管理起来
        RollBySizeSink(const std::string &basename, size_t max_size)
            : _basename(basename),
              _max_fsize(max_size),
              _cur_fsize(0),
              _name_count(0)
        {
            std::string pathname = createNewFile();
            // 1. 创建日志文件所在的目录
            util::File::create_directory(util::File::path(pathname));
            // 2. 创建并打开日志文件
            _ofs.open(pathname, std::ios::binary | std::ios::app);

            assert(_ofs.is_open());
        }

        // 将日志消息写入至标准输出，写入前判断文件大小，超过了最大大小要切换文件
        void log(const char *data, size_t len)
        {
            if (_cur_fsize >= _max_fsize)
            {
                _ofs.close();
                std::string pathname = createNewFile();
                _ofs.open(pathname, std::ios::binary | std::ios::app);
                assert(_ofs.is_open());
                _cur_fsize = 0;
            }
            _ofs.write(data, len);
            assert(_ofs.good());
            _cur_fsize += len;
        }

    private:
        // 进行大小判断，超过指定大小则创建新的文件
        std::string createNewFile()
        {
            time_t t = util::Date::now();
            struct tm lt;
            localtime_r(&t, &lt);
            std::stringstream filename;
            filename << _basename;
            filename << lt.tm_year + 1900;
            filename << lt.tm_mon + 1;
            filename << lt.tm_mday;
            filename << lt.tm_hour;
            filename << lt.tm_min;
            filename << lt.tm_sec;
            filename << "-";
            filename << _name_count++;
            filename << ".log";
            return filename.str();
        }

    private:
        // 通过基础文件名+拓展文件名组成实际当前输出文件名
        std::string _basename;
        std::ofstream _ofs;
        size_t _name_count;
        size_t _max_fsize; // 规定的文件最大大小
        size_t _cur_fsize; // 记录当前文件大小
    };

    class SinkFactory
    {
    public:
        template <typename SinkType, typename... Args>
        static LogSink::ptr create(Args &&...args)
        {
            return std::make_shared<SinkType>(std::forward<Args>(args)...);
        }
    };
}

#endif