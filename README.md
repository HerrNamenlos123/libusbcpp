# libusbcpp

This library is an object-oriented C++ wrapper around the official libusb-1.0 library.

## How to use

This library can be included by a premake-script. Create a repository with a premake project.

### Git submodule

Add the library as a git submodule to your repository:  
```
git submodule add https://github.com/HerrNamenlos123/libusbcpp.git modules/libusbcpp
```
Later you can update your submodule to the latest version with  
```
git submodule update --remote modules/libusbcpp
```
Don't forget to always do a `--recursive`-clone of your repository!

### Premake script

Then include it from `premake5.lua`:

```lua
-- Include the library
include "modules/libusbcpp" -- Path to your submodule
```

This adds the project itself to the solution. After that use the following to link it to your project:

```lua
-- libusbcpp dependency
dependson "libusbcpp"
includedirs (LIBUSBCPP_INCLUDE_DIRS)
libdirs (LIBUSBCPP_LINK_DIRS)
links (LIBUSBCPP_LINKS)
```

## Example

```C++
#include "libusbcpp.h"

int main() {
    libusbcpp::context context;
	{
		std::vector<libusbcpp::device> _devices;
		while (true) {
			libusbcpp::setLogLevel(libusbcpp::LOG_LEVEL_DEBUG);

			auto& devices = libusbcpp::findDevice(context, 0x1209, 0x0D32);
			for (auto& device : devices) {
				if (device) {
					std::cout << "Device opened" << std::endl;
					if (device->claimInterface(2)) {
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

You can define the macro `LIBUSBCPP_NO_LOGGING` to disable logging completely.

