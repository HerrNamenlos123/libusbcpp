
#include <iostream>
#include "libusbcpp.h"

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

    for (const auto& device_info : devices) {
        if (device_info.state == usb::state::OPEN) {
            printf(" -- 0x%04X/0x%04X -> %s\n",
                   device_info.vendor_id, device_info.product_id, device_info.description.c_str());
        }
        else {
            printf(" -- 0x%04X/0x%04X (%s)\n",
                   device_info.vendor_id, device_info.product_id, usb::state_str(device_info.state));
        }
    }

    return 0;
}