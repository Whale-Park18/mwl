#include <gtest/gtest.h>

#include <mwl/windows/handle/handle_view.h>
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
using TestView = mwl::windows::handle::HandleView<TestTraits>;

} // namespace

class HandleViewTest : public ::testing::Test
{
protected:

    void SetUp() override
    {
        TestTraits::close_count = 0;
    }
};

// --- 생성 ---

TEST_F(HandleViewTest, DefaultConstruction_IsInvalid)
{
    TestView v;

    EXPECT_FALSE(static_cast<bool>(v));
    EXPECT_EQ(v.Get(), TestTraits::Empty());
}

TEST_F(HandleViewTest, ConstructFromValidUniqueHandle_IsValid)
{
    TestHandle h(42);
    TestView v(h);

    EXPECT_TRUE(static_cast<bool>(v));
    EXPECT_EQ(v.Get(), 42);
}

TEST_F(HandleViewTest, ConstructFromInvalidUniqueHandle_IsInvalid)
{
    TestHandle h;
    TestView v(h);

    EXPECT_FALSE(static_cast<bool>(v));
}

// --- 복사 ---

TEST_F(HandleViewTest, CopyConstruction_SharesSameHandleValue)
{
    TestHandle h(42);
    TestView v1(h);
    TestView v2(v1);

    EXPECT_EQ(v2.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

TEST_F(HandleViewTest, CopyAssignment_SharesSameHandleValue)
{
    TestHandle h(42);
    TestView v1(h);
    TestView v2;
    v2 = v1;

    EXPECT_EQ(v2.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

// --- 이동 ---

TEST_F(HandleViewTest, MoveConstruction_TransfersHandleValue)
{
    TestHandle h(42);
    TestView v1(h);
    TestView v2(std::move(v1));

    EXPECT_EQ(v2.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

TEST_F(HandleViewTest, MoveAssignment_TransfersHandleValue)
{
    TestHandle h(42);
    TestView v1(h);
    TestView v2;
    v2 = std::move(v1);

    EXPECT_EQ(v2.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

// --- operator bool / operator Handle ---

TEST_F(HandleViewTest, OperatorBool_ValidHandle_True)
{
    TestHandle h(1);
    TestView v(h);

    EXPECT_TRUE(static_cast<bool>(v));
}

TEST_F(HandleViewTest, OperatorBool_InvalidHandle_False)
{
    TestView v;

    EXPECT_FALSE(static_cast<bool>(v));
}

TEST_F(HandleViewTest, OperatorHandle_ReturnsHandleValue)
{
    TestHandle h(42);
    TestView v(h);

    EXPECT_EQ(static_cast<TestTraits::Type>(v), 42);
}

// --- Get ---

TEST_F(HandleViewTest, Get_ReturnsHandleValue)
{
    TestHandle h(99);
    TestView v(h);

    EXPECT_EQ(v.Get(), 99);
}

TEST_F(HandleViewTest, Get_DefaultView_ReturnsEmpty)
{
    TestView v;

    EXPECT_EQ(v.Get(), TestTraits::Empty());
}

// --- 소멸자 (비소유 확인) ---

TEST_F(HandleViewTest, Destructor_DoesNotCloseHandle)
{
    TestHandle h(42);

    {
        TestView v(h);
    }

    EXPECT_EQ(TestTraits::close_count, 0);
    EXPECT_TRUE(static_cast<bool>(h)); // UniqueHandle은 여전히 유효해야 함
}

TEST_F(HandleViewTest, MultipleViews_DestructionDoesNotCloseHandle)
{
    TestHandle h(42);

    {
        TestView v1(h);
        TestView v2(v1);
        TestView v3(h);
    }

    EXPECT_EQ(TestTraits::close_count, 0);
}

// --- UniqueHandle과의 독립성 ---

TEST_F(HandleViewTest, ViewDoesNotAffectUniqueHandleLifetime)
{
    TestView v;

    {
        TestHandle h(42);
        v = TestView(h);
        EXPECT_EQ(v.Get(), 42);
    }

    // UniqueHandle 소멸 시 Close 호출 확인
    EXPECT_EQ(TestTraits::close_count, 1);
    // View는 여전히 같은 값을 들고 있음 (dangling이지만 값 자체는 유지)
    EXPECT_EQ(v.Get(), 42);
}
