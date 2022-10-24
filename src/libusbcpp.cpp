
#include <utility>
#include "libusb.h"
#include "log.h"

#define LIBUSBCPP_EXPORTS
#include "libusbcpp.h"

#define MAKE_EXCEPTION(msg) std::runtime_error("[libusbcpp] " msg)
#define THROW_AND_LOG(msg) LOG_ERROR("Exception: " msg); throw MAKE_EXCEPTION(msg)

namespace usb {

    LIBUSBCPP_API const char* state_str(enum state state) {
        switch (state) {
            case usb::state::CLOSED:
                return "CLOSED";
            case state::IN_USE_OR_UNSUPPORTED:
                return "IN_USE_OR_UNSUPPORTED";
            case state::INVALID_DRIVER:
                return "INVALID_DRIVER";
            case state::OPEN:
                return "OPEN";
            default:
                return "INVALID_STATE";
        }
    }

    LIBUSBCPP_API void enable_logging(bool enable) {
        __enable_logging = enable;
    }

    LIBUSBCPP_API context::context() {
        LOG_DEBUG("Creating libusb context");
        if (libusb_init(&_context) < 0) {
            LOG_ERROR("Failed to init libusb context");
            throw std::runtime_error("[libusbcpp]: libusb could not be initialized!");
        }
    }

    LIBUSBCPP_API context::~context() {
        LOG_DEBUG("Destroying libusb context");
        libusb_exit(_context);
    }

    LIBUSBCPP_API context::operator libusb_context*() const {
        return _context;
    }







    LIBUSBCPP_API basic_device::basic_device(libusb_device_handle* handle, usb::device_info info)
      : handle(handle), info(std::move(info)
    ) {
        if (!handle) {
            close();
        }
    }

    LIBUSBCPP_API basic_device::~basic_device() {
        close();
    }

    LIBUSBCPP_API bool basic_device::claim_interface(int _interface) {
        std::lock_guard<std::mutex> lock(mutex);

        if (!handle) {
            LOG_ERROR("Cannot claim interface: Device is not open");
            return false;
        }

#ifndef _WIN32  // Only on Linux
        if (!detach_kernel_driver(interface))
            return false;
#endif

        int status = libusb_claim_interface(handle, _interface);
        if (status != LIBUSB_SUCCESS) {
            LOG_ERROR("Failed to claim interface: %s", libusb_strerror(status));
            return false;
        }

        interfaces.push_back(_interface);
        return true;
    }

    LIBUSBCPP_API bool basic_device::is_open() {
        return (bool)handle;
    }

    LIBUSBCPP_API libusb_device_handle* basic_device::get_handle() {
        return handle;
    }

    LIBUSBCPP_API std::string basic_device::bulk_read(uint16_t endpoint, size_t max_buffer_size, uint32_t timeout) {
        std::string buffer(max_buffer_size, 0);     // Provide a string with zeros as a buffer
        size_t transferred = bulk_transfer(endpoint | LIBUSB_ENDPOINT_IN,
                                           reinterpret_cast<unsigned char *>(&buffer[0]),
                                           buffer.capacity(), timeout);
        if (transferred == -1) {  // Error
            return {};
        }

        buffer.resize(transferred);
        return buffer;
    }

    LIBUSBCPP_API size_t basic_device::bulk_write(std::vector<uint8_t> data, uint16_t endpoint, uint32_t timeout) {
        return bulk_write((uint8_t*)&data[0], data.size(), endpoint, timeout);
    }

    LIBUSBCPP_API size_t basic_device::bulk_write(const std::string& data, uint16_t endpoint, uint32_t timeout) {
        return bulk_write((uint8_t*)data.c_str(), data.length(), endpoint, timeout);
    }

    LIBUSBCPP_API size_t basic_device::bulk_write(uint8_t* data, size_t length, uint16_t endpoint, uint32_t timeout) {
        size_t transferred = bulk_transfer(endpoint | LIBUSB_ENDPOINT_OUT,
                                           data,length, timeout);
        if (transferred == -1) {  // Error
            return 0;
        }
        return transferred;
    }

    LIBUSBCPP_API size_t basic_device::bulk_transfer(uint16_t endpoint, unsigned char* buffer,
                                       size_t max_buffer_size, uint32_t timeout) {
        std::lock_guard<std::mutex> lock(mutex);

        if (!handle) {
            LOG_ERROR("Cannot bulk_read: Device is not open");
            return -1;
        }

        int transferred = 0;
        int status = libusb_bulk_transfer(handle, endpoint,buffer,
                                          (int)max_buffer_size, &transferred, timeout);
        if (status == LIBUSB_ERROR_IO) {
            lost_connection();
            return -1;
        }
        else if (status != LIBUSB_SUCCESS) {
            LOG_ERROR("Error occurred during bulk read: %s", libusb_strerror(status));
            return -1;
        }

        return transferred;
    }

