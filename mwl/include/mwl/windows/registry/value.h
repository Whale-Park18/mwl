#pragma once

#include <Windows.h>

#include <cassert>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "mwl/windows/definition.h"

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

class Value
{
public:

    using Type = std::variant<std::monostate, // ValueType::None에 해당하는 빈 상태
                              Dword,          // ValueType::Dword에 해당하는 32비트 숫자
                              Qword,          // ValueType::Qword에 해당하는 64비트 숫자
                              String,         // ValueType::String,  ValueType::ExpandString에
                                              // 해당하는 문자열 (REG_SZ, REG_EXPAND_SZ 모두 문자열로 처리)
                              MultiString,    // ValueType::MultiString에 해당하는 문자열
                                              // 배열 (REG_MULTI_SZ)
                              Binary          // ValueType::Binary에 해당하는 이진 데이터 (REG_BINARY)
                              >;

    Value() = default;

    /// <summary>
    /// Dword 값을 저장하는 Value 객체를 생성합니다.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="data"></param>
    Value(std::wstring_view name, Dword data);

    /// <summary>
    /// Qword 값을 저장하는 Value 객체를 생성합니다.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="data"></param>
    Value(std::wstring_view name, Qword data);

    /// <summary>
    /// String 값을 저장하는 Value 객체를 생성합니다.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="data"></param>
    /// <param name="isExpand">값이 REG_EXPAND_SZ인지 여부를 나타냅니다. true이면 REG_EXPAND_SZ로 처리되고, false이면
    /// REG_SZ로 처리됩니다.</param>
    Value(std::wstring_view name, String data, bool isExpand = false);

    /// <summary>
    /// MultiString 값을 저장하는 Value 객체를 생성합니다.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="data"></param>
    Value(std::wstring_view name, MultiString data);

    /// <summary>
    /// Binary 값을 저장하는 Value 객체를 생성합니다.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="data"></param>
    Value(std::wstring_view name, Binary data);

    /// <summary>
    /// RegEnumValueW/RegGetValueW가 반환한 raw 바이트 버퍼로부터 Value 객체를 생성합니다 (Read/Enum용).
    /// type에 따라 data의 바이트를 적절한 Data variant 대체자로 파싱합니다.
    /// </summary>
    /// <param name="name">값 이름</param>
    /// <param name="type">값 타입. data를 어떤 Data 대체자로 해석할지 결정합니다.</param>
    /// <param name="data">WinAPI가 채운 raw 바이트 버퍼에 대한 읽기 전용 view. 호출 후 실제로 기록된 바이트
    /// 수로 길이를 맞춰야 합니다 (버퍼의 할당 크기가 아님).</param>
    Value(std::wstring_view name, ValueType type, std::span<const Byte> data);

    /// <summary>
    /// 값 이름을 반환합니다.
    /// </summary>
    /// <returns>값 이름</returns>
    const std::wstring& name() const noexcept
    {
        return name_;
    }

    /// <summary>
    /// 값 타입을 반환합니다.
    /// </summary>
    /// <returns>값 타입</returns>
    ValueType type() const noexcept
    {
        return type_;
    }

    /// <summary>
    /// 저장된 데이터를 반환합니다.
    /// </summary>
    /// <typeparam name="T">반환할 데이터 타입</typeparam>
    /// <returns>저장된 데이터</returns>
    /// <remarks>
    /// - 반환을 어떻게 해야 하는가?
    ///     - mwl::Result로 반환해야 하는가?
    ///     - std::optional로 반환해야 하는가?
    /// - 타입을 잘못 지정했을 때, 어떻게 해야 하는가?
    /// </remarks>
    template<class T>
    const T& data() const
    {
        assert(std::holds_alternative<T>(data_));
        return std::get<T>(data_);
    }

private:

    std::wstring name_{};
    ValueType type_{ ValueType::None };
    Type data_{};
};

} // namespace mwl::windows::registry
