#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <chrono>

#include "plugin.h"
#include "spdlog/sinks/base_sink.h"

using spdlog_base_sink = spdlog::sinks::base_sink<std::mutex>;

class PluginSink : public  spdlog_base_sink
{
public:

    PluginSink(loggerfunc_t logger, int pluginHandle): spdlog_base_sink()
    {
        this->ns_logger_ = logger;
        this->pluginHandle = pluginHandle; 
    }

    void sink_it_(const spdlog::details::log_msg& in_msg) override
    {
        LogMsg msg{};
        std::string payload(in_msg.payload.data(), in_msg.payload.size());
        msg.level = (int)in_msg.level;
        msg.msg = payload.c_str();
        msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(in_msg.time.time_since_epoch()).count();
        msg.source.file = in_msg.source.filename;
        msg.source.func = in_msg.source.funcname;
        msg.source.line = in_msg.source.line;
        msg.pluginHandle = this->pluginHandle;
        this->ns_logger_(&msg);
    }

    void flush_() override
    {
        std::cout << std::flush;
    }

protected:
    loggerfunc_t ns_logger_;
    int pluginHandle = 0;

    // sink log level - default is all
    spdlog::level_t level_{ spdlog::level::trace };
};

#endif