    LIBUSBCPP_API void basic_device::close() {

        if (handle) {
            for (int interface : interfaces) {
                int error = libusb_release_interface(handle, interface);
                if (error != LIBUSB_SUCCESS) {
                    LOG_WARN("Warning: Could not release interface %d while closing device: %s",
                              interface, libusb_strerror(error));
                }
            }
            libusb_close(handle);
            handle = nullptr;
        }

        if (info.state == state::OPEN)
            info.state = state::CLOSED;
    }

    LIBUSBCPP_API void basic_device::lost_connection() {
        LOG_DEBUG("Lost connection to device %016llX", (uint64_t)handle);
        close();
    }

    LIBUSBCPP_API bool basic_device::detach_kernel_driver(int _interface) {
        int status = libusb_kernel_driver_active(handle, _interface);
        if (status < 0)
            LOG_ERROR("Cannot check if a kernel driver is active: %s", libusb_strerror(status));

        if (status == 0)
            return true;     // Everything is fine, no driver active

        // Kernel driver is active, we need to kill it
        int error = libusb_detach_kernel_driver(handle, _interface);
        if (error != LIBUSB_SUCCESS) {
            LOG_ERROR("Failed to detach kernel driver: %s", libusb_strerror(error));
            return false;
        }
        return true;
    }








    LIBUSBCPP_API generic_hotplug_handler::generic_hotplug_handler(opt_context context, int interval)
      : context(context), interval(interval) {

    }

    LIBUSBCPP_API void generic_hotplug_handler::register_device_callback(const std::function<void(usb::device)>& _callback) {
        register_device_callback(-1, -1, _callback);
    }

    LIBUSBCPP_API void generic_hotplug_handler::register_device_callback(
            uint16_t vendor_id, uint16_t product_id, const std::function<void(usb::device)>& _callback) {
        this->callback = _callback;
        this->callback_vid = vendor_id;
        this->callback_pid = product_id;
    }

    LIBUSBCPP_API void generic_hotplug_handler::update() {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update).count();
        if (elapsed_ms < interval)
            return;
        last_update = now;

