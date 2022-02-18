
#pragma warning( disable : 4200 )
#include "libusb.h"

#include "libusbcpp.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <stdexcept>


#define LIBUSBCPP_DEFAULT_LOG_LEVEL spdlog::level::warn


// =================================
// ===      libusbcpp logging    ===
// =================================

#ifndef LIBUSBCPP_NO_LOGGING

#define LOG_SET_LOGLEVEL(...)	libusbcpp::logger->set_level(__VA_ARGS__)

#define INIT_LOGGER()			{	if (!libusbcpp::logger) {	\
										spdlog::set_pattern("%^[%n]: %v%$"); \
										libusbcpp::logger = spdlog::stdout_color_mt("libusbcpp"); \
										LOG_SET_LOGLEVEL(LIBUSBCPP_DEFAULT_LOG_LEVEL); \
									} \
								}

#define LOG_TRACE(...)			{ INIT_LOGGER(); libusbcpp::logger->trace(__VA_ARGS__);		}
#define LOG_WARN(...)			{ INIT_LOGGER(); libusbcpp::logger->warn(__VA_ARGS__);		}
#define LOG_DEBUG(...)			{ INIT_LOGGER(); libusbcpp::logger->debug(__VA_ARGS__);		}
#define LOG_INFO(...)			{ INIT_LOGGER(); libusbcpp::logger->info(__VA_ARGS__);		}
#define LOG_ERROR(...)			{ INIT_LOGGER(); libusbcpp::logger->error(__VA_ARGS__);		}
#define LOG_CRITICAL(...)		{ INIT_LOGGER(); libusbcpp::logger->critical(__VA_ARGS__);	}

namespace libusbcpp {
    std::shared_ptr<spdlog::logger> logger;

    void setLogLevel(enum LogLevel logLevel) {
        INIT_LOGGER();

        switch (logLevel) {
        case LOG_LEVEL_TRACE:       LOG_SET_LOGLEVEL(spdlog::level::trace);     break;
        case LOG_LEVEL_DEBUG:       LOG_SET_LOGLEVEL(spdlog::level::debug);     break;
        case LOG_LEVEL_INFO:        LOG_SET_LOGLEVEL(spdlog::level::info);      break;
        case LOG_LEVEL_WARN:        LOG_SET_LOGLEVEL(spdlog::level::warn);      break;
        case LOG_LEVEL_ERROR:       LOG_SET_LOGLEVEL(spdlog::level::err);       break;
        case LOG_LEVEL_CRITICAL:    LOG_SET_LOGLEVEL(spdlog::level::critical);  break;
        }
    }
}

#else

#define LOG_SET_BATTERY_LOGLEVEL(...)	{ ; }
#define LOG_SET_CLIENT_LOGLEVEL(...)	{ ; }
#define LOG_SET_LOGLEVEL(...)			{ ; }

#define LOG_CORE_TRACE(...)				{ ; }
#define LOG_CORE_WARN(...)				{ ; }
#define LOG_CORE_DEBUG(...)				{ ; }
#define LOG_CORE_INFO(...)				{ ; }
#define LOG_CORE_ERROR(...)				{ ; }
#define LOG_CORE_CRITICAL(...)			{ ; }
#define LOG_TRACE(...)					{ ; }
#define LOG_WARN(...)					{ ; }
#define LOG_DEBUG(...)					{ ; }
#define LOG_INFO(...)					{ ; }
#define LOG_ERROR(...)					{ ; }
#define LOG_CRITICAL(...)				{ ; }

#endif





namespace libusbcpp {

    std::mutex scanMutex;



    // ========================================
    // ===      libusbcpp::context class    ===
    // ========================================

    context::context() {
        LOG_DEBUG("Creating libusb context");
        if (libusb_init(&_context) < 0) {
            LOG_ERROR("Failed to init libusb context");
            throw std::runtime_error("[libusbcpp]: libusb could not be initialized!");
        }
    }

    context::~context() {
        LOG_DEBUG("Destroying libusb context");
        libusb_exit(_context);
    }

    context::operator libusb_context*() const {
        return _context;
    }






