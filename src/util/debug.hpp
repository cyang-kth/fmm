/**
 * Content
 * Debug information used in development
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_DEBUG_HPP
#define MM_DEBUG_HPP

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h" // must be included for custom operator
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h" // support for basic file logging
#include "spdlog/sinks/rotating_file_sink.h"
#include <string>


namespace MM {

static const std::vector<std::string>
    LOG_LEVESLS {"0-trace","1-debug","2-info",
                 "3-warn","4-err","5-critical","6-off"};

}

#endif // MM_DEBUG_HPP
