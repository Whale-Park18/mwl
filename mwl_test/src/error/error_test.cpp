#include "gtest/gtest.h"

#include <Windows.h>

#include <system_error>

#include "mwl/error/error.h"

static_assert(sizeof(mwl::error::Error) == sizeof(void*), "Error must be pointer-sized");

TEST(Error, DefaultConstructIsOk)
{
    mwl::error::Error e;
    EXPECT_TRUE(e.ok());
}

TEST(Error, NoErrorIsOk)
{
    EXPECT_TRUE(mwl::error::NoError().ok());
}

TEST(Error, OkOperatorBool)
{
    EXPECT_TRUE(static_cast<bool>(mwl::error::NoError()));
}

TEST(Error, OkMessageEmpty)
{
    EXPECT_TRUE(mwl::error::NoError().message().empty());
}

TEST(Error, OkCodeValueIsZero)
{
    EXPECT_EQ(mwl::error::NoError().code().value(), 0);
}

TEST(Error, OkToString)
{
    EXPECT_EQ(mwl::error::NoError().ToString(), "OK");
}

TEST(Error, NotFoundErrorNotOk)
{
    auto e = mwl::error::NotFoundError("file not found");
    EXPECT_FALSE(e.ok());
    EXPECT_FALSE(static_cast<bool>(e));
}

TEST(Error, NotFoundErrorCode)
{
    auto e = mwl::error::NotFoundError("x");
    EXPECT_EQ(e.code(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST(Error, NotFoundErrorMessage)
{
    auto e = mwl::error::NotFoundError("file not found");
    EXPECT_EQ(e.message(), "file not found");
}

TEST(Error, SourceLocationCaptured)
{
    auto e = mwl::error::NotFoundError("x");
    EXPECT_GT(e.location().line(), 0u);
    EXPECT_NE(std::string_view(e.location().file_name()).find("error_test"), std::string_view::npos);
}

TEST(Error, ToStringContainsMessage)
{
    auto e = mwl::error::NotFoundError("the message");
    EXPECT_NE(e.ToString().find("the message"), std::string::npos);
}

TEST(Error, CopyConstruct)
{
    auto a = mwl::error::NotFoundError("x");
    auto b = a;
    EXPECT_FALSE(b.ok());
    EXPECT_EQ(b.message(), "x");
    EXPECT_EQ(b.code(), a.code());
}

TEST(Error, MoveConstruct)
{
    auto a = mwl::error::NotFoundError("x");
    auto b = std::move(a);
    EXPECT_FALSE(b.ok());
    EXPECT_EQ(b.message(), "x");
}

TEST(Error, AllUpperLayerFactoriesNotOk)
{
    EXPECT_FALSE(mwl::error::InvalidArgumentError("x").ok());
    EXPECT_FALSE(mwl::error::NotFoundError("x").ok());
    EXPECT_FALSE(mwl::error::PermissionDeniedError("x").ok());
    EXPECT_FALSE(mwl::error::AlreadyExistsError("x").ok());
    EXPECT_FALSE(mwl::error::ResourceExhaustedError("x").ok());
    EXPECT_FALSE(mwl::error::TimedOutError("x").ok());
    EXPECT_FALSE(mwl::error::NotSupportedError("x").ok());
}

TEST(Error, LastWindowsErrorCategory)
{
    ::SetLastError(ERROR_ACCESS_DENIED);
    auto e = mwl::error::LastWindowsError("access denied");
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_ACCESS_DENIED));
}

TEST(Error, LstatusErrorCategory)
{
    auto e = mwl::error::LstatusError("key not found", ERROR_FILE_NOT_FOUND);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_FILE_NOT_FOUND));
}

TEST(Error, HresultSucceeded)
{
    EXPECT_TRUE(mwl::error::HresultError("ok", S_OK).ok());
}

TEST(Error, HresultFacilityWin32)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    auto e = mwl::error::HresultError("access denied", hr);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_ACCESS_DENIED));
}

TEST(Error, HresultComError)
{
    auto e = mwl::error::HresultError("no interface", E_NOINTERFACE);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), mwl::error::hresult_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(E_NOINTERFACE));
}
