#include "gtest/gtest.h"

#include <memory>
#include <string>

#include "mwl/status/result.h"

TEST(Result, ValueConstructOk)
{
    mwl::Result<int> r(42);
    EXPECT_TRUE(r.ok());
    EXPECT_TRUE(static_cast<bool>(r));
}

TEST(Result, ValueAccess)
{
    mwl::Result<int> r(42);
    EXPECT_EQ(r.value(), 42);
}

TEST(Result, ErrorConstructNotOk)
{
    mwl::Result<int> r(mwl::NotFoundError("x"));
    EXPECT_FALSE(r.ok());
    EXPECT_FALSE(static_cast<bool>(r));
}

TEST(Result, ErrorAccess)
{
    mwl::Result<int> r(mwl::NotFoundError("x"));
    EXPECT_EQ(r.error().code(), std::make_error_code(std::errc::no_such_file_or_directory));
}

TEST(Result, MoveValueOut)
{
    mwl::Result<std::string> r(std::string{ "hello" });
    std::string v = std::move(r).value();
    EXPECT_EQ(v, "hello");
}

TEST(Result, MoveOnlyType)
{
    mwl::Result<std::unique_ptr<int>> r(std::make_unique<int>(7));
    EXPECT_TRUE(r.ok());
    auto ptr = std::move(r).value();
    EXPECT_EQ(*ptr, 7);
}

TEST(Result, CopyConstruct)
{
    mwl::Result<int> a(42);
    mwl::Result<int> b = a;
    EXPECT_EQ(b.value(), 42);
}

TEST(Result, MoveConstruct)
{
    mwl::Result<int> a(42);
    mwl::Result<int> b = std::move(a);
    EXPECT_EQ(b.value(), 42);
}

TEST(Result, StringType)
{
    mwl::Result<std::string> r(std::string("test"));
    EXPECT_TRUE(r.ok());
    EXPECT_EQ(r.value(), "test");
}
