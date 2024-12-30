#include "log.h"

void init_spdlog() {
    // 异步日志，具有8k个内存和1个后台线程的队列
    spdlog::init_thread_pool(1024 * 8, 1);
    // 标准控制台输出
    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    stdout_sink->set_level(spdlog::level::debug);
    // 日志文件输出，0点0分创建新日志
    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        "logs/log.txt", 0, 0);
    file_sink->set_level(spdlog::level::info);
    // 日志回调
    auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>(
        [](const spdlog::details::log_msg &msg) {
            // 日志记录器名称
            std::string name(msg.logger_name.data(), 0, msg.logger_name.size());
            // 日志消息
            std::string str(msg.payload.data(), 0, msg.payload.size());

        });
    callback_sink->set_level(spdlog::level::info);

    std::vector<spdlog::sink_ptr> sinks{stdout_sink, file_sink, callback_sink};
    auto log = std::make_shared<spdlog::async_logger>(
        "logger", sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    // 设置日志记录级别，您需要用 %^ 和 %$  括上想要彩色的部分
    log->set_level(spdlog::level::trace);
    // 设置格式
    // 参考 https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
    //[%Y-%m-%d %H:%M:%S.%e] 时间
    //[%l] 日志级别
    //[%t] 线程
    //[%s] 文件
    //[%#] 行号
    //[%!] 函数
    //[%v] 实际文本
    log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%t] [%s %!:%#] %v");
    // 设置当出发 err 或更严重的错误时立刻刷新日志到  disk
    log->flush_on(spdlog::level::err);
    // 3秒刷新一次队列
    spdlog::flush_every(std::chrono::seconds(3));
    spdlog::set_default_logger(log);
}

// 单个日志记录器
std::shared_ptr<spdlog::logger> get_async_file_logger(std::string name) {
    auto log = spdlog::get(name);
    if (!log) {
        // 指针为空，则创建日志记录器，
        log = spdlog::daily_logger_mt<spdlog::async_factory>(
            name, "logs/" + name + "/log.txt");
        log->set_level(spdlog::level::trace);
        log->flush_on(spdlog::level::err);
        log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%t] [%s %!:%#] %v");
        // 记录器是自动注册的，不需要手动注册  spdlog::register_logger(name);
    }
    return log;
}

void flush_spdlog() { spdlog::get("logger")->flush(); }