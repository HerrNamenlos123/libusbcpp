
#include <iostream>
#include "libusbcpp.h"
//#include "libusb.h"   // You can still include the raw C-style libusb api if you need more features
//                         (see way below on how to get the device handle)

// Create a libusbcpp context. This has to outlive any device objects.
// Make sure that all devices are destroyed when the main function returns.
// (Basically just means no usb::device's at the global scope)
usb::context context;

int main() {

    //usb::enable_logging(true/false);

    std::cout << "Searching for devices..." << std::endl;

    // Get a list of available devices
    std::vector<usb::device_info> devices = usb::scan_devices(context);
    // or: auto devices = usb::scan_devices(context);

    // Print the list
    std::cout << devices.size() << " devices found" << std::endl;

    for (const auto& info : devices) {
        if (info.state == usb::state::OPEN) {
            printf(" -- 0x%04X/0x%04X -> %s\n",
                   info.vendor_id, info.product_id, info.description.c_str());
        }
        else {
            printf(" -- 0x%04X/0x%04X (%s)\n",
                   info.vendor_id, info.product_id, usb::state_str(info.state));
        }
    }

    return 0;
}