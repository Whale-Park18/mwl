#pragma once
#include <cassert>
#include <variant>

#include "mwl/status/error.h"

namespace mwl
{

/// <summary>
/// 값(T) 또는 오류(Error)를 보관하는 결과 타입. 성공 시 T 값을, 실패 시 Error를 저장한다.
/// </summary>
/// <typeparam name="T">성공일 때 저장되는 값의 타입.</typeparam>
template<class T>
class Result
{
public:

    /// <summary>
    /// 성공 상태를 가진 Result 객체를 생성합니다. 전달된 값을 내부로 이동합니다.
    /// </summary>
    /// <param name="value">이동될 값. 내부 data_로 이동되어 Result가 이 값을 보유합니다.</param>
    Result(T value) : data_(std::move(value))
    {
    }

    /// <summary>
    /// 오류 상태를 가진 Result 객체를 생성합니다. 전달된 Error를 내부로 이동하고, 이동된 오류가 정상 상태(ok)가 아님을
    /// 확인합니다.
    /// </summary>
    /// <param name="error">이동될 Error 객체. 내부 data_로 이동되어 Result가 이 오류를 보유합니다.</param>
    Result(Error error) : data_(std::move(error))
    {
        assert(!std::get<Error>(data_).ok());
    }

    /// <summary>
    /// 내부 std::variant가 현재 T 타입을 활성 대체자로 가지는지 검사한다(예외를 던지지 않음).
    /// </summary>
    /// <returns>활성 대체자가 T이면 true, 아니면 false.</returns>
    bool ok() const noexcept
    {
        return std::holds_alternative<T>(data_);
    }

    explicit operator bool() const noexcept
    {
        return ok();
    }

    const T& value() const&
    {
        assert(ok());
        return *std::get_if<T>(&data_);
    }

    T& value() &
    {
        assert(ok());
        return *std::get_if<T>(&data_);
    }

    /// <summary>
    /// 이동 시 value()를 호출하면 내부의 T 값을 이동하여 반환합니다. 호출 후 Result 객체는 더 이상 유효하지 않습니다.
    /// </summary>
    /// <returns>이동된 T 값</returns>
    T value() &&
    {
        assert(ok());
        return std::get<T>(std::move(data_));
    }

    const Error& error() const noexcept
    {
        assert(!ok());
        return *std::get_if<Error>(&data_);
    }

private:

    std::variant<T, Error> data_;
};

} // namespace mwl
