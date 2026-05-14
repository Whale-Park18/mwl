#pragma once

extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* lpOutputString);
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringW(const wchar_t* lpOutputString);

namespace mwl::internal
{

inline void Log(const char* msg) noexcept
{
    ::OutputDebugStringA(msg);
}

inline void Log(const wchar_t* msg) noexcept
{
    ::OutputDebugStringW(msg);
}

} // namespace mwl::internal
