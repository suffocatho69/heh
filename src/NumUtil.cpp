#include "NumUtil.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace NumUtil {

double parseDouble(const std::string& str, double defaultValue) {
    if (str.empty()) return defaultValue;

    size_t i = 0;
    while (i < str.size() && std::isspace(static_cast<unsigned char>(str[i]))) {
        i++;
    }
    if (i >= str.size()) return defaultValue;

    bool negative = false;
    if (str[i] == '-') {
        negative = true;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    double intPart = 0.0;
    bool hasDigits = false;

    while (i < str.size() && std::isdigit(static_cast<unsigned char>(str[i]))) {
        intPart = intPart * 10.0 + (str[i] - '0');
        hasDigits = true;
        i++;
    }

    double fracPart = 0.0;
    double divisor = 10.0;

    if (i < str.size() && (str[i] == '.' || str[i] == ',')) {
        i++;
        while (i < str.size() && std::isdigit(static_cast<unsigned char>(str[i]))) {
            fracPart += (str[i] - '0') / divisor;
            divisor *= 10.0;
            hasDigits = true;
            i++;
        }
    }

    if (!hasDigits) return defaultValue;

    double result = intPart + fracPart;

    if (i < str.size() && (str[i] == 'e' || str[i] == 'E')) {
        i++;
        bool expNegative = false;
        if (i < str.size() && str[i] == '-') {
            expNegative = true;
            i++;
        } else if (i < str.size() && str[i] == '+') {
            i++;
        }

        int expValue = 0;
        while (i < str.size() && std::isdigit(static_cast<unsigned char>(str[i]))) {
            expValue = expValue * 10 + (str[i] - '0');
            i++;
        }

        double factor = std::pow(10.0, expValue);
        if (expNegative) {
            result /= factor;
        } else {
            result *= factor;
        }
    }

    return negative ? -result : result;
}

int parseInt(const std::string& str, int defaultValue) {
    double d = parseDouble(str, static_cast<double>(defaultValue));
    return static_cast<int>(d);
}

std::string formatDouble(double value, int precision) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    std::string res = ss.str();
    for (char& c : res) {
        if (c == ',') c = '.';
    }
    return res;
}

} // namespace NumUtil
