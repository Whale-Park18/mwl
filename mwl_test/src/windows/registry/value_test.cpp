#include <gtest/gtest.h>
#include <mwl/windows/definition.h>
#include <mwl/windows/registry/value.h>

#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace
{

namespace reg = mwl::windows::registry;
using mwl::windows::Binary;
using mwl::windows::Byte;
using mwl::windows::Dword;
using mwl::windows::MultiString;
using mwl::windows::Qword;
using mwl::windows::String;

// wstring_view을 null 종료 문자를 포함한(옵션) raw 바이트 시퀀스로 변환한다.
std::vector<Byte> ToBytes(std::wstring_view str, bool includeNull = true)
{
    std::vector<Byte> bytes(reinterpret_cast<const Byte*>(str.data()),
                            reinterpret_cast<const Byte*>(str.data() + str.size()));
    if (includeNull)
    {
        constexpr wchar_t kNull{ L'\0' };
        const Byte* nullBytes = reinterpret_cast<const Byte*>(&kNull);
        bytes.insert(bytes.end(), nullBytes, nullBytes + sizeof(wchar_t));
    }
    return bytes;
}

template<class T>
std::vector<Byte> ToBytes(const T& value)
{
    std::vector<Byte> bytes(sizeof(T));
    std::memcpy(bytes.data(), &value, sizeof(T));
    return bytes;
}

} // namespace

//***************************************************************************
// 값 생성자
//***************************************************************************

// 기본 생성자는 이름이 비어 있고 타입이 None인 Value를 만드는지 확인한다.
TEST(ValueTest, DefaultConstructor_HasNoneTypeAndEmptyName)
{
    reg::Value value{};

    EXPECT_TRUE(value.name().empty());
    EXPECT_EQ(value.type(), reg::ValueType::None);
}

// Dword 생성자가 이름/타입/데이터를 올바르게 설정하는지 확인한다.
TEST(ValueTest, DwordConstructor_SetsNameTypeAndData)
{
    constexpr std::wstring_view name{ L"TestDword" };
    constexpr Dword expected{ 0x1234 };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::Dword);
    EXPECT_EQ(value.data<Dword>().has_value(), true);
    EXPECT_EQ(value.data<Dword>(), expected);
}

// Qword 생성자가 이름/타입/데이터를 올바르게 설정하는지 확인한다.
TEST(ValueTest, QwordConstructor_SetsNameTypeAndData)
{
    constexpr std::wstring_view name{ L"TestQword" };
    constexpr Qword expected{ 0x1122334455667788ull };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::Qword);
    EXPECT_EQ(value.data<Qword>().has_value(), true);
    EXPECT_EQ(value.data<Qword>(), expected);
}

// String 생성자가 기본값으로 REG_SZ(String) 타입을 설정하는지 확인한다.
TEST(ValueTest, StringConstructor_DefaultsToRegSz)
{
    constexpr std::wstring_view name{ L"TestString" };
    const String expected{ L"Hello, mwl!" };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::String);
    EXPECT_EQ(value.data<String>().has_value(), true);
    EXPECT_EQ(value.data<String>(), expected);
}

// isExpand가 true이면 ExpandString 타입으로 설정되는지 확인한다.
TEST(ValueTest, StringConstructor_IsExpandTrue_SetsExpandString)
{
    constexpr std::wstring_view name{ L"TestExpandString" };
    const String expected{ L"%TEMP%\\mwl" };

    reg::Value value{ name, expected, true };

    EXPECT_EQ(value.type(), reg::ValueType::ExpandString);
    EXPECT_EQ(value.data<String>().has_value(), true);
    EXPECT_EQ(value.data<String>(), expected);
}

// MultiString 생성자가 이름/타입/데이터를 올바르게 설정하는지 확인한다.
TEST(ValueTest, MultiStringConstructor_SetsNameTypeAndData)
{
    constexpr std::wstring_view name{ L"TestMultiString" };
    const MultiString expected{ L"A", L"B", L"C" };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::MultiString);
    EXPECT_EQ(value.data<MultiString>().has_value(), true);
    EXPECT_EQ(value.data<MultiString>(), expected);
}

