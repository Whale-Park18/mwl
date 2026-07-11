#include <Windows.h>

#include <algorithm>
#include <array>
#include <format>
#include <span>
#include <vector>

#include "mwl/status/error.h"
#include "mwl/status/result.h"
#include "mwl/windows/definition.h"
#include "mwl/windows/registry/registry.h"

#ifdef _DEBUG

#include <format>

#include "mwl/internal/log.h"

#endif // _DEBUG

#pragma comment(lib, "Advapi32.lib")

namespace mwl::windows::registry
{

Result<handle::UniqueRegistryHandle> CreateKey(handle::RegistryHandleView key, StringView subkey, Dword options,
                                               Dword desired)
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

Result<handle::UniqueRegistryHandle> OpenKey(handle::RegistryHandleView key, StringView subkey, Dword desired)
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

Result<void> DeleteKey(handle::RegistryHandleView key, StringView subkey)
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

Result<std::wstring> ReadString(handle::RegistryHandleView key, StringView subKey, StringView valueName)
{
    Dword size{ 0 };

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

Result<Dword> ReadDword(handle::RegistryHandleView key, StringView subKey, StringView valueName)
{
    Dword value{ 0 };
    Dword size{ sizeof(Dword) };
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

Result<Qword> ReadQword(handle::RegistryHandleView key, StringView subKey, StringView valueName)
{
    Qword value{ 0 };
    Dword size{ sizeof(Qword) };
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

Result<void> WriteDword(handle::RegistryHandleView key, StringView subKey, StringView valueName, Dword data)
{
    LSTATUS status = RegSetKeyValueW(key.Get(),        // 키 핸들
                                     subKey.data(),    // 서브 키
                                     valueName.data(), // 값 이름
                                     REG_DWORD,        // 데이터 유형
                                     &data,            // 데이터 버퍼
                                     sizeof(Dword)     // 데이터 크기
    );

    if (status == ERROR_SUCCESS)
    {
        return {};
    }

    return LstatusError("Failed to write registry DWORD value", status);
}

Result<void> WriteQword(handle::RegistryHandleView key, StringView subKey, StringView valueName, Qword data)
{
    LSTATUS status = RegSetKeyValueW(key.Get(),        // 키 핸들
                                     subKey.data(),    // 서브 키
                                     valueName.data(), // 값 이름
                                     REG_QWORD,        // 데이터 유형
                                     &data,            // 데이터 버퍼
                                     sizeof(Qword)     // 데이터 크기
    );

    if (status == ERROR_SUCCESS)
    {
        return {};
    }

    return LstatusError("Failed to write registry REG_QWORD value", status);
}

Result<void> WriteString(handle::RegistryHandleView key, StringView subKey, StringView valueName, StringView data)
{
    // REG_SZ 값은 null 종료 문자를 포함해야 하므로, data에 null 종료 문자가 없으면 추가합니다.
    String buffer{ data };
    if (buffer.empty() || buffer.back() != L'\0')
    {
        buffer.push_back(L'\0'); // null 종료 문자를 추가합니다.
    }

    LSTATUS status = RegSetKeyValueW(key.Get(),                                            // 부모 키의 핸들
                                     subKey.data(),                                        // 서브 키
                                     valueName.data(),                                     // 값 이름
                                     REG_SZ,                                               // 데이터 유형
                                     buffer.data(),                                        // 데이터 버퍼
                                     static_cast<Dword>((buffer.size()) * sizeof(wchar_t)) // 데이터 크기
    );

    if (status == ERROR_SUCCESS)
    {
        return {};
    }

    return LstatusError("Failed to write registry REG_SZ value", status);
}

Result<void> DeleteValue(handle::RegistryHandleView key, StringView valueName)
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

Result<std::vector<std::wstring>> EnumSubKeys(handle::RegistryHandleView key)
{
    std::vector<std::wstring> subKeys{};
    Dword index{ 0 };

    while (true)
    {
        std::array<wchar_t, 256> buffer{}; // 키 이름은 255자까지 허용되므로, 256으로 충분히 큰 버퍼를 사용합니다.
        Dword bufferSize{ static_cast<Dword>(buffer.size()) };

        LSTATUS status = RegEnumKeyExW(key.Get(),     // 키 핸들
                                       index,         // 인덱스
                                       buffer.data(), // 서브 키 이름을 받을 버퍼
                                       &bufferSize,   // 버퍼 크기 (문자 수 단위)
                                       nullptr,       // 예약된 값
                                       nullptr,       // 클래스 문자열을 받을 버퍼 (레거시 옵션)
                                       nullptr,       // 클래스 문자열 버퍼 크기를 받을 포인터 (레거시 옵션)
                                       nullptr        // 마지막 수정 시간 정보를 받을 포인터 (옵션)
        );

        if (status == ERROR_NO_MORE_ITEMS)
        {
            break;
        }

        if (status != ERROR_SUCCESS)
        {
            return LstatusError("Failed to enumerate registry subkeys", status);
        }

        subKeys.emplace_back(buffer.data());
        ++index;
    }

    return subKeys;
}

Result<std::vector<Value>> EnumValues(handle::RegistryHandleView key)
{
    Dword maxNameLength{ 0 };
    Dword maxDataSize{ 0 };
    LSTATUS status = RegQueryInfoKeyW(key.Get(), // 키 핸들
                                      nullptr,   // 클래스 이름을 받을 버퍼 (레거시)
                                      nullptr,   // 버퍼 크기 (문자 수) / 실제 클래스 이름 길이를 받을 포인터 (레거시)
                                      nullptr,   // 예약된 값
                                      nullptr,   // 하위 키 수를 받을 포인터
                                      nullptr,   // 하위 키 이름 중 최대 길이 (문자 수, null 제외)를 받을 포인터
                                      nullptr,   // 하위 키 클래스 이름 중 최대 길이 (문자 수)를 받을 포인터 (레거시)
                                      nullptr,   // 값 항목 수를 받을 포인터
                                      &maxNameLength, // 값 이름 중 최대 길이 (문자 수, null 제외)를 받을 포인터
                                      &maxDataSize,   // 값 데이터 중 최대 크기 (바이트 수)를 받을 포인터
                                      nullptr,        // 보안 디스크립터 크기 (바이트 수)를 받을 포인터
                                      nullptr         // 마지막 수정 시각을 받을 FILETIME 포인터
    );

    if (status != ERROR_SUCCESS)
    {
        return LstatusError("Failed to query registry key info", status);
    }

    std::vector<Value> values{};

    std::vector<wchar_t> nameBuffer(maxNameLength + 1); // 값 이름 버퍼
    Binary dataBuffer(maxDataSize + 1);                 // 값 데이터 버퍼
    Dword index{ 0 };                                   // 값 인덱스

    while (true)
    {
        // vector를 매번 생성하지 않고, 기존 버퍼를 재사용하기 위해 초기화합니다.
        std::fill(nameBuffer.begin(), nameBuffer.end(), L'\0');
        std::fill(dataBuffer.begin(), dataBuffer.end(), Byte{ 0 });

        Dword type{};
        Dword nameLength{ maxNameLength + 1 }; // RegEnumValueW가 실제 기록한 크기로 줄이므로 매 반복마다 재설정
        Dword dataSize{ maxDataSize + 1 };     // RegEnumValueW가 실제 기록한 크기로 줄이므로 매 반복마다 재설정

        status = RegEnumValueW(key.Get(),         // 키 핸들
                               index,             // 인덱스
                               nameBuffer.data(), // 값 이름을 받을 버퍼
                               &nameLength,       // 값 이름 버퍼 크기
                               nullptr,           // 예약된 값
                               &type,             // 값 유형을 받을 포인터 (nullptr이면 유형을 가져오지 않음)
                               dataBuffer.data(), // 값 데이터를 받을 버퍼 (nullptr이면 데이터를 가져오지 않음)
                               &dataSize // 값 데이터 버퍼 크기를 받을 포인터 (nullptr이면 크기를 가져오지 않음)
        );

        if (status == ERROR_NO_MORE_ITEMS)
        {
            break;
        }

        if (status != ERROR_SUCCESS)
        {
            return LstatusError("Failed to enumerate registry values", status);
        }

        values.emplace_back(nameBuffer.data(), static_cast<ValueType>(type),
                            std::span<const Byte>(dataBuffer.data(), dataSize));
        ++index;
    }

    return values;
}

} // namespace mwl::windows::registry
