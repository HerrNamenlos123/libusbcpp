
#include <iostream>
#include "libusbcpp.h"

// Create a libusbcpp context. This has to outlive any device objects.
// Make sure that all devices are destroyed when the main function returns.
// (Basically just means no usb::device's at the global scope)
usb::context context;

int main() {

    //usb::enable_logging(true/false);

    {
        std::cout << "Searching for devices..." << std::endl;

        auto devices = usb::scan_devices(context);
        printf("%zu devices found\n", devices.size());
        for (const auto &device_info: devices) {
            if (device_info.state == usb::state::OPEN) {
                printf(" -- 0x%04X/0x%04X -> %s\n",
                       device_info.vendor_id, device_info.product_id, device_info.description.c_str());
            } else {
                printf(" -- 0x%04X/0x%04X (%s)\n",
                       device_info.vendor_id, device_info.product_id, usb::state_str(device_info.state));
            }
        }
    }


    // When the state is INVALID_DRIVER, chances are it does not support libusb right now.
    // In this case visit https://zadig.akeo.ie and download the zadig utility, which can be used to switch to a driver
    // that is supported by libusb.



    // Connecting to a device with VendorID 0x1209 and ProductID 0x0D32. This is an ODrive V3.6 servo drive board.
    // You will have to choose something that fits your device for testing.
    int vid = 0x1209;
    int pid = 0x0D32;
    printf("\nLooking for devices 0x%04X/0x%04X\n", vid, pid);

    // This is the most generic function, which opens all devices with the ids you want.
    // Then you can later close all the ones you don't want
    // It returns all devices (including invalid), that way you can inspect if the device is disconnected, or already in use.
    {
        auto devices = usb::find_devices(context, vid, pid);
        printf("%zu devices found\n", devices.size());
        for (const auto &device: devices) {
            printf("Device state: %s\n", usb::state_str(device->device_info.state));
        }

        // Here all devices are automatically closed, as the device array is destroyed and no device has been assigned
        // You could say myDevice = devices[i]; to keep it alive and actively use it
    }





    printf("\nLooking for usable devices 0x%04X/0x%04X\n", vid, pid);

    // This is a convenience function, it returns only the devices you can actually use. However, you cannot
    // differentiate if the device is disconnected, unsupported, etc.
    {
        auto devices = usb::find_valid_devices(context, vid, pid);
        printf("%zu devices found\n", devices.size());
        for (const auto &device: devices) {
            printf("Device state: %s\n", usb::state_str(device->device_info.state));
        }

        // Here all devices are automatically closed, as the device array is destroyed and no device has been assigned
        // You could say myDevice = devices[i]; to keep it alive and actively use it
    }





    printf("\nLooking for first usable device 0x%04X/0x%04X\n", vid, pid);

    // This is another convenience function, it returns the first device with your ids which can be accessed. This can
    // be used if you have multiple devices with the same id, it's going to open one with every call until no usable
    // device is found anymore.
    {
        usb::device device = usb::find_first_device(context, vid, pid);
        if (device) {
            printf("Device state: %s\n", usb::state_str(device->device_info.state));
        }
        else {
            printf("No device found :(\n");
        }

        // Now use your device
    }

    return 0;
}