// Binary 생성자가 이름/타입/데이터를 올바르게 설정하는지 확인한다.
TEST(ValueTest, BinaryConstructor_SetsNameTypeAndData)
{
    constexpr std::wstring_view name{ L"TestBinary" };
    const Binary expected{ 0x01, 0x02, 0x03, 0xFF };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::Binary);
    EXPECT_EQ(value.data<Binary>().has_value(), true);
    EXPECT_EQ(value.data<Binary>(), expected);
}

//***************************************************************************
// raw 바이트 버퍼로부터의 생성 (Read/Enum용)
//***************************************************************************

// raw 바이트 버퍼로부터 Dword 값을 올바르게 파싱하는지 확인한다.
TEST(ValueTest, RawBufferConstructor_Dword_ParsesValue)
{
    constexpr std::wstring_view name{ L"RawDword" };
    constexpr Dword expected{ 0xABCD };
    const std::vector<Byte> bytes{ ToBytes(expected) };

    reg::Value value{ name, reg::ValueType::Dword, bytes };

    EXPECT_EQ(value.type(), reg::ValueType::Dword);
    EXPECT_EQ(value.data<Dword>().has_value(), true);
    EXPECT_EQ(value.data<Dword>(), expected);
}

// raw 바이트 버퍼로부터 Qword 값을 올바르게 파싱하는지 확인한다.
TEST(ValueTest, RawBufferConstructor_Qword_ParsesValue)
{
    constexpr std::wstring_view name{ L"RawQword" };
    constexpr Qword expected{ 0x1122334455667788ull };
    const std::vector<Byte> bytes{ ToBytes(expected) };

    reg::Value value{ name, reg::ValueType::Qword, bytes };

    EXPECT_EQ(value.type(), reg::ValueType::Qword);
    EXPECT_EQ(value.data<Qword>().has_value(), true);
    EXPECT_EQ(value.data<Qword>(), expected);
}

// null 종료 문자가 포함된 버퍼를 문자열로 파싱할 때 끝의 null이 제거되는지 확인한다.
TEST(ValueTest, RawBufferConstructor_String_StripsTrailingNull)
{
    constexpr std::wstring_view name{ L"RawString" };
    constexpr std::wstring_view expected{ L"hello raw" };
    const std::vector<Byte> bytes{ ToBytes(expected, /*includeNull=*/true) };

    reg::Value value{ name, reg::ValueType::String, bytes };

    EXPECT_EQ(value.type(), reg::ValueType::String);
    EXPECT_EQ(value.data<String>().has_value(), true);
    EXPECT_EQ(value.data<String>(), expected);
}

// null 종료 문자가 없는 버퍼도 모든 문자를 그대로 보존하는지 확인한다.
TEST(ValueTest, RawBufferConstructor_String_WithoutTrailingNull_KeepsAllChars)
{
    constexpr std::wstring_view name{ L"RawStringNoNull" };
    constexpr std::wstring_view expected{ L"no null" };
    const std::vector<Byte> bytes{ ToBytes(expected, /*includeNull=*/false) };

    reg::Value value{ name, reg::ValueType::String, bytes };

    EXPECT_EQ(value.data<String>().has_value(), true);
    EXPECT_EQ(value.data<String>(), expected);
}

// ExpandString 타입 버퍼도 String과 동일하게 파싱되는지 확인한다.
TEST(ValueTest, RawBufferConstructor_ExpandString_ParsesAsString)
{
    constexpr std::wstring_view name{ L"RawExpandString" };
    constexpr std::wstring_view expected{ L"%TEMP%\\mwl" };
    const std::vector<Byte> bytes{ ToBytes(expected) };

    reg::Value value{ name, reg::ValueType::ExpandString, bytes };

    EXPECT_EQ(value.type(), reg::ValueType::ExpandString);
    EXPECT_EQ(value.data<String>(), expected);
}