    // ====================================================
    // ===                Device class                  ===
    // ====================================================

    device::device(libusb_device_handle* handle) {
        this->handle = handle;
        if (!handle) {
            LOG_ERROR("Cannot construct a device with a null handle!");
            throw std::runtime_error("[libusbcpp]: Cannot construct a device with a null handle!");
        }
        LOG_DEBUG("Device constructed with handle {:08X}", (uint64_t)handle);
    }

    device::~device() {
        LOG_DEBUG("Destroying device with handle {:08X}", (uint64_t)handle);
        close();
    }

    void device::claimInterface(int interface) {
        std::lock_guard<std::mutex> lock(mutex);

        LOG_DEBUG("Device with handle {:08X} is claiming interface {}", (uint64_t)handle, interface);
        if (!open) {
            LOG_DEBUG("Device is not open anymore, returning");
            return;
        }

        if (libusb_kernel_driver_active(handle, interface) == 1) {
            if (libusb_detach_kernel_driver(handle, interface) != LIBUSB_SUCCESS) {
                LOG_ERROR("Failed to claim interface: Kernel driver was active and it could not be detached");
                throw std::runtime_error("[libusbcpp]: Failed to claim interface: Can't detach Kernel driver");
            }
        }

        if (libusb_claim_interface(handle, interface) < 0) {
            LOG_ERROR("Failed to claim interface: Permission denied");
            throw std::runtime_error("[libusbcpp]: Failed to claim interface: Permission denied");
        }

        interfaces.push_back(interface);
        LOG_DEBUG("Interface claimed successfully");
    }

    void device::close() {
        std::lock_guard<std::mutex> lock(mutex);

        if (open) {
            return;
        }

        LOG_DEBUG("Closing device");
        for (int interface : interfaces) {
            LOG_DEBUG("Releasing interface {}", interface);
            libusb_release_interface(handle, interface);
        }
        libusb_close(handle);
        open = false;
        LOG_DEBUG("Device closed");
    }

    deviceInfo device::getInfo() {
        std::lock_guard<std::mutex> lock(mutex);

        LOG_TRACE("Requesting device getInfo");
        if (!open) {
            LOG_ERROR("Can't retrieve device info: Device is not open anymore");
            return deviceInfo();
        }

        LOG_TRACE("Requesting device struct from handle");
        libusb_device* dev = libusb_get_device(handle);
        if (dev == nullptr) {
            LOG_ERROR("Can't retrieve device info: Device struct is invalid");
            return deviceInfo();
        }

        struct libusb_device_descriptor desc;
        LOG_TRACE("Requesting device descriptor");
        if (libusb_get_device_descriptor(dev, &desc) != LIBUSB_SUCCESS) {
            LOG_ERROR("Can't retrieve device info: Device descriptor is invalid");
            return deviceInfo();
        }

        unsigned char buffer[1024];
        LOG_TRACE("Requesting ascii string descriptor");
        size_t length = libusb_get_string_descriptor_ascii(handle, desc.iProduct, buffer, sizeof(buffer));

        deviceInfo device;
        device.vendorID = desc.idVendor;
        device.productID = desc.idProduct;
        device.description = std::string((const char*)buffer, length);
        LOG_TRACE("getInfo done");

        return device;
    }

    std::vector<uint8_t> device::bulkRead(size_t expectedLength, uint16_t endpoint, uint32_t timeout) {
        std::lock_guard<std::mutex> lock(mutex);

        LOG_TRACE("device::bulkRead");
        if (!open) {
            LOG_ERROR("Can't bulkRead: Device is not open anymore");
            return {};
        }

        int transferred = 0;
        std::vector<uint8_t> buffer(expectedLength, 0);
        LOG_TRACE("libusb_bulk_transfer on device {:08X} to endpoint {:02X}", (uint64_t)handle, endpoint);
        if (libusb_bulk_transfer(handle, (endpoint | LIBUSB_ENDPOINT_IN), &buffer[0], (int)expectedLength, &transferred, timeout) != LIBUSB_SUCCESS) {
            LOG_ERROR("Error while reading from device {:08X}", (uint64_t)handle);
            return {};
        }

        buffer.resize(transferred);
        LOG_TRACE("device::bulkRead done");
        return buffer;
    }

