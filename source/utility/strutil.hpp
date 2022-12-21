// Copyright © 2022 Charles Kerr. All rights reserved.

#ifndef strutil_hpp
#define strutil_hpp
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <array>
#include <charconv>
#include <chrono>
#include <memory>
#include <ostream>
#include <system_error>
#include <type_traits>
#include <vector>
#if defined(_WIN32)
#if defined(max)
#undef max
#endif
#if defined(min)
#undef min
#endif
#endif
//=========================================================
namespace strutil {
    //=========================================================
    // Trim utilities
    //=========================================================
    
    // Trim all whitespace from the left of the string
    inline auto ltrim(const std::string &value) -> std::string {
        if (!value.empty()) {
            auto loc = value.find_first_not_of(" \t\v\f\n\r");
            if (loc == std::string::npos) {
                return std::string();
            }
            return value.substr(loc);
        }
        return value;
    }
    //=========================================================
    // Trim all whitespace from the right of the string
    inline auto rtrim(const std::string &value) -> std::string {
        if (!value.empty()) {
            auto loc = value.find_last_not_of(" \t\v\f\n\r");
            if (loc == std::string::npos) {
                return value;
            }
            return value.substr(0, loc + 1);
        }
        return value;
    }
    //=========================================================
    // Trim all whitespace from both sides of the string
    inline auto trim(const std::string &value) -> std::string {
        return rtrim(ltrim(value));
    }
    
    //=========================================================
    // Trim all whitespace from both sides of the string.
    // In addition, replace all runs of whitespace inside the string
    // with a single space character
    inline auto simplify(const std::string &value) -> std::string {
        // first get the leading/trailing whitespace off
        auto working = trim(value);
        if (!working.empty()) {
            auto startloc = working.find_first_of(" \t\v\f\n\r");
            while ((startloc != std::string::npos) && (startloc < working.size())) {
                auto endloc = working.find_first_not_of(" \t\v\f\n\r", startloc);
                auto amount = endloc - startloc;
                working.replace(startloc, amount, std::string(" "));
                startloc = working.find_first_of(" \t\v\f\n\r", startloc + 1);
            }
        }
        return working;
    }
    
    //=========================================================
    // Case utilities
    //=========================================================
    
    inline auto upper(const std::string &value) -> std::string {
        std::string rvalue;
        std::transform(value.begin(), value.end(), std::back_inserter(rvalue),
                       [](unsigned char c) { return std::toupper(c); } // correct
                       );
        return rvalue;
    }
    //========================================================================
    inline auto lower(const std::string &value) -> std::string {
        std::string rvalue;
        std::transform(value.begin(), value.end(), std::back_inserter(rvalue),
                       [](unsigned char c) { return std::tolower(c); } // correct
                       );
        return rvalue;
    }
    
    //=========================================================
    // String manipulation (remove remaining based on separator,
    // split, parse)
    //=========================================================
    
    //=========================================================
    inline auto strip(const std::string &value, const std::string &sep = "//", bool pack = true) -> std::string {
        auto rvalue = value;
        if (!sep.empty()){
            auto loc = rvalue.find(sep);
            if (loc != std::string::npos) {
                rvalue = rvalue.substr(0, loc);
            }
            if (pack) {
                rvalue = rtrim(rvalue);
            }
        }
        return rvalue;
    }
    
    //=========================================================
    inline auto split(const std::string &value, const std::string &sep)  -> std::pair<std::string, std::string> {
        auto first = value;
        auto second = std::string();
        if (!sep.empty()){
            auto loc = value.find(sep);
            if (loc != std::string::npos) {
                first = trim(value.substr(0, loc));
                loc = loc + sep.size();
                if (loc < value.size()) {
                    second = trim(value.substr(loc));
                }
            }
        }
        return std::make_pair(first, second);
    }
    
    //=========================================================
    inline auto parse(const std::string &value, const std::string &sep) -> std::vector<std::string> {
        std::vector<std::string> rvalue;
        auto current = std::string::size_type(0);
        if (!sep.empty()){
            auto loc = value.find(sep, current);
            while (loc != std::string::npos) {
                rvalue.push_back(trim(value.substr(current, loc - current)));
                current = loc + sep.size();
                if (current >= value.size()){
                    rvalue.push_back("");
                }
                loc = value.find(sep, current);
            }
            if (current < value.size()) {
                rvalue.push_back(trim(value.substr(current)));
            }
        }
        else {
            rvalue.push_back(trim(value));
        }
        return rvalue;
    }
    
    //=========================================================
    // Time/String conversions
    //=========================================================
    
    //=========================================================
    inline auto sysTimeToString(const std::chrono::system_clock::time_point &t) -> std::string {
        auto time = std::chrono::system_clock::to_time_t(t);
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
        auto time_str = std::string(std::ctime(&time));
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
        time_str.resize(time_str.size() - 1);
        return time_str;
    }
    
