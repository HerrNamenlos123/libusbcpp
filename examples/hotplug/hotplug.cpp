
#include <iostream>
#include <utility>
#include "libusbcpp.h"

//#include "libusb.h"   // You can still include the raw C-style libusb api if you need more features
// Do not include it if you don't use it (see way below on how to get the device handle)

// Create a libusbcpp context. This has to outlive any device objects.
// Make sure that all devices are destroyed when the main function returns.
// (Basically just means no usb::device's at the global scope)
usb::context context;

void synchronous_hotplug_handler_all() {

    usb::generic_hotplug_handler hotplug(context, 1000);    // rescan interval in ms

    std::vector<usb::device> devices;

    // In this example the device is simply added to an array of devices. This is synchronous, the callback is called
    // from the thread calling hotplug.update().
    // Only one callback can be registered at a time, registering overwrites any previous callback

    // You can also register a simple c-style function pointer:
    //  hotplug.register_device_callback(callback_function);

    // Here we use a C++ Lambda function for convenience
    hotplug.register_device_callback([&] (usb::device device) {

        // usb::device is actually a shared_ptr. In other cases you would always have to check it,
        // but in the hotplug callback it is guaranteed to never give you a nullptr

        // You can safely access its content and check if it's valid
        if (device->is_open()) {

            printf("New device connected: 0x%04X/0x%04X (state %s) -> %s\n",
                   device->info.vendor_id,
                   device->info.product_id,
                   usb::state_str(device->info.state),
                   device->info.description.c_str());

            // If you don't assign the device anywhere and not keep it alive (in other words you simply ignore it),
            // it is going to be closed and destroyed automatically
            devices.emplace_back(std::move(device));
        }
        else {
            // This means your device was found, but could not be accessed.

            // When the state is INVALID_DRIVER, chances are it does not support libusb right now.
            // In this case visit https://zadig.akeo.ie and download the zadig utility,
            // which can be used to switch to a driver that is supported by libusb.
            printf("Device invalid: 0x%04X/0x%04X -> state %s\n",
                   device->info.vendor_id,
                   device->info.product_id,
                   usb::state_str(device->info.state));
        }
    });

    // In the meantime we can use our device from another thread
    // (make sure to properly mutex your vector thread-safely)

    while (true) {
        hotplug.update();   // Simply call this in your main loop, it will only scan in the interval you specify above
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void synchronous_hotplug_handler_specific() {
    usb::generic_hotplug_handler hotplug(context, 1000);    // rescan interval in ms

    // This example is identical to the one above, except for the fact that it filters usb ids for you

    uint16_t vendor_id = 0x1209;
    uint16_t product_id = 0x0D32;
    std::vector<usb::device> devices;

    hotplug.register_device_callback(vendor_id, product_id, [&] (usb::device device) {

        // In the example above a device is only opened when freshly connected to the system. (might potentially be missed)
        // When specifying vendor id and product id, any fitting device which is available is opened,
        // without checking if it was already connected
        //
        // This basically means one thing: When the device in the callback is ignored,
        // the callback is called over and over again. This means all instances of your device must be used (not only one)

        if (device->is_open()) {
            printf("New device connected: 0x%04X/0x%04X (state %s) -> %s\n",
                   device->info.vendor_id,
                   device->info.product_id,
                   usb::state_str(device->info.state),
                   device->info.description.c_str());
            devices.emplace_back(std::move(device));
        }
    });

    while (true) {
        hotplug.update();   // Simply call this in your main loop, it will only scan in the interval you specify above
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void async_hotplug_handler_specific() {
    usb::hotplug_handler hotplug(context, 1000);

    // This example is again to the one above, but it is asynchronous

    uint16_t vendor_id = 0x1209;
    uint16_t product_id = 0x0D32;

    std::mutex mutex;
    std::vector<usb::device> devices;

    hotplug.register_device_callback(vendor_id, product_id, [&] (usb::device device) {

        // In this example we start the loop in a background thread. The only difference is that you do not need
        // to care about calling hotplug.update(), it is done automatically. However, be careful as the callback is now
        // called from another thread. Think about this when mutex-locking your device objects.

        if (device->is_open()) {
            printf("New device connected: 0x%04X/0x%04X (state %s) -> %s\n",
                   device->info.vendor_id,
                   device->info.product_id,
                   usb::state_str(device->info.state),
                   device->info.description.c_str());

            std::unique_lock<std::mutex> lock(mutex);   // Important: We are using multiple threads now!!!
            devices.emplace_back(std::move(device));
        }
    });

    hotplug.run_async();    // Here we start the background thread. It runs until you call .stop_async(),
                            // or until the hotplug object runs out of scope and is destroyed

    while (true) {  // Do nothing
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        //std::unique_lock<std::mutex> lock(mutex);
        //devices.something...
    }

    //hotplug.stop_async();     // Not necessary
}

int main() {

    //synchronous_hotplug_handler_all();
    //synchronous_hotplug_handler_specific();
    async_hotplug_handler_specific();

    return 0;
}