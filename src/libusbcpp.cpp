
#pragma warning( disable : 4200 )
#include "libusb.h"

#include "libusbcpp.h"
#include "magic_enum.hpp"

#define SPDLOG_COMPILED_LIB
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"


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

    std::string_view getErrorMsg(int error) {
        return magic_enum::enum_name((libusb_error)error);
    }





    // ==============================================
    // ===      libusbcpp::basic_context class    ===
    // ==============================================

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

    basic_device::basic_device(libusb_device_handle* handle) {
        this->handle = handle;
        if (!handle) {
            LOG_ERROR("Cannot construct a device with a null handle!");
            throw std::runtime_error("[libusbcpp]: Cannot construct a device with a null handle!");
        }
        LOG_DEBUG("Device constructed with handle {:08X}", (uint64_t)handle);
    }

    basic_device::~basic_device() {
        LOG_DEBUG("Destroying device with handle {:08X}", (uint64_t)handle);
        close();
    }

    bool basic_device::claimInterface(int interface) {
        std::lock_guard<std::mutex> lock(mutex);

        LOG_DEBUG("Device with handle {:08X} is claiming interface {}", (uint64_t)handle, interface);
        if (!open) {
            LOG_ERROR("Failed to claim interface: Device is not open anymore");
            return false;
        }

#ifndef _WIN32
        {        
            int error = libusb_kernel_driver_active(handle, interface);
            if (error == 1) {
                int error = libusb_detach_kernel_driver(handle, interface);
                if (error != LIBUSB_SUCCESS) {
                    LOG_ERROR("Failed to claim interface: Kernel driver was active and it could not be detached: {}", getErrorMsg(error));
                    return false;
                }
            }
            else if (error < 0) {
                LOG_ERROR("Error while checking if the kernel driver is active: {}", getErrorMsg(error));
            }
        }
#endif

        int error = libusb_claim_interface(handle, interface);
        if (error != LIBUSB_SUCCESS) {
            LOG_ERROR("Failed to claim interface: {}", getErrorMsg(error));
            return false;
        }

        interfaces.push_back(interface);
        LOG_DEBUG("Interface claimed successfully");
        return true;
    }

    deviceInfo basic_device::getInfo() {
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
        int error = libusb_get_device_descriptor(dev, &desc);
        if (error != LIBUSB_SUCCESS) {
            LOG_ERROR("Can't retrieve device info: Device descriptor is invalid: {}", getErrorMsg(error));
            return deviceInfo();
        }

        unsigned char buffer[1024];
        LOG_TRACE("Requesting ascii string descriptor");
        int length = libusb_get_string_descriptor_ascii(handle, desc.iProduct, buffer, sizeof(buffer));

        if (length < 0) {
            LOG_ERROR("Can't retrieve device info: Ascii device descriptor is invalid: {}", getErrorMsg(length));
            return deviceInfo();
        }

        deviceInfo device;
        device.vendorID = desc.idVendor;
        device.productID = desc.idProduct;
        device.description = std::string((const char*)buffer, length);
        LOG_TRACE("getInfo done");

        return device;
    }

    std::vector<uint8_t> basic_device::bulkRead(size_t expectedLength, uint16_t endpoint, uint32_t timeout) {
        std::lock_guard<std::mutex> lock(mutex);

        LOG_TRACE("device::bulkRead");
        if (!open) {
            LOG_ERROR("Can't bulkRead: Device is not open anymore");
            return {};
        }

        int transferred = 0;
        std::vector<uint8_t> buffer(expectedLength, 0);
        LOG_TRACE("libusb_bulk_transfer on device {:08X} to endpoint {:02X}", (uint64_t)handle, endpoint);
        int error = libusb_bulk_transfer(handle, (endpoint | LIBUSB_ENDPOINT_IN), &buffer[0], (int)expectedLength, &transferred, timeout);
        if (error == LIBUSB_ERROR_IO) {
            lostConnection();
            return {};
        }
        else if (error != LIBUSB_SUCCESS) {
            LOG_ERROR("Error while reading from device {:08X}: {}", (uint64_t)handle, getErrorMsg(error));
            return {};
        }

        buffer.resize(transferred);
        LOG_TRACE("device::bulkRead done");
        return buffer;
    }

    size_t basic_device::bulkWrite(std::vector<uint8_t> data, uint16_t endpoint, uint32_t timeout) {
        return bulkWrite((uint8_t*)&data[0], data.size(), endpoint, timeout);
    }

    size_t basic_device::bulkWrite(const std::string& data, uint16_t endpoint, uint32_t timeout) {
        return bulkWrite((uint8_t*)data.c_str(), data.length(), endpoint, timeout);
    }

    size_t basic_device::bulkWrite(uint8_t* data, size_t length, uint16_t endpoint, uint32_t timeout) {
        std::lock_guard<std::mutex> lock(mutex);

        LOG_TRACE("device::bulkWrite");
        if (!open) {
            LOG_ERROR("Can't bulkWrite: Device is not open anymore");
            return -1;
        }

        int transferred = 0;
        LOG_TRACE("libusb_bulk_transfer on device {:08X} to endpoint {:02X}", (uint64_t)handle, endpoint);
        int error = libusb_bulk_transfer(handle, (endpoint | LIBUSB_ENDPOINT_OUT), data, (int)length, &transferred, timeout);
        if (error == LIBUSB_ERROR_IO) {
            lostConnection();
            return -1;
        }
        else if (error != LIBUSB_SUCCESS) {
            LOG_ERROR("Error while writing to device {:08X}: {}", (uint64_t)handle, getErrorMsg(error));
            return -1;
        }

        LOG_TRACE("device::bulkWrite done");
        return transferred;
    }

    void basic_device::close() {

        if (open) {
            LOG_DEBUG("Closing device");
            for (int interface : interfaces) {
                LOG_DEBUG("Releasing interface {}", interface);
                int error = libusb_release_interface(handle, interface);
                if (error != LIBUSB_SUCCESS) {
                    LOG_ERROR("Warning while closing device: Can't release interface {}: {}", interface, getErrorMsg(error));
                }
            }
            libusb_close(handle);
            open = false;
            LOG_DEBUG("Device closed");
        }
    }

    void basic_device::lostConnection() {
        LOG_DEBUG("Lost connection to device {:08X}", (uint64_t)handle);
        close();
    }






    // ====================================================
    // ===               General functions              ===
    // ====================================================

    std::vector<device> findDevice(const context& ctx, uint16_t vendorID, uint16_t productID) {
        std::lock_guard<std::mutex> lock(scanMutex);
        LOG_DEBUG("Looking for device vid={:04X} pid={:04X}", vendorID, productID);

        libusb_device** rawDeviceList;
        LOG_TRACE("Requesting device list");
        size_t count = libusb_get_device_list((libusb_context*)ctx, &rawDeviceList);

        if (count < 0) {
            LOG_ERROR("Failed to retrieve device list: {}", getErrorMsg((int)count));
            return {};
        }

        std::vector<device> devices;
        for (size_t i = 0; i < count; i++) {
            libusb_device* rawDevice = rawDeviceList[i];

            // Get device information
            struct libusb_device_descriptor desc;
            LOG_TRACE("Requesting device descriptor");
            int error = libusb_get_device_descriptor(rawDevice, &desc);
            if (error != LIBUSB_SUCCESS) {
                LOG_TRACE("Failed to retrieve device descriptor, skipping device entry: {}", getErrorMsg(error));
                continue;   // Jump back to top
            }

            if (desc.idVendor != vendorID || desc.idProduct != productID) {
                continue;   // Jump back to top
            }

            // Open the device
            libusb_device_handle* handle = nullptr;
            LOG_TRACE("Trying to open device");
            error = libusb_open(rawDevice, &handle);
            if (error != LIBUSB_SUCCESS) {
                LOG_TRACE("Failed: Device vid={:04X} pid={:04X} was found, but could not be opened: {}", vendorID, productID, getErrorMsg(error));
                continue;   // Jump back to top
            }

            LOG_DEBUG("Device opened successfully");
            devices.emplace_back(std::make_shared<basic_device>(handle));
        }

        LOG_TRACE("Freeing device list");
        libusb_free_device_list(rawDeviceList, 1);

        LOG_DEBUG("Opened {} devices", devices.size());
        return devices;
    }




}
