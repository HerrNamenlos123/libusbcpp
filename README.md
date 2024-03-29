# usb

This library is an object-oriented C++ wrapper around the official libusb-1.0 library.

## How to use

This library can be included by a premake-script. Create a repository with a premake project.

### Git submodule

Add the library as a git submodule to your repository:  
```
git submodule add https://github.com/HerrNamenlos123/usb.git modules/usb
git submodule update --init --recursive
```
Later you can update your submodule to the latest version with  
```
git submodule update --remote modules/usb
```
Don't forget to always do a `--recursive`-clone of your repository!

### Premake script

Then include it from `premake5.lua`:

```lua
-- Include the library
include "modules/usb" -- Path to your submodule
```

This adds the project itself to the solution. After that use the following to link it to your project:

```lua
-- usb dependency
dependson "usb"
includedirs (LIBUSBCPP_INCLUDE_DIRS)
libdirs (LIBUSBCPP_LINK_DIRS)
links (LIBUSBCPP_LINKS)
```

## Example

```C++
#include "usb.h"

int main() {
    usb::context context;
	{
		std::vector<usb::device> _devices;
		while (true) {
			usb::setLogLevel(usb::LOG_LEVEL_DEBUG);

			auto& devices = usb::find_device(context, 0x1209, 0x0D32);
			for (auto& device : devices) {
				if (device) {
					std::cout << "Device opened" << std::endl;
					if (device->claim_interface(2)) {
						_devices.push_back(device);
					}
				}
				else {
					std::cout << "No device found" << std::endl;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}
}
```

CMake `LIBUSBCPP_VERBOSE_LOGGING` can be used to enable `LOG`, `LOG_DEBUG` and `LOG_INFO`.  
`LOG_WARN` and `LOG_ERROR` are always enabled.

