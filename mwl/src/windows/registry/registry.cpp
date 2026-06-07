#include <Windows.h>

#include <format>
#include <vector>

#include "mwl/status/error.h"
#include "mwl/status/result.h"
#include "mwl/windows/registry/registry.h"

#ifdef _DEBUG

#include <format>

#include "mwl/internal/log.h"

#endif // _DEBUG

#pragma comment(lib, "Advapi32.lib")

namespace mwl::windows::registry
{

Result<handle::UniqueRegistryHandle> CreateKey(handle::RegistryHandleView key, std::wstring_view subkey, DWORD options,
                                               DWORD desired)
{
    HKEY rawHandle{ nullptr };
    LSTATUS status = RegCreateKeyExW(key.Get(),     // 키 핸들
                                     subkey.data(), // 서브 키
                                     0,             // 예약된 값
                                     nullptr,       // 클래스 문자열 (옵션)
                                     options,       // 키 옵션
                                     desired,       // 키에 대한 액세스 권한
                                     nullptr,       // 보안 속성
                                     &rawHandle,    // 성공 시, 새로 생성된 키의 핸들을 받는 포인터
                                     nullptr        // 성공 시, 키 상태
    );

    if (status == ERROR_SUCCESS)
    {
        return handle::UniqueRegistryHandle{ rawHandle };
    }

    return LstatusError("Failed to create registry key", status);
}

Result<handle::UniqueRegistryHandle> OpenKey(handle::RegistryHandleView key, std::wstring_view subkey, DWORD desired)
{
    HKEY rawHandle{ nullptr };
    LSTATUS status = RegOpenKeyExW(key.Get(),     // 키 핸들
                                   subkey.data(), // 서브 키
                                   0,             // 예약된 값
                                   desired,       // 키에 대한 액세스 권한
                                   &rawHandle     // 성공 시, 열려진 키의 핸들을 받는 포인터
    );

    if (status == ERROR_SUCCESS)
    {
        return handle::UniqueRegistryHandle{ rawHandle };
    }

    return LstatusError("Failed to open registry key", status);
}

Result<void> DeleteKey(handle::RegistryHandleView key, std::wstring_view subkey)
{
    LSTATUS status = RegDeleteKeyW(key.Get(),    // 키 핸들
                                   subkey.data() // 삭제할 서브 키의 상대 경로
    );

    if (status == ERROR_SUCCESS)
    {
        return {};
    }

    return LstatusError("Failed to delete registry key", status);
}

Result<std::wstring> ReadString(handle::RegistryHandleView key, std::wstring_view subKey, std::wstring_view valueName)
{
    DWORD size{ 0 };

    // 1차 호출: 필요한 버퍼 크기 획득
    LSTATUS status = RegGetValueW(key.Get(),        // 키 핸들
                                  subKey.data(),    // 서브 키
                                  valueName.data(), // 값 이름
                                  RRF_RT_REG_SZ,    // 원하는 데이터 유형 (REG_SZ)
                                  nullptr,          // 데이터 버퍼 (첫 번째 호출에서는 nullptr)
                                  nullptr,          // 데이터 유형 반환 (옵션)
                                  &size             // 버퍼 크기 반환
    );

    if (status != ERROR_SUCCESS)
    {
        return LstatusError("Failed to read registry REG_SZ value size", status);
    }

    // 버퍼 크기는 바이트 단위이므로 wchar_t 크기로 나누고, 널 종료를 위해 +1
    std::vector<wchar_t> buffer((size / sizeof(wchar_t)) + 1);
    status = RegGetValueW(key.Get(), subKey.data(), valueName.data(), RRF_RT_REG_SZ, nullptr, buffer.data(), &size);
    if (status == ERROR_SUCCESS)
    {
        return std::wstring(buffer.data());
    }

    return LstatusError("Failed to read registry REG_SZ value", status);
}

Result<DWORD> ReadDword(handle::RegistryHandleView key, std::wstring_view subKey, std::wstring_view valueName)
{
    DWORD value{ 0 };
    DWORD size{ sizeof(DWORD) };
    LSTATUS status = RegGetValueW(key.Get(),        // 키 핸들
                                  subKey.data(),    // 서브 키
                                  valueName.data(), // 값 이름
                                  RRF_RT_REG_DWORD, // 원하는 데이터 유형 (REG_DWORD)
                                  nullptr,          // 데이터 유형 반환 (옵션)
                                  &value,           // 데이터 버퍼
                                  &size             // 버퍼 크기 반환
    );

    if (status == ERROR_SUCCESS)
    {
        return value;
    }

    return LstatusError("Failed to read registry DWORD value", status);
}

Result<DWORD64> ReadQword(handle::RegistryHandleView key, std::wstring_view subKey, std::wstring_view valueName)
{
    DWORD64 value{ 0 };
    DWORD size{ sizeof(DWORD64) };
    LSTATUS status = RegGetValueW(key.Get(),        // 키 핸들
                                  subKey.data(),    // 서브 키
                                  valueName.data(), // 값 이름
                                  RRF_RT_REG_QWORD, // 원하는 데이터 유형 (REG_QWORD)
                                  nullptr,          // 데이터 유형 반환 (옵션)
                                  &value,           // 데이터 버퍼
                                  &size             // 버퍼 크기 반환
    );

    if (status == ERROR_SUCCESS)
    {
        return value;
    }

    return LstatusError("Failed to read registry DWORD value", status);
}

Result<void> WriteDword(handle::RegistryHandleView key, std::wstring_view subKey, std::wstring_view valueName,
                        DWORD data)
{
    LSTATUS status = RegSetKeyValueW(key.Get(),        // 키 핸들
                                     subKey.data(),    // 서브 키
                                     valueName.data(), // 값 이름
                                     REG_DWORD,        // 데이터 유형
                                     &data,            // 데이터 버퍼
                                     sizeof(DWORD)     // 데이터 크기
    );

    if (status == ERROR_SUCCESS)
    {
        return {};
    }

    return LstatusError("Failed to write registry DWORD value", status);
}

Result<void> WriteQword(handle::RegistryHandleView key, std::wstring_view subKey, std::wstring_view valueName,
                        DWORD64 data)
{
    LSTATUS status = RegSetKeyValueW(key.Get(),        // 키 핸들
                                     subKey.data(),    // 서브 키
                                     valueName.data(), // 값 이름
                                     REG_QWORD,        // 데이터 유형
                                     &data,            // 데이터 버퍼
                                     sizeof(DWORD64)   // 데이터 크기
    );

    if (status == ERROR_SUCCESS)
    {
        return {};
    }

    return LstatusError("Failed to write registry REG_QWORD value", status);
}

Result<void> WriteString(handle::RegistryHandleView key, std::wstring_view subKey, std::wstring_view valueName,
                         std::wstring_view data)
{
    LSTATUS status = RegSetKeyValueW(key.Get(),                                              // 부모 키의 핸들
                                     subKey.data(),                                          // 서브 키
                                     valueName.data(),                                       // 값 이름
                                     REG_SZ,                                                 // 데이터 유형
                                     data.data(),                                            // 데이터 버퍼
                                     static_cast<DWORD>((data.size() + 1) * sizeof(wchar_t)) // 데이터 크기
    );

    if (status == ERROR_SUCCESS)
    {
        return {};
    }

    return LstatusError("Failed to write registry REG_SZ value", status);
}

Result<void> DeleteValue(handle::RegistryHandleView key, std::wstring_view valueName)
{
    LSTATUS status = RegDeleteValueW(key.Get(),       // 키 핸들
                                     valueName.data() // 값 이름
    );

    if (status == ERROR_SUCCESS)
    {
        return {};
    }

    return LstatusError("Failed to delete registry value", status);
}

} // namespace mwl::windows::registry
