#pragma once
#include <Windows.h>

#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <system_error>

namespace mwl
{

const std::error_category& hresult_category() noexcept;

/// <summary>
/// Error 클래스는 에러 상태를 나타내는 객체입니다. OK 상태인 경우 내부 포인터가 nullptr이며, 그렇지 않은 경우
/// std::error_code, 메시지, 소스 위치를 저장합니다.
/// </summary>
class Error
{
public:

    /// <summary>
    /// 기본 생성자는 OK 상태의 Error 객체를 생성합니다.
    /// </summary>
    Error() noexcept;

    /// <summary>
    /// 주어진 std::error_code, 메시지, 소스 위치로 Error 객체를 생성합니다.
    /// </summary>
    /// <param name="errorCode">에러 코드</param>
    /// <param name="message">에러 메시지</param>
    /// <param name="location">소스 위치</param>
    Error(std::error_code errorCode, std::string_view message, std::source_location location);

    Error(const Error& other);
    Error(Error&&) noexcept = default;
    Error& operator=(const Error& other);
    Error& operator=(Error&&) noexcept = default;

    /// <summary>
    /// Error 객체가 OK 상태인지 여부를 반환합니다.
    /// </summary>
    /// <returns>OK 상태이면 true, 그렇지 않으면 false</returns>
    bool ok() const noexcept;
    explicit operator bool() const noexcept;

    /// <summary>
    /// Error 객체에 저장된 std::error_code를 반환합니다. OK 상태인 경우 기본값을 반환합니다.
    /// </summary>
    /// <returns>저장된 std::error_code, OK 상태인 경우 기본값</returns>
    std::error_code code() const noexcept;

    /// <summary>
    /// Error 객체에 저장된 메시지를 반환합니다. OK 상태인 경우 빈 문자열을 반환합니다.
    /// </summary>
    /// <returns></returns>
    std::string_view message() const noexcept;

    /// <summary>
    /// Error 객체에 저장된 소스 위치를 반환합니다. OK 상태인 경우 기본값을 반환합니다.
    /// </summary>
    /// <returns></returns>
    const std::source_location& location() const noexcept;

    /// <summary>
    /// Error 객체를 문자열로 변환합니다. (예: [file:line] category:value - message)
    /// </summary>
    /// <returns></returns>
    std::string ToString() const;

private:

    struct ErrorPayload
    {
        std::error_code errorCode;
        std::string message;
        std::source_location location;
    };

    std::unique_ptr<ErrorPayload> payload_;
};

// ---- 팩토리 — 상위 레이어 (std::errc 기반) ----

Error NoError() noexcept;

Error InvalidArgumentError(std::string_view msg, std::source_location loc = std::source_location::current());
Error NotFoundError(std::string_view msg, std::source_location loc = std::source_location::current());
Error PermissionDeniedError(std::string_view msg, std::source_location loc = std::source_location::current());
Error AlreadyExistsError(std::string_view msg, std::source_location loc = std::source_location::current());
Error ResourceExhaustedError(std::string_view msg, std::source_location loc = std::source_location::current());
Error TimedOutError(std::string_view msg, std::source_location loc = std::source_location::current());
Error NotSupportedError(std::string_view msg, std::source_location loc = std::source_location::current());

// ---- 팩토리 — 하위 레이어 (WinAPI 기반) ----

/// <summary>
/// 마지막 Windows 오류를 기반으로 Error 객체를 생성합니다.
/// </summary>
/// <param name="msg">에러 메시지</param>
/// <param name="loc">소스 위치</param>
/// <returns>생성된 Error 객체</returns>
Error LastWindowsError(std::string_view msg, std::source_location loc = std::source_location::current());

/// <summary>
/// 마지막 WSAGetLastError() 오류를 기반으로 Error 객체를 생성합니다.
/// </summary>
/// <param name="msg"></param>
/// <param name="loc"></param>
/// <returns></returns>
Error LastWsaError(std::string_view msg, std::source_location loc = std::source_location::current());
Error LstatusError(std::string_view msg, LSTATUS status, std::source_location loc = std::source_location::current());
Error HresultError(std::string_view msg, HRESULT hr, std::source_location loc = std::source_location::current());

} // namespace mwl
