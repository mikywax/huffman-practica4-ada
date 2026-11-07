#pragma once
#include <string>

namespace huf {
bool compress(const std::string& in_path, const std::string& out_path);
bool decompress(const std::string& in_path, const std::string& out_path);
}