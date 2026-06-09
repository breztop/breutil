#pragma once

#include <iostream>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <cstring>

namespace bre {

bool isMacBookPro() {
    bool macBookPro = false;

    char buf[128]{};
    size_t length = sizeof(buf);

    int intErr = sysctlbyname("hw.model", buf, &length, nullptr, 0);
    if (intErr != 0) {
        std::cerr << "Error in sysctlbyname(): " << strerror(errno) << "\n";
    } else {
        buf[length < sizeof(buf) ? length : sizeof(buf) - 1] = '\0'; // Ensure null-termination
        std::cout << "Hardware model: " << buf << "\n";
        if (strncmp(buf, "MacBookPro", 10) == 0) {
            macBookPro = true;
        }
    }
    return macBookPro;
}

} // namespace bre