        update_devices();   // Only update in the specified interval
    }

    LIBUSBCPP_API void generic_hotplug_handler::update_devices() {

        if (!callback)  // If no callback, don't do anything
            return;

        // Scan for new devices
        auto devices_found = scan_devices(context);

        // If usb id is specified, immediately open device no matter what (just try to open the device)
        if (callback_vid != (uint16_t)-1 || callback_pid != (uint16_t)-1) {
            for (auto& dev : devices_found) {
                if (dev.vendor_id == callback_vid && dev.product_id == callback_pid) {  // Our usb id exists
                    if (dev.state != state::IN_USE_OR_UNSUPPORTED) {    // Not if it is already in use
                        // So, a device with our usb id exists, which is not yet in use
                        usb::device device = find_first_device(dev.vendor_id, dev.product_id, context);
                        if (device) {
                            callback(device);
                        }
                    }
                }
            }
            return;
        }

        // Otherwise, uo usb id is specified, only handle callback for a device we do not know yet

        // Check for new devices
        std::vector<device_info> new_devices;
        for (auto& info : devices_found) {
            if(std::find(old_devices.begin(), old_devices.end(), info) == old_devices.end()) {
                new_devices.emplace_back(info);
            }
        }
        old_devices = devices_found;

        for (auto& dev : new_devices) {
            if (dev.state != state::IN_USE_OR_UNSUPPORTED) {
                usb::device device = find_first_device(dev.vendor_id, dev.product_id, context);
                if (device) {
                    callback(device);
                }
            }
        }
    }




    LIBUSBCPP_API hotplug_handler::hotplug_handler(opt_context context, int interval)
        : handler(context, interval), interval(interval) {

    }

    LIBUSBCPP_API hotplug_handler::~hotplug_handler() {
        stop_async();
    }

    LIBUSBCPP_API void hotplug_handler::register_device_callback(const std::function<void(usb::device)>& callback) {
        handler.register_device_callback(callback);
    }

    LIBUSBCPP_API void hotplug_handler::register_device_callback(uint16_t vendor_id, uint16_t product_id,
                                                                 const std::function<void(usb::device)>& callback) {
        handler.register_device_callback(vendor_id, product_id, callback);
    }

    LIBUSBCPP_API void hotplug_handler::update() {
        handler.update();
    }

    LIBUSBCPP_API void hotplug_handler::run_async() {
        terminate = false;
        thread = std::thread([&] {
            while (!terminate) {
                handler.update();
                std::this_thread::sleep_for(std::chrono::milliseconds(interval / 10));
            }
        });
    }

    LIBUSBCPP_API void hotplug_handler::stop_async() {
        if (thread.joinable()) {
            terminate = true;
            thread.join();
        }
    }










    LIBUSBCPP_API void scan_and_process_devices(
                libusb_context* context, const std::function<bool(device_info, libusb_device_handle*)>& callback) {
        static std::mutex mutex;
        std::unique_lock<std::mutex> lock(mutex);

        libusb_device** device_list;
        ssize_t device_count = libusb_get_device_list(context, &device_list);
        if (device_count < 0) {
            LOG_ERROR("Cannot scan devices, libusb_get_device_list failed: %s", libusb_strerror(device_count));
            return;
        }

        for (size_t i = 0; i < device_count; i++) {

            struct libusb_device_descriptor descriptor{};
            int status = libusb_get_device_descriptor(device_list[i], &descriptor);
            if (status != LIBUSB_SUCCESS) {
                LOG_DEBUG("Cannot retrieve device descriptor: %s", libusb_strerror(status));
                continue;   // Jump back to top
            }

            // Open the device temporarily
            libusb_device_handle* device_handle = nullptr;
            status = libusb_open(device_list[i], &device_handle);
            if (status != LIBUSB_SUCCESS) {
                LOG_ERROR("Opening device vid=0x%04X pid=0x%04X failed: (%d) %s",
                          descriptor.idVendor, descriptor.idProduct, status, libusb_strerror(status));

                device_info info;       // Opening failed, this usually means the libusb driver is not valid
                info.vendor_id = descriptor.idVendor;           // Zadig utility can be used to fix it
                info.product_id = descriptor.idProduct;
                info.description = "";

                if (status == LIBUSB_ERROR_NOT_SUPPORTED) {
                    info.state = usb::state::IN_USE_OR_UNSUPPORTED;
                    callback(info, nullptr);
                }
                else if (status == LIBUSB_ERROR_NOT_FOUND) {
                    info.state = usb::state::INVALID_DRIVER;
                    callback(info, nullptr);
                }
                else {
                    info.state = usb::state::OTHER_LIBUSB_ERROR;
                    callback(info, nullptr);
                }

                continue;   // Jump back to top
            }

            unsigned char buffer[1024];
            status = libusb_get_string_descriptor_ascii(device_handle, descriptor.iProduct,
                                                        buffer, sizeof(buffer));
            if (status < 0) {
                LOG_ERROR("Reading string descriptor for device vid=0x%04X pid=0x%04X failed: %s",
                          descriptor.idVendor, descriptor.idProduct, libusb_strerror(status));
            }

            device_info info;
            info.vendor_id = descriptor.idVendor;
            info.product_id = descriptor.idProduct;
            info.description = status >= 0 ? std::string((char*)buffer) : "";
            info.state = usb::state::OPEN;

            if (!callback(info, device_handle)) {
                libusb_close(device_handle);
            }
        }

        libusb_free_device_list(device_list, 1);
    }

    LIBUSBCPP_API std::vector<device_info> scan_devices(opt_context context) {
        std::vector<device_info> device_list;

        libusb_context* _context = context.has_value() ? (libusb_context*)context.value().get() : nullptr;
        scan_and_process_devices(_context, [&] (const struct device_info& info, libusb_device_handle*) {
            device_list.emplace_back(info);
            return false;       // Do not keep the device open
        });

        return device_list;
    }

    LIBUSBCPP_API std::vector<usb::device> find_devices(uint16_t vendor_id, uint16_t product_id, opt_context context) {
        std::vector<usb::device> devices;

        // Following function calls the callback for every entry
        libusb_context* _context = context.has_value() ? (libusb_context*)context.value().get() : nullptr;
        scan_and_process_devices(_context,
                                 [&] (const struct device_info& info, libusb_device_handle* handle){
            if (info.vendor_id == vendor_id && info.product_id == product_id) { // Correct device id
                devices.emplace_back(std::make_shared<usb::basic_device>(handle, info));
                return true;    // Keep it open if it is valid
            }
            return false;   // Wrong device
        });

        return devices;
    }

    LIBUSBCPP_API std::vector<usb::device> find_valid_devices(uint16_t vendor_id, uint16_t product_id, opt_context context) {
        std::vector<usb::device> devices;
        auto all_devices = find_devices(vendor_id, product_id, context);
        for (const auto& device : all_devices) {
            if (device->info.state == state::OPEN) {
                devices.emplace_back(device);
            }
        }
        return devices;
    }

    LIBUSBCPP_API usb::device find_first_device(uint16_t vendor_id, uint16_t product_id, opt_context context) {
        auto devices = find_valid_devices(vendor_id, product_id, context);
        for (auto& device : devices) {
            return device;
        }
        return nullptr;
    }
}
