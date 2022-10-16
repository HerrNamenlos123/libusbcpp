#pragma once

#define __LOG_COLOR_RED "1;91"
#define __LOG_COLOR_GREEN "0;92"
#define __LOG_COLOR_BLUE "1;94"
#define __LOG_COLOR_YELLOW "0;93"
#define __LOG_COLOR_WHITE "0;97"
#define __LOG_COLOR(color, msg, ...) printf("\033[%sm" msg "\033[m\n", color, ##__VA_ARGS__)

#ifndef LIBUSBCPP_VERBOSE_LOGGING
    #define LOG(fmt, ...) printf("[libusbcpp] " fmt, ##__VA_ARGS__)
    #define LOG_DEBUG(fmt, ...) __LOG_COLOR(__LOG_COLOR_BLUE, "[libusbcpp] " fmt, ##__VA_ARGS__)
    #define LOG_INFO(fmt, ...) __LOG_COLOR(__LOG_COLOR_GREEN, "[libusbcpp] " fmt, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...) __LOG_COLOR(__LOG_COLOR_YELLOW, "[libusbcpp] " fmt, ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) __LOG_COLOR(__LOG_COLOR_RED, "[libusbcpp] " fmt, ##__VA_ARGS__)
#else
#define PRINTF(fmt, ...)
    #define PRINTF_DEBUG(fmt, ...)
    #define PRINTF_INFO(fmt, ...)
#endif
