#include "../logs/mlog.h"
#include <vector>
#include <thread>

void bench(const std::string &logger_name, size_t thr_count, size_t msg_count, size_t msg_len)
{
    // 1.获取日志器
    logsys::Logger::ptr logger = logsys::getLogger(logger_name);
    if (logger.get() == nullptr)
    {
        return;
    }
    std::cout << "测试日志: " << msg_count << "条, 总大小: " << (msg_count * msg_len) / 1024 << "KB\n";
    // 2. 组织指定长度的日志消息
    std::string msg(msg_len - 1, 'A');
    // 3. 创建指定数量的线程
    std::vector<std::thread> threads;
    std::vector<double> cost_arry(thr_count);
    // 总日志数量/线程数量 = 每个线程要输出的日志数量
    size_t msg_ptr_thr = msg_count / thr_count;
    for (int i = 0; i < thr_count; i++)
    {
        threads.emplace_back([&, i]()
                             {
            //4. 线程函数内部开始计时
            auto start = std::chrono::high_resolution_clock::now();
            //5. 开始循环写日志
            for (int j = 0; j < msg_ptr_thr;j++)
            {
                logger->fatal("%s", msg.c_str());

            }
            //6.线程函数内部结束计时
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> cost = end-start;
            cost_arry[i] = cost.count();
            std::cout << "\t线程 " << i << ": "
                      << "输出日志数量: " << msg_ptr_thr << ", 耗时: "
                      << cost.count() << "s" << std::endl; });
    }
    for (int i = 0; i < thr_count; i++)
    {
        threads[i].join();
    }

    // 7.计算总耗时，由于线程并发，耗时最高线程即总时间
    double max_cost = cost_arry[0];
    for (int i = 0; i < thr_count; i++)
    {
        max_cost = max_cost < cost_arry[i] ? cost_arry[i] : max_cost;
    }
    size_t msg_per_sec = msg_count / max_cost;
    size_t size_per_sec = (msg_count * msg_len) / (max_cost * 1024);
    // 8.进行输出打印
    std::cout << "\t总耗时: " << max_cost << "s\n";
    std::cout << "\t每秒输出日志数量: " << msg_per_sec << " 条\n";
    std::cout << "\t每秒输出日志大小: " << size_per_sec << " KB\n";
}

void sync_bench()
{
    std::cout << "**************************同步日志器测试**************************" << std::endl;
    std::unique_ptr<logsys::LoggerBuilder> builder(new logsys::GlobalLoggerBuilder());
    builder->buildLoggerName("sync_logger");
    builder->buildFormmatter("%m%n");
    builder->buildLoggerType(logsys::LoggerType::LOGGER_SYNC);
    builder->buildSink<logsys::FileSink>("./logfile/sync.log");
    builder->build();

    std::cout << "********单线程测试********" << std::endl;
    bench("sync_logger", 1, 1000000, 100);
    std::cout << "\n";
    std::cout << "********多线程测试********" << std::endl;
    bench("sync_logger", 2, 1000000, 100);
    std::cout << "\n";
}

void async_bench()
{
    std::cout << "**************************异步日志器测试**************************" << std::endl;
    std::unique_ptr<logsys::LoggerBuilder> builder(new logsys::GlobalLoggerBuilder());
    builder->buildLoggerName("async_logger");
    builder->buildFormmatter("%m%n");
    builder->buildLoggerType(logsys::LoggerType::LOGGER_SYNC);
    // builder->buildEnableUnSafeAsync(); // 启用非安全模式，排除实际落地时间
    builder->buildSink<logsys::FileSink>("./logfile/async.log");
    builder->build();

    std::cout << "********单线程测试********" << std::endl;
    bench("async_logger", 1, 1000000, 100);
    std::cout << "\n";
    std::cout << "********多线程测试********" << std::endl;
    bench("async_logger", 2, 1000000, 100);
    std::cout << "\n";
}

int main()
{
    sync_bench();
    async_bench();
    return 0;
}