    //=========================================================
    // Format expected by the default format is: Thu Dec 30 14:13:28 2021
    inline auto stringToSysTime(const std::string &str, const std::string &format = "%a %b %d %H:%M:%S %Y")  -> std::chrono::system_clock::time_point {
        std::stringstream timbuf(str);
        tm converted;
        
        timbuf >> std::get_time(&converted, format.c_str());
        converted.tm_isdst = -1;
        auto ntime = mktime(&converted);
        return std::chrono::system_clock::from_time_t(ntime);
    }
    
    //==========================================================
    // String formatting (like printf)
    //==========================================================
    
    //==========================================================
    // The source for this was found on StackOverflow at:
    // https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    // It was slightly modified for a tad of error checking.
    template <typename... Args>
    std::string format(const std::string &format_str, Args... args) {
        auto rvalue = std::string();
        if (!format_str.empty()) {
            // First see how much space we need?
            auto size_s = std::snprintf(nullptr, 0, format_str.c_str(), args...);
            if (size_s < 0) {
                throw std::runtime_error("Error applying format string");
            }
            if (size_s > 0) {
                // Take the space we need and add 1 for the terminating \0
                size_s += 1;
                auto size = static_cast<size_t>(size_s);
                // Lets create a buffer we need for the data
                auto buf = std::make_unique<char[]>(size);
                size_s =
                std::snprintf(buf.get(), size, format_str.c_str(), args...);
                if (size_s < 0) {
                    throw std::runtime_error("Error applying format string");
                }
                if (size_s > 0) {
                    rvalue = std::string(buf.get(), buf.get() + size_s);
                }
            }
        }
        return rvalue;
    }
    
    //==========================================================
    // Number/string conversions
    //==========================================================
    
    //==========================================================
    // Radix that are supported
    enum class radix_t { dec = 10, oct = 8, hex = 16, bin = 2 };
    
    //==========================================================
    // The maximum characters in a string number for conversion sake
    inline constexpr auto max_characters_in_number = 12;
    
    //==========================================================
    // Convert a bool to a string
    // the true_value/false_value are returned based on the bool
    inline auto ntos(bool value, const std::string &true_value = "true", const std::string &false_value = "false") -> std::string {
        return (value ? true_value : false_value);
    }
    
