#include <gtest/gtest.h>
#include <mwl/windows/handle/registry_handle.h>
#include <mwl/windows/registry/registry.h>

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
        key_.Reset();
        static_cast<void>(reg::DeleteKey(RegistryHandleView{ HKEY_CURRENT_USER }, kTestSubKey));
    }

    UniqueRegistryHandle key_;
};

// --- 키 관리 ---

TEST_F(RegistryTest, CreateKey_Succeeds)
{
    EXPECT_NE(key_.Get(), nullptr);
}

TEST_F(RegistryTest, OpenKey_Existing_Succeeds)
{
    auto result = reg::OpenKey(RegistryHandleView{ HKEY_CURRENT_USER }, kTestSubKey, KEY_READ);
    AssertOk(result, "OpenKey existing");

    EXPECT_NE(result.value().Get(), nullptr);
}

TEST_F(RegistryTest, OpenKey_Nonexistent_Fails)
{
    auto result = reg::OpenKey(RegistryHandleView{ HKEY_CURRENT_USER }, LR"(SOFTWARE\MwlTestDoesNotExist)", KEY_READ);

    EXPECT_FALSE(result.ok());
}

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

// --- 값 읽기/쓰기 round-trip ---

TEST_F(RegistryTest, WriteReadString_RoundTrip)
{
    constexpr std::wstring_view name{ L"TestString" };
    constexpr std::wstring_view expected{ L"Hello, mwl!" };

    AssertOk(reg::WriteString(key_, L"", name, expected), "WriteString");

    auto read = reg::ReadString(key_, L"", name);
    AssertOk(read, "ReadString");
    EXPECT_EQ(read.value(), expected);
}

TEST_F(RegistryTest, WriteReadDword_RoundTrip)
{
    constexpr std::wstring_view name{ L"TestDword" };
    constexpr DWORD expected{ 0x1234 };

    AssertOk(reg::WriteDword(key_, L"", name, expected), "WriteDword");

    auto read = reg::ReadDword(key_, L"", name);
    AssertOk(read, "ReadDword");
    EXPECT_EQ(read.value(), expected);
}

TEST_F(RegistryTest, WriteReadQword_RoundTrip)
{
    constexpr std::wstring_view name{ L"TestQword" };
    constexpr DWORD64 expected{ 0x1122334455ull };

    AssertOk(reg::WriteQword(key_, L"", name, expected), "WriteQword");

    auto read = reg::ReadQword(key_, L"", name);
    AssertOk(read, "ReadQword");
    EXPECT_EQ(read.value(), expected);
}

// --- 값 읽기 실패 / 삭제 ---

TEST_F(RegistryTest, ReadString_MissingValue_Fails)
{
    auto read = reg::ReadString(key_, L"", L"NoSuchValue");

    EXPECT_FALSE(read.ok());
}

TEST_F(RegistryTest, DeleteValue_RemovesValue)
{
    constexpr std::wstring_view name{ L"ToDelete" };

    AssertOk(reg::WriteString(key_, L"", name, L"temp"), "DeleteValue precondition WriteString");

    auto deleted = reg::DeleteValue(key_, name);
    AssertOk(deleted, "DeleteValue");

    auto read = reg::ReadString(key_, L"", name);
    EXPECT_FALSE(read.ok());
}
