#include <Windows.h>

#include <system_error>

#include "gtest/gtest.h"
#include "mwl/status/error.h"

static_assert(sizeof(mwl::Error) == sizeof(void*), "Error must be pointer-sized");

TEST(Error, DefaultConstructIsOk)
{
    mwl::Error e;
    EXPECT_TRUE(e.ok());
}

TEST(Error, NoErrorIsOk)
{
    EXPECT_TRUE(mwl::NoError().ok());
}

TEST(Error, OkOperatorBool)
{
    EXPECT_TRUE(static_cast<bool>(mwl::NoError()));
}

TEST(Error, OkMessageEmpty)
{
    EXPECT_TRUE(mwl::NoError().message().empty());
}

TEST(Error, OkCodeValueIsZero)
{
    EXPECT_EQ(mwl::NoError().code().value(), 0);
}

TEST(Error, OkToString)
{
    EXPECT_EQ(mwl::NoError().ToString(), "OK");
}

TEST(Error, NotFoundErrorNotOk)
{
    auto e = mwl::NotFoundError("file not found");
    EXPECT_FALSE(e.ok());
    EXPECT_FALSE(static_cast<bool>(e));
}

TEST(Error, NotFoundErrorCode)
{
    auto e = mwl::NotFoundError("x");
    EXPECT_EQ(e.code(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST(Error, NotFoundErrorMessage)
{
    auto e = mwl::NotFoundError("file not found");
    EXPECT_EQ(e.message(), "file not found");
}

TEST(Error, SourceLocationCaptured)
{
    auto e = mwl::NotFoundError("x");
    EXPECT_GT(e.location().line(), 0u);
    EXPECT_NE(std::string_view(e.location().file_name()).find("error_test"), std::string_view::npos);
}

TEST(Error, ToStringContainsMessage)
{
    auto e = mwl::NotFoundError("the message");
    EXPECT_NE(e.ToString().find("the message"), std::string::npos);
}

TEST(Error, CopyConstruct)
{
    auto a = mwl::NotFoundError("x");
    auto b = a;
    EXPECT_FALSE(b.ok());
    EXPECT_EQ(b.message(), "x");
    EXPECT_EQ(b.code(), a.code());
}

TEST(Error, MoveConstruct)
{
    auto a = mwl::NotFoundError("x");
    auto b = std::move(a);
    EXPECT_FALSE(b.ok());
    EXPECT_EQ(b.message(), "x");
}

TEST(Error, AllUpperLayerFactoriesNotOk)
{
    EXPECT_FALSE(mwl::InvalidArgumentError("x").ok());
    EXPECT_FALSE(mwl::NotFoundError("x").ok());
    EXPECT_FALSE(mwl::PermissionDeniedError("x").ok());
    EXPECT_FALSE(mwl::AlreadyExistsError("x").ok());
    EXPECT_FALSE(mwl::ResourceExhaustedError("x").ok());
    EXPECT_FALSE(mwl::TimedOutError("x").ok());
    EXPECT_FALSE(mwl::NotSupportedError("x").ok());
}

TEST(Error, LastWindowsErrorCategory)
{
    ::SetLastError(ERROR_ACCESS_DENIED);
    auto e = mwl::LastWindowsError("access denied");
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_ACCESS_DENIED));
}

TEST(Error, LstatusErrorCategory)
{
    auto e = mwl::LstatusError("key not found", ERROR_FILE_NOT_FOUND);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_FILE_NOT_FOUND));
}

TEST(Error, HresultSucceeded)
{
    EXPECT_TRUE(mwl::HresultError("ok", S_OK).ok());
}

TEST(Error, HresultFacilityWin32)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    auto e = mwl::HresultError("access denied", hr);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_ACCESS_DENIED));
}

TEST(Error, HresultComError)
{
    auto e = mwl::HresultError("no interface", E_NOINTERFACE);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), mwl::hresult_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(E_NOINTERFACE));
}
