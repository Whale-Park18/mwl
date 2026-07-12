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

//***************************************************************************
// 생성
//***************************************************************************

// 기본 생성된 핸들은 유효하지 않은 상태인지 확인한다.
TEST_F(UniqueHandleTest, DefaultConstruction_IsInvalid)
{
    TestHandle h;

    EXPECT_FALSE(static_cast<bool>(h));
    EXPECT_EQ(h.Get(), TestTraits::Empty());
}

// 유효한 값으로 생성하면 유효한 상태가 되는지 확인한다.
TEST_F(UniqueHandleTest, ConstructFromValidHandle_IsValid)
{
    TestHandle h(42);

    EXPECT_TRUE(static_cast<bool>(h));
    EXPECT_EQ(h.Get(), 42);
}

// Empty() 값으로 생성하면 유효하지 않은 상태가 되는지 확인한다.
TEST_F(UniqueHandleTest, ConstructFromEmptyHandle_IsInvalid)
{
    TestHandle h(TestTraits::Empty());

    EXPECT_FALSE(static_cast<bool>(h));
}

//***************************************************************************
// 이동
//***************************************************************************

// 이동 생성 시 소유권이 이전되고 원본은 무효화되는지 확인한다.
TEST_F(UniqueHandleTest, MoveConstruction_TransfersOwnership)
{
    TestHandle src(42);
    TestHandle dst(std::move(src));

    EXPECT_EQ(dst.Get(), 42);
    EXPECT_FALSE(static_cast<bool>(src));
    EXPECT_EQ(TestTraits::close_count, 0);
}

// 이동 대입 시 대상이 이전에 갖고 있던 핸들이 닫히는지 확인한다.
TEST_F(UniqueHandleTest, MoveAssignment_ClosesOldHandle)
{
    TestHandle src(42);
    TestHandle dst(99);
    dst = std::move(src);

    EXPECT_EQ(dst.Get(), 42);
    EXPECT_FALSE(static_cast<bool>(src));
    EXPECT_EQ(TestTraits::close_count, 1); // dst의 기존 핸들(99)이 닫혀야 함
}

// 빈 핸들에 이동 대입할 때는 Close가 호출되지 않는지 확인한다.
TEST_F(UniqueHandleTest, MoveAssignment_ToEmpty_NoClose)
{
    TestHandle src(42);
    TestHandle dst;
    dst = std::move(src);

    EXPECT_EQ(dst.Get(), 42);
    EXPECT_FALSE(static_cast<bool>(src));
    EXPECT_EQ(TestTraits::close_count, 0);
}

//***************************************************************************
// operator bool / operator Handle
//***************************************************************************

// 유효한 핸들에서 operator bool이 true를 반환하는지 확인한다.
TEST_F(UniqueHandleTest, OperatorBool_ValidHandle_True)
{
    TestHandle h(1);
    EXPECT_TRUE(static_cast<bool>(h));
}

// 무효한 핸들에서 operator bool이 false를 반환하는지 확인한다.
TEST_F(UniqueHandleTest, OperatorBool_InvalidHandle_False)
{
    TestHandle h;
    EXPECT_FALSE(static_cast<bool>(h));
}

// operator Handle이 내부 핸들 값을 그대로 반환하는지 확인한다.
TEST_F(UniqueHandleTest, OperatorHandle_ReturnsHandleValue)
{
    TestHandle h(42);
    EXPECT_EQ(static_cast<TestTraits::Type>(h), 42);
}

//***************************************************************************
// Get
//***************************************************************************

// Get()이 내부 핸들 값을 그대로 반환하는지 확인한다.
TEST_F(UniqueHandleTest, Get_ReturnsHandleValue)
{
    TestHandle h(99);
    EXPECT_EQ(h.Get(), 99);
}

// 기본 생성된 핸들의 Get()이 Empty() 값을 반환하는지 확인한다.
TEST_F(UniqueHandleTest, Get_DefaultHandle_ReturnsEmpty)
{
    TestHandle h;
    EXPECT_EQ(h.Get(), TestTraits::Empty());
}

