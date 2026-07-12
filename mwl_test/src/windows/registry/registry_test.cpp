#include <gtest/gtest.h>
#include <mwl/windows/handle/registry_handle.h>
#include <mwl/windows/registry/registry.h>

#include <algorithm>
#include <format>
#include <string>
#include <string_view>

namespace
{

namespace reg = mwl::windows::registry;
using mwl::windows::handle::RegistryHandleView;
using mwl::windows::handle::UniqueRegistryHandle;

constexpr std::wstring_view kTestSubKey{ LR"(SOFTWARE\MwlTest)" };

// Result가 실패하면 메시지와 함께 테스트를 즉시 중단한다.
template<typename T>
void AssertOk(const mwl::Result<T>& result, std::string_view context)
{
    ASSERT_TRUE(result.ok()) << std::format("{} — code: {} / msg: {}", context, result.error().code().value(),
                                            result.error().message().data());
}

} // namespace

class RegistryTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        auto result = reg::CreateKey(RegistryHandleView{ HKEY_CURRENT_USER }, kTestSubKey);
        ASSERT_TRUE(result.ok()) << std::format("SetUp CreateKey — code: {} / msg: {}", result.error().code().value(),
                                                result.error().message().data());
        key_ = std::move(result.value());
    }

    void TearDown() override
    {
        // 테스트 중 생성된 서브 키가 남아 있으면 RegDeleteKeyW(kTestSubKey)가 실패하므로,
        // assertion 실패로 본문의 정리 코드가 건너뛰어진 경우를 대비해 먼저 정리한다.
        if (key_)
        {
            auto subKeys = reg::EnumSubKeys(key_);
            if (subKeys.ok())
            {
                for (const auto& subKey : subKeys.value())
                {
                    static_cast<void>(reg::DeleteKey(key_, subKey));
                }
            }
        }

        key_.Reset();
        static_cast<void>(reg::DeleteKey(RegistryHandleView{ HKEY_CURRENT_USER }, kTestSubKey));
    }

    UniqueRegistryHandle key_;
};

//***************************************************************************
// 키 관리
//***************************************************************************

// 새 서브 키를 생성하면 유효한 핸들을 반환하는지 확인한다.
TEST_F(RegistryTest, CreateKey_Succeeds)
{
    EXPECT_NE(key_.Get(), nullptr);
}

// 존재하는 서브 키를 KEY_READ로 여는 데 성공하는지 확인한다.
TEST_F(RegistryTest, OpenKey_Existing_Succeeds)
{
    auto result = reg::OpenKey(RegistryHandleView{ HKEY_CURRENT_USER }, kTestSubKey, KEY_READ);
    AssertOk(result, "OpenKey existing");

    EXPECT_NE(result.value().Get(), nullptr);
}

// 존재하지 않는 서브 키를 열면 실패를 반환하는지 확인한다.
TEST_F(RegistryTest, OpenKey_Nonexistent_Fails)
{
    auto result = reg::OpenKey(RegistryHandleView{ HKEY_CURRENT_USER }, LR"(SOFTWARE\MwlTestDoesNotExist)", KEY_READ);

    EXPECT_FALSE(result.ok());
}

// 서브 키 삭제 후 같은 이름으로 다시 열면 실패하는지 확인한다.
TEST_F(RegistryTest, DeleteKey_RemovesKey)
{
    constexpr std::wstring_view child{ L"Child" };

    auto created = reg::CreateKey(key_, child);
    AssertOk(created, "DeleteKey precondition CreateKey");

    auto deleted = reg::DeleteKey(key_, child);
    AssertOk(deleted, "DeleteKey");

    auto reopened = reg::OpenKey(key_, child, KEY_READ);
    EXPECT_FALSE(reopened.ok());
}

//***************************************************************************
// 값 읽기/쓰기 round-trip
//***************************************************************************

// REG_SZ 값을 쓰고 다시 읽으면 원본과 같은지 확인한다.
TEST_F(RegistryTest, WriteReadString_RoundTrip)
{
    constexpr std::wstring_view name{ L"TestString" };
    constexpr std::wstring_view expected{ L"Hello, mwl!" };

    AssertOk(reg::WriteString(key_, L"", name, expected), "WriteString");

    auto read = reg::ReadString(key_, L"", name);
    AssertOk(read, "ReadString");
    EXPECT_EQ(read.value(), expected);
}

// REG_DWORD 값을 쓰고 다시 읽으면 원본과 같은지 확인한다.
TEST_F(RegistryTest, WriteReadDword_RoundTrip)
{
    constexpr std::wstring_view name{ L"TestDword" };
    constexpr DWORD expected{ 0x1234 };

    AssertOk(reg::WriteDword(key_, L"", name, expected), "WriteDword");

    auto read = reg::ReadDword(key_, L"", name);
    AssertOk(read, "ReadDword");
    EXPECT_EQ(read.value(), expected);
}

// REG_QWORD 값을 쓰고 다시 읽으면 원본과 같은지 확인한다.
TEST_F(RegistryTest, WriteReadQword_RoundTrip)
{
    constexpr std::wstring_view name{ L"TestQword" };
    constexpr DWORD64 expected{ 0x1122334455ull };

    AssertOk(reg::WriteQword(key_, L"", name, expected), "WriteQword");

    auto read = reg::ReadQword(key_, L"", name);
    AssertOk(read, "ReadQword");
    EXPECT_EQ(read.value(), expected);
}

//***************************************************************************
// 값 읽기 실패 / 삭제
//***************************************************************************

