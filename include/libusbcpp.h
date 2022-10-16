#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <optional>
#include <functional>
#include <cinttypes>

#define LIBUSBCPP_DEFAULT_BUFFER_SIZE (1024 * 8)   // Default 8 kB buffer
#define LIBUSBCPP_DEFAULT_TIMEOUT 1000             // Default 1 sec timeout

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

    template<typename T>
    using ref = std::reference_wrapper<T>;

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
    };

	class LIBUSBCPP_API basic_device {
	public:
		explicit basic_device(libusb_device_handle* handle, usb::device_info  info);
		~basic_device();

        device_info device_info;

		bool claim_interface(int _interface);

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

	typedef std::shared_ptr<basic_device> device;

    LIBUSBCPP_API std::vector<device_info> scan_devices(const std::optional<ref<context>>& context = std::nullopt);
    LIBUSBCPP_API usb::device find_device(const context& context, uint16_t vendor_id, uint16_t product_id);

}
