#include "Validator.h"
#include <regex>
bool Validator::IsValidEmail(const std::string& email) {
    const std::regex pattern(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    return std::regex_match(email, pattern);
}
bool Validator::IsValidPrice(const std::string& price) {
    const std::regex pattern(R"(^\d+(\.\d{1,2})?$)");
    return std::regex_match(price, pattern);
}

bool Validator::IsValidDate(const std::string& date) {
    const std::regex pattern(R"(^\d{4}-\d{2}-\d{2}$)");
    return std::regex_match(date, pattern);

}