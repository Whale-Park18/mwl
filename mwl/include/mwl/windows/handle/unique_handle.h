#pragma once

#include <utility>

namespace mwl::windows::handle
{

/// <summary>
/// 핸들을 소유하고 소유권의 이동만 허용하는 고유(Unique) 핸들을 나타내는 템플릿 클래스입니다. 복사는 금지되고 이동
/// 생성자/이동 대입 연산자는 기본 제공됩니다.
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
class UniqueHandle
{
public:

    using Handle = typename Traits::Type;

    UniqueHandle() noexcept = default;

    explicit UniqueHandle(Handle handle) noexcept : handle_(handle)
    {
    }

    UniqueHandle(UniqueHandle&& other) noexcept : handle_(other.handle_)
    {
        other.handle_ = Traits::Empty();
    }

    UniqueHandle& operator=(UniqueHandle&& other) noexcept
    {
        if (handle_ != other.handle_)
        {
            Reset(other.handle_);
            other.handle_ = Traits::Empty();
        }
        return *this;
    }

    ~UniqueHandle()
    {
        if (Traits::IsValid(handle_))
        {
            Traits::Close(handle_);
        }
    }

    UniqueHandle(const UniqueHandle& other) = delete;
    UniqueHandle& operator=(const UniqueHandle& other) = delete;

    explicit operator bool() const noexcept
    {
        return Traits::IsValid(handle_);
    }

    explicit operator Handle() const noexcept
    {
        return handle_;
    }

    /// <summary>
    /// 소유 중인 핸들을 반환하고, 소유권을 포기하여 핸들을 무효 상태로 만드는 멤버 함수. 반환된 핸들은 호출자가
    /// 관리해야 하며, 이후 UniqueHandle 객체는 해당 핸들을 더 이상 소유하지 않게 된다.
    /// </summary>
    /// <returns>소유권을 이전받은 핸들</returns>
    Handle Release() noexcept
    {
        Handle temp = handle_;
        handle_ = Traits::Empty();

        return temp;
    }

    /// <summary>
    /// 내부 핸들을 주어진 값으로 재설정한다. 전달된 핸들이 현재와 같으면 아무 작업도 하지 않으며, 이전 핸들이 유효하면
    /// Traits::Close로 닫는다. noexcept로 예외를 던지지 않는다.
    /// </summary>
    /// <param name="handle">재설정할 핸들. 기본값은 Traits::Empty().</param>
    void Reset(Handle handle = Traits::Empty()) noexcept
    {
        if (handle_ == handle)
        {
            return;
        }

        if (Traits::IsValid(handle_))
        {
            Traits::Close(handle_);
        }

        handle_ = handle;
    }

    /// <summary>
    /// 현재 객체와 다른 UniqueHandle 인스턴스의 내부 핸들을 교환합니다. 예외를 발생시키지 않습니다.
    /// </summary>
    /// <param name="other">교환 대상인 다른 UniqueHandle의 참조.</param>
    void Swap(UniqueHandle& other) noexcept
    {
        std::swap(handle_, other.handle_);
    }

    /// <summary>
    /// 멤버 변수 handle_의 값을 반환하는 const 멤버 함수입니다. noexcept로 예외를 발생시키지 않습니다.
    /// </summary>
    /// <returns>Handle 타입으로서 handle_의 값.</returns>
    Handle Get() const noexcept
    {
        return handle_;
    }

private:

    Handle handle_{ Traits::Empty() };
};

} // namespace mwl::windows::handle
