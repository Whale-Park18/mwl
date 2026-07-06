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

// --- 값 생성자 ---

TEST(ValueTest, DefaultConstructor_HasNoneTypeAndEmptyName)
{
    reg::Value value{};

    EXPECT_TRUE(value.name().empty());
    EXPECT_EQ(value.type(), reg::ValueType::None);
}

TEST(ValueTest, DwordConstructor_SetsNameTypeAndData)
{
    constexpr std::wstring_view name{ L"TestDword" };
    constexpr Dword expected{ 0x1234 };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::Dword);
    EXPECT_EQ(value.data<Dword>(), expected);
}

TEST(ValueTest, QwordConstructor_SetsNameTypeAndData)
{
    constexpr std::wstring_view name{ L"TestQword" };
    constexpr Qword expected{ 0x1122334455667788ull };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::Qword);
    EXPECT_EQ(value.data<Qword>(), expected);
}

TEST(ValueTest, StringConstructor_DefaultsToRegSz)
{
    constexpr std::wstring_view name{ L"TestString" };
    const String expected{ L"Hello, mwl!" };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::String);
    EXPECT_EQ(value.data<String>(), expected);
}

TEST(ValueTest, StringConstructor_IsExpandTrue_SetsExpandString)
{
    constexpr std::wstring_view name{ L"TestExpandString" };
    const String expected{ L"%TEMP%\\mwl" };

    reg::Value value{ name, expected, true };

    EXPECT_EQ(value.type(), reg::ValueType::ExpandString);
    EXPECT_EQ(value.data<String>(), expected);
}

TEST(ValueTest, MultiStringConstructor_SetsNameTypeAndData)
{
    constexpr std::wstring_view name{ L"TestMultiString" };
    const MultiString expected{ L"A", L"B", L"C" };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::MultiString);
    EXPECT_EQ(value.data<MultiString>(), expected);
}

TEST(ValueTest, BinaryConstructor_SetsNameTypeAndData)
{
    constexpr std::wstring_view name{ L"TestBinary" };
    const Binary expected{ 0x01, 0x02, 0x03, 0xFF };

    reg::Value value{ name, expected };

    EXPECT_EQ(value.name(), name);
    EXPECT_EQ(value.type(), reg::ValueType::Binary);
    EXPECT_EQ(value.data<Binary>(), expected);
}

// --- raw 바이트 버퍼로부터의 생성 (Read/Enum용) ---

TEST(ValueTest, RawBufferConstructor_Dword_ParsesValue)
{
    constexpr std::wstring_view name{ L"RawDword" };
    constexpr Dword expected{ 0xABCD };
    const std::vector<Byte> bytes{ ToBytes(expected) };

    reg::Value value{ name, reg::ValueType::Dword, bytes };

    EXPECT_EQ(value.type(), reg::ValueType::Dword);
    EXPECT_EQ(value.data<Dword>(), expected);
}

TEST(ValueTest, RawBufferConstructor_Qword_ParsesValue)
{
    constexpr std::wstring_view name{ L"RawQword" };
    constexpr Qword expected{ 0x1122334455667788ull };
    const std::vector<Byte> bytes{ ToBytes(expected) };

    reg::Value value{ name, reg::ValueType::Qword, bytes };

    EXPECT_EQ(value.type(), reg::ValueType::Qword);
    EXPECT_EQ(value.data<Qword>(), expected);
}

TEST(ValueTest, RawBufferConstructor_String_StripsTrailingNull)
{
    constexpr std::wstring_view name{ L"RawString" };
    constexpr std::wstring_view expected{ L"hello raw" };
    const std::vector<Byte> bytes{ ToBytes(expected, /*includeNull=*/true) };

    reg::Value value{ name, reg::ValueType::String, bytes };

    EXPECT_EQ(value.type(), reg::ValueType::String);
    EXPECT_EQ(value.data<String>(), expected);
}

TEST(ValueTest, RawBufferConstructor_String_WithoutTrailingNull_KeepsAllChars)
{
    constexpr std::wstring_view name{ L"RawStringNoNull" };
    constexpr std::wstring_view expected{ L"no null" };
    const std::vector<Byte> bytes{ ToBytes(expected, /*includeNull=*/false) };

    reg::Value value{ name, reg::ValueType::String, bytes };

    EXPECT_EQ(value.data<String>(), expected);
}

TEST(ValueTest, RawBufferConstructor_ExpandString_ParsesAsString)
{
    constexpr std::wstring_view name{ L"RawExpandString" };
    constexpr std::wstring_view expected{ L"%TEMP%\\mwl" };
    const std::vector<Byte> bytes{ ToBytes(expected) };

    reg::Value value{ name, reg::ValueType::ExpandString, bytes };

    EXPECT_EQ(value.type(), reg::ValueType::ExpandString);
    EXPECT_EQ(value.data<String>(), expected);
}

TEST(ValueTest, RawBufferConstructor_MultiString_ParsesAllStrings)
{
    constexpr std::wstring_view name{ L"RawMultiString" };
    const std::vector<wchar_t> chars{ L'A', L'\0', L'B', L'C', L'\0', L'\0' };
    std::vector<Byte> bytes(reinterpret_cast<const Byte*>(chars.data()),
                            reinterpret_cast<const Byte*>(chars.data() + chars.size()));

    reg::Value value{ name, reg::ValueType::MultiString, bytes };

    const MultiString expected{ L"A", L"BC" };
    EXPECT_EQ(value.type(), reg::ValueType::MultiString);
    EXPECT_EQ(value.data<MultiString>(), expected);
}

TEST(ValueTest, RawBufferConstructor_MultiString_EmptyBuffer_ReturnsEmpty)
{
    constexpr std::wstring_view name{ L"RawMultiStringEmpty" };
    const std::vector<Byte> bytes{};

    reg::Value value{ name, reg::ValueType::MultiString, bytes };

    EXPECT_TRUE(value.data<MultiString>().empty());
}

TEST(ValueTest, RawBufferConstructor_Binary_CopiesBytes)
{
    constexpr std::wstring_view name{ L"RawBinary" };
    const std::vector<Byte> expected{ 0x01, 0x02, 0x03, 0xFF };

    reg::Value value{ name, reg::ValueType::Binary, expected };

    EXPECT_EQ(value.type(), reg::ValueType::Binary);
    EXPECT_EQ(value.data<Binary>(), expected);
}

TEST(ValueTest, RawBufferConstructor_UnknownType_FallsBackToBinary)
{
    constexpr std::wstring_view name{ L"RawUnknown" };
    const std::vector<Byte> expected{ 0xDE, 0xAD, 0xBE, 0xEF };

    reg::Value value{ name, reg::ValueType::None, expected };

    EXPECT_EQ(value.type(), reg::ValueType::None);
    EXPECT_EQ(value.data<Binary>(), expected);
}
