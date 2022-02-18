#pragma once

#include "inttypes.h"

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <functional>

struct libusb_context;
struct libusb_device_handle;




// ==========================
// ===      Logging       ===
// ==========================

#ifndef LIBUSBCPP_NO_LOGGING
namespace libusbcpp {

	enum LogLevel {
		LOG_LEVEL_TRACE,
		LOG_LEVEL_DEBUG,
		LOG_LEVEL_INFO,
		LOG_LEVEL_WARN,
		LOG_LEVEL_ERROR,
		LOG_LEVEL_CRITICAL
	};

	void setLogLevel(enum LogLevel logLevel);
}
#endif






namespace libusbcpp {



	// ============================================
	// ===      libusbcpp::deviceInfo struct    ===
	// ============================================

	struct deviceInfo {
		uint16_t vendorID = 0x00;
		uint16_t productID = 0x00;
		std::string description;
	};




	// ==============================================
	// ===      libusbcpp::basic_context class    ===
	// ==============================================

    class context {
    public:
        context();
        ~context();

        operator libusb_context*() const;

        context(context const&) = delete;
        void operator=(context const&) = delete;

    private:
        libusb_context* _context = nullptr;
    };




	// =============================================
	// ===      libusbcpp::basic_device class    ===
	// =============================================

	class basic_device {
	public:
		basic_device(libusb_device_handle* handle);
		~basic_device();

		bool claimInterface(int interface);

		deviceInfo getInfo();

		std::vector<uint8_t> bulkRead(size_t expectedLength, uint16_t endpoint, uint32_t timeout = 1000);
		size_t bulkWrite(std::vector<uint8_t> data, uint16_t endpoint, uint32_t timeout = 1000);
		size_t bulkWrite(const std::string& data, uint16_t endpoint, uint32_t timeout = 1000);
		size_t bulkWrite(uint8_t* data, size_t length, uint16_t endpoint, uint32_t timeout = 1000);

		basic_device(basic_device const&) = delete;
		void operator=(basic_device const&) = delete;

	private:
		void close();
		void lostConnection();

		libusb_device_handle* handle = nullptr;
		std::vector<int> interfaces;
		bool open = true;
		std::mutex mutex;
	};

	typedef std::shared_ptr<basic_device> device;






	// ====================================================
	// ===               General functions              ===
	// ====================================================

	std::vector<device> findDevice(const context& ctx, uint16_t vendorID, uint16_t productID);

}
