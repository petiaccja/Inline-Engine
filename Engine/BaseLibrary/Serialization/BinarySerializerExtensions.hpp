#pragma once

#include <string>

class BinarySerializer;


// strings
BinarySerializer& operator<<(BinarySerializer& s, const std::string& str) = delete;
BinarySerializer& operator>>(BinarySerializer& s, std::string& str) = delete;