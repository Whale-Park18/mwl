#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "mwl/status/result.h"

//***************************************************************************
// 값/에러 생성 및 접근
//***************************************************************************

// 값으로 생성한 Result가 성공(ok) 상태인지 확인한다.
TEST(Result, ValueConstructOk)
{
    mwl::Result<int> r(42);
    EXPECT_TRUE(r.ok());
    EXPECT_TRUE(static_cast<bool>(r));
}

// value()가 저장된 값을 그대로 반환하는지 확인한다.
TEST(Result, ValueAccess)
{
    mwl::Result<int> r(42);
    EXPECT_EQ(r.value(), 42);
}

// Error로 생성한 Result가 실패(not ok) 상태인지 확인한다.
TEST(Result, ErrorConstructNotOk)
{
    mwl::Result<int> r(mwl::NotFoundError("x"));
    EXPECT_FALSE(r.ok());
    EXPECT_FALSE(static_cast<bool>(r));
}

// error()가 저장된 에러의 코드를 그대로 반환하는지 확인한다.
TEST(Result, ErrorAccess)
{
    mwl::Result<int> r(mwl::NotFoundError("x"));
    EXPECT_EQ(r.error().code(), std::make_error_code(std::errc::no_such_file_or_directory));
}

//***************************************************************************
// 이동 시맨틱
//***************************************************************************

// rvalue Result에서 value()를 호출하면 내부 값이 이동되어 나오는지 확인한다.
TEST(Result, MoveValueOut)
{
    mwl::Result<std::string> r(std::string{ "hello" });
    std::string v = std::move(r).value();
    EXPECT_EQ(v, "hello");
}

// move-only 타입(unique_ptr)도 Result에 담아 이동할 수 있는지 확인한다.
TEST(Result, MoveOnlyType)
{
    mwl::Result<std::unique_ptr<int>> r(std::make_unique<int>(7));
    EXPECT_TRUE(r.ok());
    auto ptr = std::move(r).value();
    EXPECT_EQ(*ptr, 7);
}

//***************************************************************************
// 복사/이동 생성
//***************************************************************************

// 복사 생성된 Result가 원본과 같은 값을 갖는지 확인한다.
TEST(Result, CopyConstruct)
{
    mwl::Result<int> a(42);
    mwl::Result<int> b = a;
    EXPECT_EQ(b.value(), 42);
}

// 이동 생성된 Result가 원본의 값을 그대로 전달받는지 확인한다.
TEST(Result, MoveConstruct)
{
    mwl::Result<int> a(42);
    mwl::Result<int> b = std::move(a);
    EXPECT_EQ(b.value(), 42);
}

//***************************************************************************
// 타입 다양성
//***************************************************************************

// std::string처럼 값 타입이 다른 경우에도 Result가 올바르게 동작하는지 확인한다.
TEST(Result, StringType)
{
    mwl::Result<std::string> r(std::string("test"));
    EXPECT_TRUE(r.ok());
    EXPECT_EQ(r.value(), "test");
}
