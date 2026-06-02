#pragma once

#include "unique_handle.h"

namespace mwl::windows::handle
{

/// <summary>
/// 핸들을 소유하지 않고 단순히 참조만 하는 뷰(View)를 나타내는 템플릿 클래스입니다. 핸들의 소유권을 가지지 않으므로
/// 핸들을 해제하지 않습니다.
/// </summary>
/// <typeparam name="Traits">
/// 핸들 타입과 핸들 해제(파괴) 동작을 정의하는 정책 타입(예: 무효 값 표현과 해제 함수 제공).
/// <example>
/// <code>
/// struct ExampleTraits
/// {
///     using Type = SomeHandleType; // 핸들 타입 정의
///
///     static bool IsValid(Type handle) noexcept
///     {
///         // 핸들이 유효한지 검사하는 로직 구현
///     }
///
///     static Type Empty() noexcept
///     {
///         // 핸들의 무효 상태를 나타내는 값을 반환하는 로직 구현
///     }
///
///     static void Close(Type handle) noexcept
///     {
///         // 핸들을 해제하는 로직 구현 (예: CloseHandle(handle) 호출)
///     }
/// }
/// </code>
/// </example>
/// </typeparam>
template<class Traits>
class HandleView
{
public:

    using Handle = Traits::Type;

    HandleView() noexcept = default;

    explicit HandleView(const UniqueHandle<Traits>& handle) noexcept : handle_(handle.Get())
    {
    }

    HandleView(const HandleView& other) noexcept = default;
    HandleView& operator=(const HandleView& other) noexcept = default;
    HandleView(HandleView&& other) noexcept = default;
    HandleView& operator=(HandleView&& other) noexcept = default;
    ~HandleView() = default;

    explicit operator bool() const noexcept
    {
        return Traits::IsValid(handle_);
    }

    explicit operator Handle() const noexcept
    {
        return handle_;
    }

    Handle Get() const noexcept
    {
        return handle_;
    }

private:

    Handle handle_{ Traits::Empty() };
};

} // namespace mwl::windows::handle
