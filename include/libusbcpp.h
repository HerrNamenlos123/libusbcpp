#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <optional>
#include <queue>
#include <functional>
#include <cinttypes>
#include <atomic>

#define LIBUSBCPP_DEFAULT_BUFFER_SIZE (1024 * 8)        // [bytes] Default 8 kB buffer
#define LIBUSBCPP_DEFAULT_TIMEOUT 1000                  // [ms]
#define LIBUSBCPP_DEFAULT_HOTPLUG_RESCAN_INTERVAL 1000  // [ms]

#ifndef LIBUSBCPP_STATIC_LIB
    #ifdef LIBUSBCPP_EXPORTS
        #define LIBUSBCPP_API __declspec(dllexport)
    #else
        #define LIBUSBCPP_API __declspec(dllimport)
    #endif
#else
    #define LIBUSBCPP_API
#endif

struct libusb_context;          // Forward declarations
struct libusb_device_handle;

namespace usb {

    enum class state {
        CLOSED,
        IN_USE_OR_UNSUPPORTED,
        INVALID_DRIVER,
        OTHER_LIBUSB_ERROR,
        OPEN
    };

    LIBUSBCPP_API const char* state_str(enum state state);
    LIBUSBCPP_API void enable_logging(bool enable = true);

    class LIBUSBCPP_API context {
    public:
        context();
        ~context();

        operator libusb_context*() const;

        context(context const&) = delete;           // Copying prohibited
        void operator=(context const&) = delete;

        context(context&&) = default;               // Moving allowed
        context& operator=(context&&) = default;

    private:
        libusb_context* _context = nullptr;
    };

    struct LIBUSBCPP_API device_info {
        uint16_t vendor_id = 0x00;
        uint16_t product_id = 0x00;
        std::string description;
        enum state state = usb::state::CLOSED;

        bool operator==(const device_info& other) {
            return vendor_id == other.vendor_id && product_id == other.product_id;
        }
    };

	class LIBUSBCPP_API basic_device {
	public:
		explicit basic_device(libusb_device_handle* handle, usb::device_info  info);
		~basic_device();

        device_info info;

		bool claim_interface(int _interface);
        bool is_open();
        libusb_device_handle* get_handle();

        std::string bulk_read(uint16_t endpoint,
                              size_t max_buffer_size = LIBUSBCPP_DEFAULT_BUFFER_SIZE,
                              uint32_t timeout = LIBUSBCPP_DEFAULT_TIMEOUT);

        // These return 0 on error
		size_t bulk_write(std::vector<uint8_t> data, uint16_t endpoint, uint32_t timeout = LIBUSBCPP_DEFAULT_TIMEOUT);
		size_t bulk_write(const std::string& data, uint16_t endpoint, uint32_t timeout = LIBUSBCPP_DEFAULT_TIMEOUT);
		size_t bulk_write(uint8_t* data, size_t length, uint16_t endpoint, uint32_t timeout = LIBUSBCPP_DEFAULT_TIMEOUT);

		basic_device(basic_device const&) = delete;
        basic_device& operator=(basic_device const&) = delete;

	private:

        size_t bulk_transfer(uint16_t endpoint, unsigned char* buffer, size_t max_buffer_size, uint32_t timeout);

		void close();
		void lost_connection();
        bool detach_kernel_driver(int _interface);

		libusb_device_handle* handle = nullptr;
        std::vector<int> interfaces;

		std::mutex mutex;   // Lock for all callable functions
	};



    template<typename T>
    using ref = std::reference_wrapper<T>;
    using opt_context = std::optional<ref<context>>;
    typedef std::shared_ptr<basic_device> device;
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;




    class LIBUSBCPP_API generic_hotplug_handler {
    public:

        int interval = 0;

        explicit generic_hotplug_handler(opt_context context = std::nullopt, int interval = LIBUSBCPP_DEFAULT_HOTPLUG_RESCAN_INTERVAL);

        void register_device_callback(const std::function<void(usb::device)>& callback);
        void register_device_callback(uint16_t vendor_id, uint16_t product_id, const std::function<void(usb::device)>& callback);

        void update();

    private:
        void update_devices();

        opt_context context = std::nullopt;

        std::function<void(usb::device)> callback;
        uint16_t callback_vid = -1;
        uint16_t callback_pid = -1;

        std::vector<device_info> old_devices;   // Storing what devices are already known to the system
        timepoint last_update;
    };

    class LIBUSBCPP_API hotplug_handler {
    public:

        explicit hotplug_handler(opt_context context = std::nullopt, int interval = LIBUSBCPP_DEFAULT_HOTPLUG_RESCAN_INTERVAL);
        ~hotplug_handler();

        void register_device_callback(const std::function<void(usb::device)>& callback);
        void register_device_callback(uint16_t vendor_id, uint16_t product_id, const std::function<void(usb::device)>& callback);

        void update();

        void run_async();
        void stop_async();

    private:
        std::atomic<bool> terminate = false;
        std::thread thread;
        int interval = 0;

        generic_hotplug_handler handler;
    };



    LIBUSBCPP_API std::vector<device_info> scan_devices(opt_context context = std::nullopt);
    LIBUSBCPP_API std::vector<usb::device> find_devices(uint16_t vendor_id, uint16_t product_id, opt_context context = std::nullopt);
    LIBUSBCPP_API std::vector<usb::device> find_valid_devices(uint16_t vendor_id, uint16_t product_id, opt_context context = std::nullopt);
    LIBUSBCPP_API usb::device find_first_device(uint16_t vendor_id, uint16_t product_id, opt_context context = std::nullopt);

}
