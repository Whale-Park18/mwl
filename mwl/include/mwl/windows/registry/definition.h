#pragma once

#include <Windows.h>

namespace mwl::windows::registry
{

enum class ValueType : DWORD
{
    None = REG_NONE,              // No value type
    String = REG_SZ,              // null-terminated 문자열입니다.
    MultiString = REG_MULTI_SZ,   // null-terminated 문자열 배열입니다. 각 문자열은 null 문자로 구분되고, 전체 배열은 두
                                  // 개의 null 문자로 끝납니다.
    ExpandString = REG_EXPAND_SZ, // null-terminated 문자열로, 환경 변수 참조를 포함할 수 있습니다. 시스템이 값을 읽을
                                  // 때 참조가 확장됩니다.
    Binary = REG_BINARY,          // 이진 데이터입니다.
    Dword = REG_DWORD,            // 32비트 숫자입니다.
    Qword = REG_QWORD,            // 64비트 숫자입니다.
};

} // namespace mwl::windows::registry
