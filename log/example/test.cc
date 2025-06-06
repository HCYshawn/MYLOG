#include "../logs/mlog.h"

void test_log(const std::string &name)
{
    INFO("%s", "测试开始");
    logsys::Logger::ptr logger = logsys::LoggerManager::getInstance().getLogger(name);
    logger->debug("%s", "测试日志");
    logger->info("%s", "测试日志");
    logger->warn("%s", "测试日志");
    logger->error("%s", "测试日志");
    logger->fatal("%s", "测试日志");
    INFO("%s", "测试完毕");
}

int main()
{
    std::unique_ptr<logsys::LoggerBuilder> builder(new logsys::GlobalLoggerBuilder());
    builder->buildLoggerName("async_logger");
    builder->buildLoggerLevel(logsys::LogLevel::value::DEBUG);
    builder->buildFormmatter("[%c][%f:%l][%p]%m%n");
    builder->buildLoggerType(logsys::LoggerType::LOGGER_SYNC);
    // builder->buildEnableUnSafeAsync();
    builder->buildSink<logsys::FileSink>("./logfile/sync.log");
    builder->buildSink<logsys::RollBySizeSink>("./logfile/roll-sync-by-size", 1024 * 1024);
    builder->build();
    test_log("async_logger");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}