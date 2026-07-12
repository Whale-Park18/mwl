#include <Windows.h>

#include <system_error>

#include "gtest/gtest.h"
#include "mwl/status/error.h"

static_assert(sizeof(mwl::Error) == sizeof(void*), "Error must be pointer-sized");

//***************************************************************************
// 성공(OK) 상태
//***************************************************************************

// 기본 생성된 Error는 성공(ok) 상태인지 확인한다.
TEST(Error, DefaultConstructIsOk)
{
    mwl::Error e;
    EXPECT_TRUE(e.ok());
}

// NoError()가 성공 상태를 반환하는지 확인한다.
TEST(Error, NoErrorIsOk)
{
    EXPECT_TRUE(mwl::NoError().ok());
}

// 성공 상태에서 operator bool이 true를 반환하는지 확인한다.
TEST(Error, OkOperatorBool)
{
    EXPECT_TRUE(static_cast<bool>(mwl::NoError()));
}

// 성공 상태의 메시지가 비어 있는지 확인한다.
TEST(Error, OkMessageEmpty)
{
    EXPECT_TRUE(mwl::NoError().message().empty());
}

// 성공 상태의 에러 코드 값이 0인지 확인한다.
TEST(Error, OkCodeValueIsZero)
{
    EXPECT_EQ(mwl::NoError().code().value(), 0);
}

// 성공 상태의 ToString()이 "OK"를 반환하는지 확인한다.
TEST(Error, OkToString)
{
    EXPECT_EQ(mwl::NoError().ToString(), "OK");
}

//***************************************************************************
// 에러 생성 및 속성
//***************************************************************************

// NotFoundError가 실패(not ok) 상태를 만드는지 확인한다.
TEST(Error, NotFoundErrorNotOk)
{
    auto e = mwl::NotFoundError("file not found");
    EXPECT_FALSE(e.ok());
    EXPECT_FALSE(static_cast<bool>(e));
}

// NotFoundError의 에러 코드가 no_such_file_or_directory와 일치하는지 확인한다.
TEST(Error, NotFoundErrorCode)
{
    auto e = mwl::NotFoundError("x");
    EXPECT_EQ(e.code(), std::make_error_code(std::errc::no_such_file_or_directory));
}

// NotFoundError가 전달한 메시지를 그대로 보존하는지 확인한다.
TEST(Error, NotFoundErrorMessage)
{
    auto e = mwl::NotFoundError("file not found");
    EXPECT_EQ(e.message(), "file not found");
}

// 에러 생성 시 소스 위치(파일/라인)가 올바르게 캡처되는지 확인한다.
TEST(Error, SourceLocationCaptured)
{
    auto e = mwl::NotFoundError("x");
    EXPECT_GT(e.location().line(), 0u);
    EXPECT_NE(std::string_view(e.location().file_name()).find("error_test"), std::string_view::npos);
}

// ToString() 결과에 원본 메시지가 포함되는지 확인한다.
TEST(Error, ToStringContainsMessage)
{
    auto e = mwl::NotFoundError("the message");
    EXPECT_NE(e.ToString().find("the message"), std::string::npos);
}

//***************************************************************************
// 복사/이동
//***************************************************************************

// 복사 생성된 Error가 원본과 동일한 상태(메시지/코드)를 갖는지 확인한다.
TEST(Error, CopyConstruct)
{
    auto a = mwl::NotFoundError("x");
    auto b = a;
    EXPECT_FALSE(b.ok());
    EXPECT_EQ(b.message(), "x");
    EXPECT_EQ(b.code(), a.code());
}

// 이동 생성된 Error가 원본의 상태를 그대로 전달받는지 확인한다.
TEST(Error, MoveConstruct)
{
    auto a = mwl::NotFoundError("x");
    auto b = std::move(a);
    EXPECT_FALSE(b.ok());
    EXPECT_EQ(b.message(), "x");
}

//***************************************************************************
// 상위 레이어 팩토리 함수
//***************************************************************************

// 모든 상위 레이어 팩토리 함수가 실패 상태의 Error를 생성하는지 확인한다.
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

//***************************************************************************
// 플랫폼 에러 카테고리 (Windows/LSTATUS/HRESULT)
//***************************************************************************

// LastWindowsError가 SetLastError 값을 system_category로 감싸는지 확인한다.
TEST(Error, LastWindowsErrorCategory)
{
    ::SetLastError(ERROR_ACCESS_DENIED);
    auto e = mwl::LastWindowsError("access denied");
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_ACCESS_DENIED));
}

// LstatusError가 LSTATUS 값을 system_category로 감싸는지 확인한다.
TEST(Error, LstatusErrorCategory)
{
    auto e = mwl::LstatusError("key not found", ERROR_FILE_NOT_FOUND);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_FILE_NOT_FOUND));
}

// HresultError가 성공 HRESULT(S_OK)에 대해 ok 상태를 반환하는지 확인한다.
TEST(Error, HresultSucceeded)
{
    EXPECT_TRUE(mwl::HresultError("ok", S_OK).ok());
}

// FACILITY_WIN32 HRESULT가 system_category로 매핑되는지 확인한다.
TEST(Error, HresultFacilityWin32)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    auto e = mwl::HresultError("access denied", hr);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), std::system_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(ERROR_ACCESS_DENIED));
}

// 일반 COM HRESULT가 hresult_category로 매핑되는지 확인한다.
TEST(Error, HresultComError)
{
    auto e = mwl::HresultError("no interface", E_NOINTERFACE);
    EXPECT_FALSE(e.ok());
    EXPECT_EQ(e.code().category(), mwl::hresult_category());
    EXPECT_EQ(e.code().value(), static_cast<int>(E_NOINTERFACE));
}
