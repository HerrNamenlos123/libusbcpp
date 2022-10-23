
#include <iostream>
#include "libusbcpp.h"

// Create a libusbcpp context. This has to outlive any device objects.
// Make sure that all devices are destroyed when the main function returns.
// (Basically just means no usb::device's at the global scope)
usb::context context;

int main() {

    hotplug

    auto device = usb::find_device(context, 0x03F0, 0x2C24);

    device->claim_interface(2);
    std::cout << device->device_info.description << std::endl;

    return 0;
}