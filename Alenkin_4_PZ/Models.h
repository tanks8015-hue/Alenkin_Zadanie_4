#pragma once
#include <string>
struct Part {
    int PartID;
    std::wstring PartName;
    int CategoryID;
    int SupplierID;
    double Price;
};
struct User {
    int UserID;
    std::wstring Username;
    int RoleID;

};