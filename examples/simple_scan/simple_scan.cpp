
#include <iostream>
#include "libusbcpp.h"

int main() {

    // Create a libusbcpp context. This has to outlive any device objects
    usb::context context;

    // Get a list of available devices
    std::cout << "Searching for devices..." << std::endl;
    std::vector<usb::device_info> devices = usb::scan_devices(context);

    // Print the list
    std::cout << devices.size() << " devices found" << std::endl;
    for (auto& device : devices) {
        std::cout << std::hex << " -- 0x" << device.vendor_id << "|" <<
                "0x" << device.product_id << " -> " << device.description << std::endl;
    }

#ifdef _WIN32
    std::cout << "Press [Enter] to exit" << std::endl;
    std::cin.get();
#endif

    return 0;
}