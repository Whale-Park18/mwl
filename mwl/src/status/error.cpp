#include <format>

#include "mwl/status/error.h"

#pragma comment(lib, "Ws2_32.lib")

namespace mwl
{

// ---- HresultCategory ----

class HresultCategory : public std::error_category
{
public:

    const char* name() const noexcept override
    {
        return "hresult";
    }

    std::string message(int ev) const override
    {
        char* buf = nullptr;
        DWORD len = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                         FORMAT_MESSAGE_IGNORE_INSERTS,
                                     nullptr, static_cast<DWORD>(ev), 0, reinterpret_cast<char*>(&buf), 0, nullptr);

        if (len == 0 || buf == nullptr)
            return "unknown HRESULT error";

        std::string result(buf, len);
        ::LocalFree(buf);
        return result;
    }
};

const std::error_category& hresult_category() noexcept
{
    static const HresultCategory kInstance;
    return kInstance;
}

// ---- Error ----

Error::Error() noexcept = default;

Error::Error(const Error& other)
{
    if (other.payload_)
        payload_ = std::make_unique<ErrorPayload>(*other.payload_);
}

Error& Error::operator=(const Error& other)
{
    if (this != &other)
    {
        if (other.payload_)
            payload_ = std::make_unique<ErrorPayload>(*other.payload_);
        else
            payload_.reset();
    }
    return *this;
}

Error::Error(std::error_code errorCode, std::string_view message, std::source_location location)
{
    if (errorCode)
        payload_ = std::make_unique<ErrorPayload>(errorCode, std::string(message), location);
}

bool Error::ok() const noexcept
{
    return payload_ == nullptr;
}

Error::operator bool() const noexcept
{
    return ok();
}

std::error_code Error::code() const noexcept
{
    if (ok())
        return {};
    return payload_->errorCode;
}

std::string_view Error::message() const noexcept
{
    if (ok())
        return {};
    return payload_->message;
}

const std::source_location& Error::location() const noexcept
{
    static const std::source_location kDefault{};
    if (ok())
        return kDefault;
    return payload_->location;
}

std::string Error::ToString() const
{
    if (ok())
        return "OK";

    return std::format("[{}:{}] {}:{} - {}", payload_->location.file_name(), payload_->location.line(),
                       payload_->errorCode.category().name(), payload_->errorCode.value(), payload_->message);
}

// ---- 팩토리 — 상위 레이어 ----

Error NoError() noexcept
{
    return {};
}

Error InvalidArgumentError(std::string_view msg, std::source_location loc)
{
    return Error(std::make_error_code(std::errc::invalid_argument), msg, loc);
}

Error NotFoundError(std::string_view msg, std::source_location loc)
{
    return Error(std::make_error_code(std::errc::no_such_file_or_directory), msg, loc);
}

Error PermissionDeniedError(std::string_view msg, std::source_location loc)
{
    return Error(std::make_error_code(std::errc::permission_denied), msg, loc);
}

Error AlreadyExistsError(std::string_view msg, std::source_location loc)
{
    return Error(std::make_error_code(std::errc::file_exists), msg, loc);
}

Error ResourceExhaustedError(std::string_view msg, std::source_location loc)
{
    return Error(std::make_error_code(std::errc::not_enough_memory), msg, loc);
}

Error TimedOutError(std::string_view msg, std::source_location loc)
{
    return Error(std::make_error_code(std::errc::timed_out), msg, loc);
}

Error NotSupportedError(std::string_view msg, std::source_location loc)
{
    return Error(std::make_error_code(std::errc::not_supported), msg, loc);
}

// ---- 팩토리 — 하위 레이어 ----

Error LastWindowsError(std::string_view msg, std::source_location loc)
{
    return Error(std::error_code(static_cast<int>(::GetLastError()), std::system_category()), msg, loc);
}

Error LastWsaError(std::string_view msg, std::source_location loc)
{
    return Error(std::error_code(::WSAGetLastError(), std::system_category()), msg, loc);
}

Error LstatusError(std::string_view msg, LSTATUS status, std::source_location loc)
{
    return Error(std::error_code(static_cast<int>(status), std::system_category()), msg, loc);
}

Error HresultError(std::string_view msg, HRESULT hr, std::source_location loc)
{
    if (SUCCEEDED(hr))
        return NoError();

    std::error_code errorCode;
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
        errorCode = std::error_code(HRESULT_CODE(hr), std::system_category());
    else
        errorCode = std::error_code(static_cast<int>(hr), hresult_category());

    return Error(errorCode, msg, loc);
}

} // namespace mwl
