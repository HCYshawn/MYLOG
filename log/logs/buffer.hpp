#ifndef __M_BUF_H__
#define __M_BUF_H__

#include <vector>
#include <cassert>
#include "util.hpp"

namespace logsys
{
#define DEFAULT_BUFFER_SIZE (10 * 1024 * 1024)
#define THRESHOLD_BUFFER_SIZE (8 * 1024 * 1024)
#define INCREMENT_BUFFER_SIZE (1 * 1024 * 1024)
    class Buffer
    {
    public:
        Buffer() : _buffer(DEFAULT_BUFFER_SIZE), _writer_idx(0), _reader_idx(0)
        {
        }

        // 向缓冲区写入数据
        void push(const char *data, size_t len)
        {
            // 检测空间大小
            ensureEnoughSize(len);
            // 拷贝至缓冲区
            std::copy(data, data + len, &_buffer[_writer_idx]);
            // 写入位置向后偏移
            moveWriter(len);
        }

        size_t writeAbleSize()
        {
            return (_buffer.size() - _writer_idx);
        }

        // 返回可读数据的地址
        const char *begin()
        {
            return &_buffer[_reader_idx];
        }

        // 返回可读数据的长度
        size_t readAbleSize()
        {
            return (_writer_idx - _reader_idx);
        }

        // 对读写指针进行向后偏移操作
        void moveReader(size_t len)
        {
            assert(len <= readAbleSize());
            _reader_idx += len;
        }

        // 重置读写位置，初始化缓冲区
        void reset()
        {
            _writer_idx = 0;
            _reader_idx = 0;
        }

        // 实现交换操作
        void swap(Buffer &buffer)
        {
            _buffer.swap(buffer._buffer);
            std::swap(_reader_idx, buffer._reader_idx);
            std::swap(_writer_idx, buffer._writer_idx);
        }

        // 判断缓冲区是否为空
        bool empty()
        {
            return (_reader_idx == _writer_idx);
        }

    private:
        // 对空间进行扩容
        void ensureEnoughSize(size_t len)
        {
            if (len < writeAbleSize())
                return; // 不需要扩容
            size_t new_size = 0;
            if (_buffer.size() < THRESHOLD_BUFFER_SIZE)
            {
                new_size = _buffer.size() * 2 + len; // 翻倍增长
            }
            else
            {
                new_size = _buffer.size() + INCREMENT_BUFFER_SIZE; // 线性增长
            }
            _buffer.resize(new_size);
        }

        // 对读写指针进行向后偏移操作
        void moveWriter(size_t len)
        {
            assert((len + _writer_idx) <= _buffer.size());
            _writer_idx += len;
        }

    private:
        std::vector<char> _buffer;
        size_t _reader_idx; // 当前可读数据的指针--本质是下标
        size_t _writer_idx; // 当前可写数据的指针
    };
}

#endif