    //==========================================================
    // Convert a number to a string, with options on radix,prefix,size,pad
    // Radix indicates the radix the string will represent
    // prefix indicates if the "radix" indicator will be present at the front of the
    // string size is the minimum size the string must be (if the number is not
    // large enough it will use the "pad" character prepended to the number
    template <typename T>
    auto ntos(T value, radix_t radix = radix_t::dec, bool prefix = false,  int size = 0, char pad = '0') -> std::string {
        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            // first, thing we need to convert the value to a string
            std::array<char, max_characters_in_number> str;
            
            if (auto [pc, ec] = std::to_chars(str.data(), str.data() + str.size(),
                                              value, static_cast<int>(radix));
                ec == std::errc()) {
                // How many characters did this number take
                auto numchars = static_cast<int>(std::distance(str.data(), pc));
                // what is larger, that is the size of our string
                auto sizeofstring = std::max(numchars, size);
                // where do we start adding the number into our string ?
                auto index = sizeofstring - numchars;
                if (prefix) {
                    // We have a prefix, so we add two characters to the beginning
                    sizeofstring += 2;
                    index += 2;
                }
                auto rvalue = std::string(sizeofstring, pad);
                // copy the value into the string
                std::copy(str.data(), pc, rvalue.begin() + index);
                // do we need our prefix?
                if (prefix) {
                    switch (static_cast<int>(radix)) {
                        case static_cast<int>(radix_t::dec):
                            // We dont add anything for decimal!
                            break;
                        case static_cast<int>(radix_t::hex):
                            rvalue[0] = '0';
                            rvalue[1] = 'x';
                            break;
                        case static_cast<int>(radix_t::oct):
                            rvalue[0] = '0';
                            rvalue[1] = 'o';
                            break;
                        case static_cast<int>(radix_t::bin):
                            rvalue[0] = '0';
                            rvalue[1] = 'b';
                            break;
                            
                        default:
                            break;
                    }
                }
                return rvalue;
                
            } else {
                // The conversion was not successful, so we return an empty string
                return std::string();
            }
        }
    }
    
    //==========================================================
    // Convert a string to a number
    template <typename T>
    typename std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, T>
    ston(const std::string &str_value, radix_t radix = radix_t::dec) {
        auto value = T{0};
        if (!str_value.empty()) {
            if (str_value.size() < 2) {
                std::from_chars(str_value.data(),
                                str_value.data() + str_value.size(), value,
                                static_cast<int>(radix));
                
            } else if (std::isalpha(
                                    static_cast<int>(static_cast<int>(str_value[1])))) {
                                        // This has a "radix indicator"
                                        switch (str_value[1]) {
                                            case 'b':
                                            case 'B':
                                                std::from_chars(str_value.data() + 2,
                                                                str_value.data() + str_value.size(), value,
                                                                static_cast<int>(radix_t::bin));
                                                break;
                                            case 'x':
                                            case 'X':
                                                std::from_chars(str_value.data() + 2,
                                                                str_value.data() + str_value.size(), value,
                                                                static_cast<int>(radix_t::hex));
                                                break;
                                            case 'o':
                                            case 'O':
                                                std::from_chars(str_value.data() + 2,
                                                                str_value.data() + str_value.size(), value,
                                                                static_cast<int>(radix_t::oct));
                                                break;
                                            default:
                                                // we dont do anything, we dont undertand so let value be 0
                                                break;
                                        }
                                        
                                    } else {
                                        auto [ptr,ec] = std::from_chars(str_value.data(),
                                                                        str_value.data() + str_value.size(), value,
                                                                        static_cast<int>(radix));
                                        if (ec == std::errc::invalid_argument) {
                                            throw std::runtime_error("Invalid argument for number conversion from string.");
                                        }
                                        else if (ec == std::errc::result_out_of_range) {
                                            throw std::runtime_error("Out of range for number conversion from string.");
                                        }
                                        return value;
                                    }
        }
        return value;
    }
    
    //==========================================================
    // Convert a string to a bool
    template <typename T>
    typename std::enable_if_t<std::is_integral_v<T> && std::is_same_v<T, bool>, T>
    ston(const std::string &str_value, const std::string &true_value = "true") {
        // If string empty, we return false
        //  we take advantege, that if in ston() we set value to 0 false, and if
        //  the from_chars fails, it doesn't touch value
        auto numvalue = ston<int>(str_value);
        if ((str_value == true_value) || (numvalue != 0)) {
            return true;
        }
        return false;
    }
    
    //===========================================================
    // Formatted dump of a byte buffer
    //===========================================================
    
    //===========================================================
    // Dumps a byte buffer, formatted to a provided stream.
    // The entries_line indicate how many bytes to display per line.
    inline auto dump(std::ostream &output, const std::uint8_t *buffer,
                     std::size_t length, radix_t radix = radix_t::hex,
                     int entries_line = 8) -> void {
        // number of characters for entry
        auto entry_size = 3; // decimal and octal
        switch (static_cast<int>(radix)) {
            case static_cast<int>(radix_t::hex):
                entry_size = 2;
                break;
            case static_cast<int>(radix_t::bin):
                entry_size = 8;
                break;
            default:
                break;
        }
        auto num_rows =
        (length / entries_line) + (((length % entries_line) == 0) ? 0 : 1);
        // what is the largest number for the address ?
        auto max_address_chars =
        static_cast<int>((ntos(num_rows * entries_line)).size());
        
        // first write out the header
        output << std::setw(max_address_chars + 2) << "" << std::setw(1);
        for (auto i = 0; i < entries_line; ++i) {
            output << ntos(i, radix_t::dec, false, entry_size, ' ') << " ";
        }
        output << "\n";
        
        // now we write out the values for each line
        std::string text(entries_line, ' ');
        
        for (std::size_t i = 0; i < length; ++i) {
            auto row = i / entries_line;
            if (((i % static_cast<size_t>(entries_line) == 0) &&
                 (i >= static_cast<size_t>(entries_line))) ||
                (i == 0)) {
                // This is a new line!
                output << ntos(row * entries_line, radix_t::dec, false,
                               max_address_chars, ' ')
                << ": ";
                text = std::string(entries_line, ' ');
            }
            output << ntos(buffer[i], radix, false, entry_size) << " ";
            // If it is an alpha, we want to write it
            if (std::isalpha(static_cast<int>(buffer[i])) != 0) {
                // we want to write this to the string
                text[(i % entries_line)] = buffer[i];
            } else {
                text[(i % entries_line)] = '.';
            }
            if (i % entries_line == entries_line - 1) {
                output << " " << text << "\n";
            }
        }
        // what if we had a partial last line, we need to figure that out
        auto last_line_entry = length % entries_line;
        if (last_line_entry != 0) {
            // we need to put the number of leading spaces
            output << std::setw(static_cast<int>((entries_line - last_line_entry) *
                                                 (entry_size + 1)))
            << "" << std::setw(1) << " " << text << "\n";
        }
    }
    
} // namespace strutil
#endif /* strutil_hpp */
