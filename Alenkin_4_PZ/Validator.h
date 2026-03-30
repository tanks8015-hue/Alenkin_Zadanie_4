#pragma once
#include <string>

class Validator {
public:
    static bool IsValidEmail(const std::string& email);
    static bool IsValidPrice(const std::string& price);
    static bool IsValidDate(const std::string& date);

};