    size_t device::bulkWrite(std::vector<uint8_t> data, uint16_t endpoint, uint32_t timeout) {
        return bulkWrite((uint8_t*)&data[0], data.size(), endpoint, timeout);
    }

    size_t device::bulkWrite(const std::string& data, uint16_t endpoint, uint32_t timeout) {
        return bulkWrite((uint8_t*)data.c_str(), data.length(), endpoint, timeout);
    }

    size_t device::bulkWrite(uint8_t* data, size_t length, uint16_t endpoint, uint32_t timeout) {
        std::lock_guard<std::mutex> lock(mutex);

        LOG_TRACE("device::bulkWrite");
        if (!open) {
            LOG_ERROR("Can't bulkWrite: Device is not open anymore");
            return -1;
        }

        int transferred = 0;
        LOG_TRACE("libusb_bulk_transfer on device {:08X} to endpoint {:02X}", (uint64_t)handle, endpoint);
        int errorCode = libusb_bulk_transfer(handle, (endpoint | LIBUSB_ENDPOINT_OUT), data, (int)length, &transferred, timeout);
        if (errorCode != LIBUSB_SUCCESS) {
            LOG_ERROR("Error while writing to device {:08X}", (uint64_t)handle);
            return -1;
        }

        LOG_TRACE("device::bulkWrite done");
        return transferred;
    }








    // ========================================================
    // ===              HotplugListener class               ===
    // ========================================================

    hotplugListener::~hotplugListener() {
        stop();
    }

    void hotplugListener::start(std::function<void(std::shared_ptr<device> device)> onConnect, float interval) {
        using namespace std::placeholders;

        running = true;
        LOG_TRACE("Hotpluglistener: Starting listener, detaching thread");
        listener = std::thread(std::bind(&hotplugListener::listen, this, _1, _2), onConnect, interval);
    }

    void hotplugListener::scanOnce(std::function<void(std::shared_ptr<device> device)> onConnect) {
        std::lock_guard<std::mutex> lock(mutex);

        try {
            LOG_TRACE("HotplugListener: Scanning for devices");
            auto newDevices = scanDevices(context);
            for (auto& device : newDevices) {
                if (!isDeviceKnown(device)) {
                    LOG_DEBUG("HotplugListener: Found device vid={:04X} pid={:04X}, trying to connect", device.vendorID, device.productID);
                    auto dev = openDevice(context, device.vendorID, device.productID);
                    if (dev) {
                        onConnect(dev);
                        LOG_TRACE("HotplugListener: User callback finished");
                    }
                    else {
                        LOG_ERROR("HotplugListener: Cannot open device vid={:04X} pid={:04X}", device.vendorID, device.productID);
                    }
                }
            }
            LOG_TRACE("HotplugListener: Scan finished");

            knownDevices.clear();
            knownDevices = std::move(newDevices);
        }
        catch (const std::exception& e) {
            LOG_ERROR("HotplugListener: Thread exception: {}", e.what());
        }
    }

    void hotplugListener::stop() {
        if (running) {
            running = false;
            LOG_TRACE("Hotpluglistener: Stopping and joining thread");
            listener.join();
        }
    }

    bool hotplugListener::isDeviceKnown(deviceInfo& info) {
        for (auto& device : knownDevices) {
            if (device.vendorID == info.vendorID && device.productID == info.productID) {
                return true;
            }
        }

        return false;
    }

    void hotplugListener::listen(std::function<void(std::shared_ptr<device> device)> onConnect, float interval) {

        LOG_TRACE("Hotpluglistener: Thread started");
        while (running) {

            LOG_TRACE("Hotpluglistener: Scanning once");
            scanOnce(onConnect);

            LOG_TRACE("Hotpluglistener: Scanning done, sleeping {}s", interval);
            std::this_thread::sleep_for(std::chrono::milliseconds((uint64_t)(interval * 1000.f)));

        }
        LOG_TRACE("Hotpluglistener: Thread stopped");
    }






    // ====================================================
    // ===               General functions              ===
    // ====================================================

