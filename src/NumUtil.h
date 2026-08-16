#ifndef NUMUTIL_H
#define NUMUTIL_H

#include <string>

namespace NumUtil {
    // Parse double from string regardless of system locale (accepts '.' and ',')
    double parseDouble(const std::string& str, double defaultValue = 0.0);

    // Parse integer from string regardless of system locale
    int parseInt(const std::string& str, int defaultValue = 0);

    // Format double to string with fixed precision using '.' as decimal point
    std::string formatDouble(double value, int precision = 2);
}

#endif // NUMUTIL_H