// null로 구분된 버퍼가 여러 문자열로 올바르게 분리되는지 확인한다.
TEST(ValueTest, RawBufferConstructor_MultiString_ParsesAllStrings)
{
    constexpr std::wstring_view name{ L"RawMultiString" };
    const std::vector<wchar_t> chars{ L'A', L'B', L'C', L'\0', L'D', L'E', L'F', L'\0', L'\0' };
    std::vector<Byte> bytes(reinterpret_cast<const Byte*>(chars.data()),
                            reinterpret_cast<const Byte*>(chars.data() + chars.size()));

    reg::Value value{ name, reg::ValueType::MultiString, bytes };

    const MultiString expected{ L"ABC", L"DEF" };
    EXPECT_EQ(value.type(), reg::ValueType::MultiString);
    EXPECT_EQ(value.data<MultiString>().has_value(), true);
    EXPECT_EQ(value.data<MultiString>(), expected);
}

// 빈 버퍼를 MultiString으로 파싱하면 빈 목록이 되는지 확인한다.
TEST(ValueTest, RawBufferConstructor_MultiString_EmptyBuffer_ReturnsEmpty)
{
    constexpr std::wstring_view name{ L"RawMultiStringEmpty" };
    const std::vector<Byte> bytes{};

    reg::Value value{ name, reg::ValueType::MultiString, bytes };

    EXPECT_EQ(value.data<MultiString>().has_value(), true);
    EXPECT_TRUE(value.data<MultiString>().value().empty());
}

// Binary 타입 버퍼가 바이트 그대로 복사되는지 확인한다.
TEST(ValueTest, RawBufferConstructor_Binary_CopiesBytes)
{
    constexpr std::wstring_view name{ L"RawBinary" };
    const std::vector<Byte> expected{ 0x01, 0x02, 0x03, 0xFF };

    reg::Value value{ name, reg::ValueType::Binary, expected };

    EXPECT_EQ(value.type(), reg::ValueType::Binary);
    EXPECT_EQ(value.data<Binary>().has_value(), true);
    EXPECT_EQ(value.data<Binary>(), expected);
}

// 알 수 없는 타입(None)이면 Binary로 폴백해 파싱되는지 확인한다.
TEST(ValueTest, RawBufferConstructor_UnknownType_FallsBackToBinary)
{
    constexpr std::wstring_view name{ L"RawUnknown" };
    const std::vector<Byte> expected{ 0xDE, 0xAD, 0xBE, 0xEF };

    reg::Value value{ name, reg::ValueType::None, expected };

    EXPECT_EQ(value.type(), reg::ValueType::None);
    EXPECT_EQ(value.data<Binary>().has_value(), true);
    EXPECT_EQ(value.data<Binary>(), expected);
}

//***************************************************************************
// data<T>() 타입 불일치
//***************************************************************************

// 저장된 타입과 다른 타입으로 data<T>()를 호출하면 nullopt를 반환하는지 확인한다.
TEST(ValueTest, Data_TypeMismatch_ReturnsNullopt)
{
    reg::Value value{ L"TestDword", Dword{ 0x1234 } };

    EXPECT_EQ(value.data<Qword>().has_value(), false);
    EXPECT_EQ(value.data<String>().has_value(), false);
    EXPECT_EQ(value.data<MultiString>().has_value(), false);
    EXPECT_EQ(value.data<Binary>().has_value(), false);
}

// 기본 생성된 Value는 어떤 타입으로 조회해도 data<T>()가 nullopt를 반환하는지 확인한다.
TEST(ValueTest, DefaultConstructor_Data_AnyType_ReturnsNullopt)
{
    reg::Value value{};

    EXPECT_EQ(value.data<Dword>().has_value(), false);
    EXPECT_EQ(value.data<Qword>().has_value(), false);
    EXPECT_EQ(value.data<String>().has_value(), false);
    EXPECT_EQ(value.data<MultiString>().has_value(), false);
    EXPECT_EQ(value.data<Binary>().has_value(), false);
}
