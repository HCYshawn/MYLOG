
#ifndef __M_FMT_H__
#define __M_FMT_H__

#include "level.hpp"
#include "message.hpp"
#include <ctime>
#include <memory>
#include <vector>
#include <cassert>

namespace logsys
{
    // 抽象格式化子项基类
    class FormatItem
    {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual void format(std::ostream &out, const LogMsg &msg) = 0;
    };

    // 派生格式化子项子类--消息，等级，时间，文件名，行号，线程ID，日志器名，制表符，换行，其他

    class MsgFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._payload;
        }
    };

    class LevelFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << LogLevel::toString(msg._level);
        }
    };

    class TimeFormatItem : public FormatItem
    {
    public:
        TimeFormatItem(const std::string &fmt = "%H:%M:%S")
            : _time_fmt(fmt) {}
        void format(std::ostream &out, const LogMsg &msg) override
        {
            struct tm t;
            localtime_r(&msg._ctime, &t);
            char tmp[32] = {0};
            strftime(tmp, 31, _time_fmt.c_str(), &t);
            out << tmp;
        }

    private:
        std::string _time_fmt;
    };

    class FileFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._file;
        }
    };

    class LineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._line;
        }
    };

    class ThreadFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._tid;
        }
    };

    class LoggerFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << msg._logger;
        }
    };

    class TabFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << "\t";
        }
    };

    class NlineFormatItem : public FormatItem
    {
    public:
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << "\n";
        }
    };

    class OtherFormatItem : public FormatItem
    {
    public:
        OtherFormatItem(const std::string &str)
            : _str(str) {}
        void format(std::ostream &out, const LogMsg &msg) override
        {
            out << _str;
        }

    private:
        std::string _str;
    };

    /*
        %d 日期
        %T 缩进
        %t 线程ID
        %p 日志级别
        %c 日志器名称
        %f 文件名
        %l 行号
        %m 日志信息
        %n 换行
    */

    class Formatter
    {
    public:
        using ptr = std::shared_ptr<Formatter>;
        Formatter(const std::string &pattern = "[%d{%H:%M:%S}][%t][%c][%f:%l][%p]%T%m%n")
            : _pattern(pattern)
        {
            assert(parsePattern());
        }

        void format(std::ostream &out, const LogMsg &msg)
        {
            for (auto &item : _items)
            {
                item->format(out, msg);
            }
        }

        const std::string format(const LogMsg &msg)
        {
            std::stringstream ss;
            format(ss, msg);
            return ss.str();
        }

    private:
        bool parsePattern()
        {
            // 对格式化规则字符串进行解析
            std::vector<std::pair<std::string, std::string>> fmt_order;
            size_t pos = 0;
            std::string key, val;
            while (pos < _pattern.size())
            {
                // 处理原始字符串--判断是否是 %，不是就是原始字符
                if (_pattern[pos] != '%')
                {
                    val.push_back(_pattern[pos++]);
                    continue;
                }

                // 至此即表示pos位置为 % 字符，%% 处理称为一个原始 % 字符
                if (pos + 1 < _pattern.size() && _pattern[pos + 1] == '%')
                {
                    val.push_back('%');
                    pos += 2;
                    continue;
                }

                // 至此即表示 % 后边为格式化字符，代表原始字符串处理完毕
                if (val.empty() == false)
                {
                    fmt_order.push_back(std::make_pair("", val));
                    val.clear();
                }

                // 此时pos指向的是 % 位置，是格式化字符的处理
                pos += 1; // 指向格式化字符位置
                if (pos == _pattern.size())
                {
                    std::cout << "% 之后无对应的格式化字符！\n";
                    return false;
                }

                key = _pattern[pos];
                pos += 1; // 指向格式化后的位置
                if (pos < _pattern.size() && _pattern[pos] == '{')
                {
                    pos += 1; // 指向 { 之后，子规则的起始位置
                    while (pos < _pattern.size() && _pattern[pos] != '}')
                    {
                        val.push_back(_pattern[pos++]);
                    }

                    // 循环至末尾跳出循环，没有遇到 }，格式出错
                    if (pos == _pattern.size())
                    {
                        std::cout << "子规则{}匹配出错！\n";
                        return false;
                    }
                    pos += 1; // 行至下一个新的处理位置
                }
                fmt_order.push_back(std::make_pair(key, val));
                key.clear();
                val.clear();
            }
            for (const auto &it : fmt_order)
            {
                _items.push_back(createItem(it.first, it.second));
            }
            return true;
        }

    private:
        FormatItem::ptr createItem(const std::string &key, const std::string &val)
        {
            if (key == "d")
            {
                return std::make_shared<TimeFormatItem>(val);
            }
            if (key == "T")
            {
                return std::make_shared<TabFormatItem>();
            }
            if (key == "t")
            {
                return std::make_shared<ThreadFormatItem>();
            }
            if (key == "p")
            {
                return std::make_shared<LevelFormatItem>();
            }
            if (key == "c")
            {
                return std::make_shared<LoggerFormatItem>();
            }
            if (key == "f")
            {
                return std::make_shared<FileFormatItem>();
            }
            if (key == "l")
            {
                return std::make_shared<LineFormatItem>();
            }
            if (key == "m")
            {
                return std::make_shared<MsgFormatItem>();
            }
            if (key == "n")
            {
                return std::make_shared<NlineFormatItem>();
            }
            if (key == "")
            {
                return std::make_shared<OtherFormatItem>(val);
            }
            std::cout << "无对应的格式化字符：%" << key << std::endl;
            abort();
            return FormatItem::ptr();
        }

    private:
        std::string _pattern; // 格式化字符串
        std::vector<FormatItem::ptr> _items;
    };
}

#endif