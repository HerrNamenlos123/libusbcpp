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




	// ========================================
	// ===      libusbcpp::context class    ===
	// ========================================

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




	// =======================================
	// ===      libusbcpp::device class    ===
	// =======================================

	class device {
	public:
		device(libusb_device_handle* handle);
		~device();

		void claimInterface(int interface);
		void close();

		deviceInfo getInfo();

		std::vector<uint8_t> bulkRead(size_t expectedLength, uint16_t endpoint, uint32_t timeout = 1000);
		size_t bulkWrite(std::vector<uint8_t> data, uint16_t endpoint, uint32_t timeout = 1000);
		size_t bulkWrite(const std::string& data, uint16_t endpoint, uint32_t timeout = 1000);
		size_t bulkWrite(uint8_t* data, size_t length, uint16_t endpoint, uint32_t timeout = 1000);

		device(device const&) = delete;
		void operator=(device const&) = delete;

	private:
		libusb_device_handle* handle = nullptr;
		std::vector<int> interfaces;
		bool open = true;
		std::mutex mutex;
	};





	// ========================================================
	// ===              HotplugListener class               ===
	// ========================================================

	class hotplugListener {
	public:
		hotplugListener(context& ctx) : context(ctx) {}
		~hotplugListener();

		void start(std::function<void(std::shared_ptr<device> device)> onConnect, float interval = 0.2f);
		void scanOnce(std::function<void(std::shared_ptr<device> device)> onConnect);
		void stop();

	private:
		bool isDeviceKnown(deviceInfo& info);
		void listen(std::function<void(std::shared_ptr<device> device)> onConnect, float interval);

		std::thread listener;
		std::atomic<bool> running = false;
		std::vector<deviceInfo> knownDevices;
		context& context;
		std::mutex mutex;
	};






	// ====================================================
	// ===               General functions              ===
	// ====================================================

	std::vector<deviceInfo> scanDevices(const context& ctx);

	std::shared_ptr<device> openDevice(const context& ctx, uint16_t vendorID, uint16_t productID);

}
