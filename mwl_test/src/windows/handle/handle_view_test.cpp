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

//***************************************************************************
// 생성
//***************************************************************************

// 기본 생성된 view는 유효하지 않은 상태인지 확인한다.
TEST_F(HandleViewTest, DefaultConstruction_IsInvalid)
{
    TestView v;

    EXPECT_FALSE(static_cast<bool>(v));
    EXPECT_EQ(v.Get(), TestTraits::Empty());
}

// 유효한 값으로 생성하면 유효한 상태가 되는지 확인한다.
TEST_F(HandleViewTest, ConstructFromValidHandle_IsValid)
{
    TestView v(42);

    EXPECT_TRUE(static_cast<bool>(v));
    EXPECT_EQ(v.Get(), 42);
}

// 유효한 UniqueHandle로부터 생성하면 같은 값을 가리키는 유효한 view가 되는지 확인한다.
TEST_F(HandleViewTest, ConstructFromValidUniqueHandle_IsValid)
{
    TestHandle h(42);
    TestView v(h);

    EXPECT_TRUE(static_cast<bool>(v));
    EXPECT_EQ(v.Get(), 42);
}

// 무효한 UniqueHandle로부터 생성하면 무효한 view가 되는지 확인한다.
TEST_F(HandleViewTest, ConstructFromInvalidUniqueHandle_IsInvalid)
{
    TestHandle h;
    TestView v(h);

    EXPECT_FALSE(static_cast<bool>(v));
}

//***************************************************************************
// 복사
//***************************************************************************

// 복사 생성된 view가 같은 핸들 값을 공유하는지 확인한다.
TEST_F(HandleViewTest, CopyConstruction_SharesSameHandleValue)
{
    TestHandle h(42);
    TestView v1(h);
    TestView v2(v1);

    EXPECT_EQ(v2.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

// 복사 대입된 view가 같은 핸들 값을 공유하는지 확인한다.
TEST_F(HandleViewTest, CopyAssignment_SharesSameHandleValue)
{
    TestHandle h(42);
    TestView v1(h);
    TestView v2;
    v2 = v1;

    EXPECT_EQ(v2.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

//***************************************************************************
// 이동
//***************************************************************************

// 이동 생성된 view가 핸들 값을 그대로 전달받는지 확인한다.
TEST_F(HandleViewTest, MoveConstruction_TransfersHandleValue)
{
    TestHandle h(42);
    TestView v1(h);
    TestView v2(std::move(v1));

    EXPECT_EQ(v2.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

// 이동 대입된 view가 핸들 값을 그대로 전달받는지 확인한다.
TEST_F(HandleViewTest, MoveAssignment_TransfersHandleValue)
{
    TestHandle h(42);
    TestView v1(h);
    TestView v2;
    v2 = std::move(v1);

    EXPECT_EQ(v2.Get(), 42);
    EXPECT_EQ(TestTraits::close_count, 0);
}

//***************************************************************************
// operator bool / operator Handle
//***************************************************************************

// 유효한 핸들을 가리키는 view에서 operator bool이 true를 반환하는지 확인한다.
TEST_F(HandleViewTest, OperatorBool_ValidHandle_True)
{
    TestHandle h(1);
    TestView v(h);

    EXPECT_TRUE(static_cast<bool>(v));
}

// 무효한 view에서 operator bool이 false를 반환하는지 확인한다.
TEST_F(HandleViewTest, OperatorBool_InvalidHandle_False)
{
    TestView v;

    EXPECT_FALSE(static_cast<bool>(v));
}

// operator Handle이 내부 핸들 값을 그대로 반환하는지 확인한다.
TEST_F(HandleViewTest, OperatorHandle_ReturnsHandleValue)
{
    TestHandle h(42);
    TestView v(h);

    EXPECT_EQ(static_cast<TestTraits::Type>(v), 42);
}

//***************************************************************************
// Get
//***************************************************************************

// Get()이 내부 핸들 값을 그대로 반환하는지 확인한다.
TEST_F(HandleViewTest, Get_ReturnsHandleValue)
{
    TestHandle h(99);
    TestView v(h);

    EXPECT_EQ(v.Get(), 99);
}

// 기본 생성된 view의 Get()이 Empty() 값을 반환하는지 확인한다.
TEST_F(HandleViewTest, Get_DefaultView_ReturnsEmpty)
{
    TestView v;

    EXPECT_EQ(v.Get(), TestTraits::Empty());
}

//***************************************************************************
// 소멸자 (비소유 확인)
//***************************************************************************

// view가 소멸돼도 원본 핸들이 닫히지 않는지 확인한다.
TEST_F(HandleViewTest, Destructor_DoesNotCloseHandle)
{
    TestHandle h(42);

    {
        TestView v(h);
    }

    EXPECT_EQ(TestTraits::close_count, 0);
    EXPECT_TRUE(static_cast<bool>(h)); // UniqueHandle은 여전히 유효해야 함
}

// 여러 view가 동시에 소멸돼도 원본 핸들이 닫히지 않는지 확인한다.
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

//***************************************************************************
// UniqueHandle과의 독립성
//***************************************************************************

// view는 UniqueHandle의 수명에 관여하지 않으며, UniqueHandle 소멸 후에도 이전 값을 그대로 들고 있는지 확인한다.
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
