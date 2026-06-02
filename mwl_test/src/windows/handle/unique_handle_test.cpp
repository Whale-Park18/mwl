#include <gtest/gtest.h>
#include <mwl/windows/handle/unique_handle.h>

namespace
{

struct TestTraits
{
    using Type = int;

    static int close_count;

    static Type Empty() noexcept
    {
        return 0;
    }

    static bool IsValid(Type handle) noexcept
    {
        return handle != 0;
    }

    static void Close(Type) noexcept
    {
        ++close_count;
    }
};

int TestTraits::close_count = 0;

using TestHandle = mwl::windows::handle::UniqueHandle<TestTraits>;

} // namespace

class UniqueHandleTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        TestTraits::close_count = 0;
    }
};

// --- 생성 ---

TEST_F(UniqueHandleTest, DefaultConstruction_IsInvalid)
{
    TestHandle h;

    EXPECT_FALSE(static_cast<bool>(h));
    EXPECT_EQ(h.Get(), TestTraits::Empty());
}

TEST_F(UniqueHandleTest, ConstructFromValidHandle_IsValid)
{
    TestHandle h(42);

    EXPECT_TRUE(static_cast<bool>(h));
    EXPECT_EQ(h.Get(), 42);
}

TEST_F(UniqueHandleTest, ConstructFromEmptyHandle_IsInvalid)
{
    TestHandle h(TestTraits::Empty());

    EXPECT_FALSE(static_cast<bool>(h));
}

// --- 이동 ---

TEST_F(UniqueHandleTest, MoveConstruction_TransfersOwnership)
{
    TestHandle src(42);
    TestHandle dst(std::move(src));

    EXPECT_EQ(dst.Get(), 42);
    EXPECT_FALSE(static_cast<bool>(src));
    EXPECT_EQ(TestTraits::close_count, 0);
}

TEST_F(UniqueHandleTest, MoveAssignment_ClosesOldHandle)
{
    TestHandle src(42);
    TestHandle dst(99);
    dst = std::move(src);

    EXPECT_EQ(dst.Get(), 42);
    EXPECT_FALSE(static_cast<bool>(src));
    EXPECT_EQ(TestTraits::close_count, 1); // dst의 기존 핸들(99)이 닫혀야 함
}

TEST_F(UniqueHandleTest, MoveAssignment_ToEmpty_NoClose)
{
    TestHandle src(42);
    TestHandle dst;
    dst = std::move(src);

    EXPECT_EQ(dst.Get(), 42);
    EXPECT_FALSE(static_cast<bool>(src));
    EXPECT_EQ(TestTraits::close_count, 0);
}

// --- operator bool / operator Handle ---

TEST_F(UniqueHandleTest, OperatorBool_ValidHandle_True)
{
    TestHandle h(1);
    EXPECT_TRUE(static_cast<bool>(h));
}

TEST_F(UniqueHandleTest, OperatorBool_InvalidHandle_False)
{
    TestHandle h;
    EXPECT_FALSE(static_cast<bool>(h));
}

TEST_F(UniqueHandleTest, OperatorHandle_ReturnsHandleValue)
{
    TestHandle h(42);
    EXPECT_EQ(static_cast<TestTraits::Type>(h), 42);
}

// --- Get ---

TEST_F(UniqueHandleTest, Get_ReturnsHandleValue)
{
    TestHandle h(99);
    EXPECT_EQ(h.Get(), 99);
}

TEST_F(UniqueHandleTest, Get_DefaultHandle_ReturnsEmpty)
{
    TestHandle h;
    EXPECT_EQ(h.Get(), TestTraits::Empty());
}

// --- Release ---

TEST_F(UniqueHandleTest, Release_ReturnsHandleAndBecomesInvalid)
{
    TestHandle h(42);
    auto raw = h.Release();

    EXPECT_EQ(raw, 42);
    EXPECT_FALSE(static_cast<bool>(h));
    EXPECT_EQ(TestTraits::close_count, 0); // Close 호출 없음
}

TEST_F(UniqueHandleTest, Release_InvalidHandle_ReturnsEmpty)
{
    TestHandle h;
    auto raw = h.Release();

    EXPECT_EQ(raw, TestTraits::Empty());
    EXPECT_EQ(TestTraits::close_count, 0);
}

// --- Reset ---

TEST_F(UniqueHandleTest, Reset_NoArg_ClosesValidHandle)
{
    TestHandle h(42);
    h.Reset();

    EXPECT_FALSE(static_cast<bool>(h));
    EXPECT_EQ(TestTraits::close_count, 1);
}

TEST_F(UniqueHandleTest, Reset_NoArg_InvalidHandle_DoesNotClose)
{
    TestHandle h;
    h.Reset();

    EXPECT_EQ(TestTraits::close_count, 0);
}

TEST_F(UniqueHandleTest, Reset_SameHandle_IsNoOp)
{
    TestHandle h(42);
    h.Reset(42);

    EXPECT_EQ(h.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

TEST_F(UniqueHandleTest, Reset_DifferentHandle_ClosesOldAndSetsNew)
{
    TestHandle h(42);
    h.Reset(99);

    EXPECT_EQ(h.Get(), 99);
    EXPECT_EQ(TestTraits::close_count, 1);
}

// --- Swap ---

TEST_F(UniqueHandleTest, Swap_ExchangesHandles)
{
    TestHandle a(42);
    TestHandle b(99);
    a.Swap(b);

    EXPECT_EQ(a.Get(), 99);
    EXPECT_EQ(b.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

TEST_F(UniqueHandleTest, Swap_WithInvalid_TransfersHandle)
{
    TestHandle a(42);
    TestHandle b;
    a.Swap(b);

    EXPECT_FALSE(static_cast<bool>(a));
    EXPECT_EQ(b.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

// --- 소멸자 ---

TEST_F(UniqueHandleTest, Destructor_ValidHandle_CallsClose)
{
    {
        TestHandle h(42);
    }

    EXPECT_EQ(TestTraits::close_count, 1);
}

TEST_F(UniqueHandleTest, Destructor_InvalidHandle_DoesNotCallClose)
{
    {
        TestHandle h;
    }

    EXPECT_EQ(TestTraits::close_count, 0);
}
