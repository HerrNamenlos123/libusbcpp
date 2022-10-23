#pragma once

//#define __LOG_COLOR_RED "1;91"
//#define __LOG_COLOR_GREEN "0;92"
//#define __LOG_COLOR_BLUE "1;94"
//#define __LOG_COLOR_YELLOW "0;93"
//#define __LOG_COLOR_WHITE "0;97"
//#define __LOG_COLOR(color, msg, ...) printf("\033[%sm" msg "\033[m\n", color, ##__VA_ARGS__)

#define __LOG_COLOR(msg, ...) if (usb::__enable_logging) printf(msg"\n", ##__VA_ARGS__)

#define LOG(fmt, ...) if (usb::__enable_logging) printf("[libusbcpp] " fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) __LOG_COLOR("[libusbcpp] DEBUG: " fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) __LOG_COLOR("[libusbcpp] INFO: " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) __LOG_COLOR("[libusbcpp] WARNING: " fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) __LOG_COLOR("[libusbcpp] ERROR: " fmt, ##__VA_ARGS__)
//#define LOG_DEBUG(fmt, ...) __LOG_COLOR(__LOG_COLOR_BLUE, "[libusbcpp] " fmt, ##__VA_ARGS__)
//#define LOG_INFO(fmt, ...) __LOG_COLOR(__LOG_COLOR_GREEN, "[libusbcpp] " fmt, ##__VA_ARGS__)
//#define LOG_WARN(fmt, ...) __LOG_COLOR(__LOG_COLOR_YELLOW, "[libusbcpp] " fmt, ##__VA_ARGS__)
//#define LOG_ERROR(fmt, ...) __LOG_COLOR(__LOG_COLOR_RED, "[libusbcpp] " fmt, ##__VA_ARGS__)

namespace usb {
    inline bool __enable_logging = false;
}
