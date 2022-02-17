#pragma once

#include "inttypes.h"

namespace libusbcpp {

    class context {
    public:
        context();
        ~context();

        operator libusb_context*() const;

        context(context const&) = delete;
        void operator=(context const&) = delete;

    private:
        libusb_context* _context = nullptr;
    };

	struct deviceInfo {
		uint16_t vendorID = 0x00;
		uint16_t productID = 0x00;
		std::string description;
	};

}