    std::vector<deviceInfo> scanDevices(const context& ctx) {
        std::lock_guard<std::mutex> lock(scanMutex);
        LOG_DEBUG("Scanning for devices...");

        libusb_device** rawDeviceList;
        LOG_TRACE("Requesting device list");
        size_t count = libusb_get_device_list(ctx, &rawDeviceList);

        if (count < 0) {
            LOG_ERROR("Failed to retrieve device list");
            return {};
        }

        std::vector<deviceInfo> devices;
        for (size_t i = 0; i < count; i++) {
            libusb_device* rawDevice = rawDeviceList[i];

            // Get device information
            struct libusb_device_descriptor desc;
            LOG_TRACE("Requesting device descriptor");
            if (libusb_get_device_descriptor(rawDevice, &desc) != LIBUSB_SUCCESS) {
                LOG_TRACE("Failed to retrieve device descriptor, skipping device entry");
                continue;   // Jump back to top
            }

            // Open device for reading
            libusb_device_handle* handle = NULL;
            LOG_TRACE("Trying to open device");
            if (libusb_open(rawDevice, &handle) != LIBUSB_SUCCESS) {
                LOG_TRACE("Failed to open device vid={:04X} pid={:04X}, skipping device entry", desc.idVendor, desc.idProduct);
                continue;   // Jump back to top
            }

            // Parse the description text
            unsigned char buffer[1024];
            LOG_TRACE("Requesting ascii string descriptor");
            size_t length = libusb_get_string_descriptor_ascii(handle, desc.iProduct, buffer, sizeof(buffer));

            LOG_TRACE("Closing device");
            libusb_close(handle);

            if (length < 0) {
                LOG_TRACE("Ascii string descriptor is invalid, skipping device entry");
                continue;
            }

            deviceInfo device;
            device.vendorID = desc.idVendor;
            device.productID = desc.idProduct;
            device.description = std::string((const char*)buffer, length);

            devices.push_back(std::move(device));
        }

        LOG_TRACE("Freeing list");
        libusb_free_device_list(rawDeviceList, 1);

        LOG_DEBUG("Scanning done, found {} devices", devices.size());
        return devices;
    }

    std::shared_ptr<device> openDevice(const context& ctx, uint16_t vendorID, uint16_t productID) {
        std::lock_guard<std::mutex> lock(scanMutex);
        LOG_DEBUG("Opening device vid={:04X} pid={:04X}", vendorID, productID);

        libusb_device** rawDeviceList;
        LOG_TRACE("Requesting device list");
        size_t count = libusb_get_device_list(ctx, &rawDeviceList);

        if (count < 0) {
            LOG_ERROR("Failed to retrieve device list");
            return nullptr;
        }

        for (size_t i = 0; i < count; i++) {
            libusb_device* rawDevice = rawDeviceList[i];

            // Get device information
            struct libusb_device_descriptor desc;
            LOG_TRACE("Requesting device descriptor");
            if (libusb_get_device_descriptor(rawDevice, &desc) != LIBUSB_SUCCESS) {
                LOG_TRACE("Failed to retrieve device descriptor, skipping device entry");
                continue;   // Jump back to top
            }

            if (desc.idVendor != vendorID || desc.idProduct != productID) {
                continue;   // Jump back to top
            }

            // Open the device
            libusb_device_handle* handle = nullptr;
            LOG_TRACE("Trying to open device");
            if (libusb_open(rawDevice, &handle) < 0) {
                LOG_TRACE("Failed: Device vid={:04X} pid={:04X} was found, but could not be opened", vendorID, productID);
                continue;   // Jump back to top
            }

            // Device is opened, return it
            LOG_TRACE("Freeing device list");
            libusb_free_device_list(rawDeviceList, 1);
            LOG_DEBUG("Device opened successfully");
            return std::make_shared<device>(handle);
        }

        LOG_TRACE("Freeing device list");
        libusb_free_device_list(rawDeviceList, 1);

        LOG_WARN("Cannot open device vid={:04X} pid={:04X}: No valid candidate", vendorID, productID);
        return nullptr;
    }




}
