
#include "libusbcpp.h"
#include "libusb.h"

#include <exception>
#include <string>

namespace libusbcpp {

    context::context() {
        if (libusb_init(&_context) < 0) {
            throw std::runtime_error("[libusb]: libusb could not be initialized!");
        }
    }

    context::~context() {
        libusb_exit(_context);
    }

    context::operator libusb_context*() const {
        return _context;
    }

	struct deviceInfo {
		uint16_t vendorID = 0x00;
		uint16_t productID = 0x00;
		std::string description;
	};

}
