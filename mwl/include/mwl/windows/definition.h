#pragma once

#include <Windows.h>

#include <string>
#include <vector>

namespace mwl::windows
{

using Byte = BYTE;
using Dword = DWORD;
using Qword = DWORD64;
using String = std::wstring;
using MultiString = std::vector<std::wstring>;
using Binary = std::vector<Byte>;

using StringView = std::wstring_view;

} // namespace mwl::windows