//***************************************************************************
// Release
//***************************************************************************

// Release()가 핸들 값을 반환하고 Close 호출 없이 소유권을 포기하는지 확인한다.
TEST_F(UniqueHandleTest, Release_ReturnsHandleAndBecomesInvalid)
{
    TestHandle h(42);
    auto raw = h.Release();

    EXPECT_EQ(raw, 42);
    EXPECT_FALSE(static_cast<bool>(h));
    EXPECT_EQ(TestTraits::close_count, 0); // Close 호출 없음
}

// 무효한 핸들에서 Release()가 Empty() 값을 반환하고 Close를 호출하지 않는지 확인한다.
TEST_F(UniqueHandleTest, Release_InvalidHandle_ReturnsEmpty)
{
    TestHandle h;
    auto raw = h.Release();

    EXPECT_EQ(raw, TestTraits::Empty());
    EXPECT_EQ(TestTraits::close_count, 0);
}

//***************************************************************************
// Reset
//***************************************************************************

// 인자 없는 Reset()이 유효한 핸들을 닫고 무효화하는지 확인한다.
TEST_F(UniqueHandleTest, Reset_NoArg_ClosesValidHandle)
{
    TestHandle h(42);
    h.Reset();

    EXPECT_FALSE(static_cast<bool>(h));
    EXPECT_EQ(TestTraits::close_count, 1);
}

// 무효한 핸들에 Reset()을 호출해도 Close가 호출되지 않는지 확인한다.
TEST_F(UniqueHandleTest, Reset_NoArg_InvalidHandle_DoesNotClose)
{
    TestHandle h;
    h.Reset();

    EXPECT_EQ(TestTraits::close_count, 0);
}

// 같은 값으로 Reset()하면 Close 호출 없이 아무 동작도 하지 않는지 확인한다.
TEST_F(UniqueHandleTest, Reset_SameHandle_IsNoOp)
{
    TestHandle h(42);
    h.Reset(42);

    EXPECT_EQ(h.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

// 다른 값으로 Reset()하면 기존 핸들을 닫고 새 값으로 교체되는지 확인한다.
TEST_F(UniqueHandleTest, Reset_DifferentHandle_ClosesOldAndSetsNew)
{
    TestHandle h(42);
    h.Reset(99);

    EXPECT_EQ(h.Get(), 99);
    EXPECT_EQ(TestTraits::close_count, 1);
}

//***************************************************************************
// Swap
//***************************************************************************

// Swap()이 두 핸들 값을 서로 교환하고 Close를 호출하지 않는지 확인한다.
TEST_F(UniqueHandleTest, Swap_ExchangesHandles)
{
    TestHandle a(42);
    TestHandle b(99);
    a.Swap(b);

    EXPECT_EQ(a.Get(), 99);
    EXPECT_EQ(b.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

// 무효한 핸들과 Swap()해도 값이 올바르게 이동하는지 확인한다.
TEST_F(UniqueHandleTest, Swap_WithInvalid_TransfersHandle)
{
    TestHandle a(42);
    TestHandle b;
    a.Swap(b);

    EXPECT_FALSE(static_cast<bool>(a));
    EXPECT_EQ(b.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

//***************************************************************************
// 소멸자
//***************************************************************************

// 유효한 핸들이 스코프를 벗어나면 소멸자가 Close를 호출하는지 확인한다.
TEST_F(UniqueHandleTest, Destructor_ValidHandle_CallsClose)
{
    {
        TestHandle h(42);
    }

    EXPECT_EQ(TestTraits::close_count, 1);
}

// 무효한 핸들이 스코프를 벗어나도 소멸자가 Close를 호출하지 않는지 확인한다.
TEST_F(UniqueHandleTest, Destructor_InvalidHandle_DoesNotCallClose)
{
    {
        TestHandle h;
    }

    EXPECT_EQ(TestTraits::close_count, 0);
}
