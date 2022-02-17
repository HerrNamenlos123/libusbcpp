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
    
}
```