// 존재하지 않는 값을 읽으면 실패를 반환하는지 확인한다.
TEST_F(RegistryTest, ReadString_MissingValue_Fails)
{
    auto read = reg::ReadString(key_, L"", L"NoSuchValue");

    EXPECT_FALSE(read.ok());
}

// 값을 삭제한 후 다시 읽으면 실패하는지 확인한다.
TEST_F(RegistryTest, DeleteValue_RemovesValue)
{
    constexpr std::wstring_view name{ L"ToDelete" };

    AssertOk(reg::WriteString(key_, L"", name, L"temp"), "DeleteValue precondition WriteString");

    auto deleted = reg::DeleteValue(key_, name);
    AssertOk(deleted, "DeleteValue");

    auto read = reg::ReadString(key_, L"", name);
    EXPECT_FALSE(read.ok());
}

//***************************************************************************
// 서브 키 나열
//***************************************************************************

// 서브 키가 없는 키를 나열하면 빈 목록을 반환하는지 확인한다.
TEST_F(RegistryTest, EnumSubKeys_EmptyKey_ReturnsEmpty)
{
    auto result = reg::EnumSubKeys(key_);
    AssertOk(result, "EnumSubKeys empty");

    EXPECT_TRUE(result.value().empty());
}

// 서브 키가 있는 키를 나열하면 모든 서브 키 이름이 포함되는지 확인한다.
TEST_F(RegistryTest, EnumSubKeys_WithChildren_ReturnsAllNames)
{
    {
        auto a = reg::CreateKey(key_, L"ChildA");
        AssertOk(a, "CreateKey ChildA");
        auto b = reg::CreateKey(key_, L"ChildB");
        AssertOk(b, "CreateKey ChildB");
    }

    auto result = reg::EnumSubKeys(key_);
    AssertOk(result, "EnumSubKeys");

    const auto& keys = result.value();
    EXPECT_EQ(keys.size(), 2u);
    EXPECT_NE(std::find(keys.begin(), keys.end(), std::wstring(L"ChildA")), keys.end());
    EXPECT_NE(std::find(keys.begin(), keys.end(), std::wstring(L"ChildB")), keys.end());
}

//***************************************************************************
// 값 나열
//***************************************************************************

// 값이 없는 키를 나열하면 빈 목록을 반환하는지 확인한다.
TEST_F(RegistryTest, EnumValues_EmptyKey_ReturnsEmpty)
{
    auto result = reg::EnumValues(key_);
    AssertOk(result, "EnumValues empty");

    EXPECT_TRUE(result.value().empty());
}

// DWORD 값을 나열했을 때 타입과 데이터가 올바르게 파싱되는지 확인한다.
TEST_F(RegistryTest, EnumValues_DwordValue_HasCorrectTypeAndData)
{
    constexpr std::wstring_view name{ L"EnumDword" };
    constexpr DWORD expected{ 0xABCD };

    AssertOk(reg::WriteDword(key_, L"", name, expected), "WriteDword for enum");

    auto result = reg::EnumValues(key_);
    AssertOk(result, "EnumValues");

    const auto& values = result.value();
    auto it = std::find_if(values.begin(), values.end(), [&](const reg::Value& v) { return v.name() == name; });
    ASSERT_NE(it, values.end());
    EXPECT_EQ(it->type(), reg::ValueType::Dword);
    EXPECT_EQ(it->data<DWORD>(), expected);
}

// QWORD 값을 나열했을 때 타입과 데이터가 올바르게 파싱되는지 확인한다.
TEST_F(RegistryTest, EnumValues_QwordValue_HasCorrectTypeAndData)
{
    constexpr std::wstring_view name{ L"EnumQword" };
    constexpr DWORD64 expected{ 0x1122334455667788ull };

    AssertOk(reg::WriteQword(key_, L"", name, expected), "WriteQword for enum");

    auto result = reg::EnumValues(key_);
    AssertOk(result, "EnumValues");

    const auto& values = result.value();
    auto it = std::find_if(values.begin(), values.end(), [&](const reg::Value& v) { return v.name() == name; });
    ASSERT_NE(it, values.end());
    EXPECT_EQ(it->type(), reg::ValueType::Qword);
    EXPECT_EQ(it->data<DWORD64>(), expected);
}

// 문자열 값을 나열했을 때 타입과 데이터가 올바르게 파싱되는지 확인한다.
TEST_F(RegistryTest, EnumValues_StringValue_HasCorrectTypeAndData)
{
    constexpr std::wstring_view name{ L"EnumString" };
    constexpr std::wstring_view expected{ L"hello enum" };

    AssertOk(reg::WriteString(key_, L"", name, expected), "WriteString for enum");

    auto result = reg::EnumValues(key_);
    AssertOk(result, "EnumValues");

    const auto& values = result.value();
    auto it = std::find_if(values.begin(), values.end(), [&](const reg::Value& v) { return v.name() == name; });
    ASSERT_NE(it, values.end());
    EXPECT_EQ(it->type(), reg::ValueType::String);
    EXPECT_EQ(it->data<std::wstring>(), expected);
}

// 여러 값을 쓴 뒤 나열하면 개수가 일치하는지 확인한다.
TEST_F(RegistryTest, EnumValues_MultipleValues_CountMatches)
{
    AssertOk(reg::WriteDword(key_, L"", L"V1", 1u), "WriteDword V1");
    AssertOk(reg::WriteQword(key_, L"", L"V2", 2ull), "WriteQword V2");
    AssertOk(reg::WriteString(key_, L"", L"V3", L"three"), "WriteString V3");

    auto result = reg::EnumValues(key_);
    AssertOk(result, "EnumValues multi");

    EXPECT_EQ(result.value().size(), 3u);
}
