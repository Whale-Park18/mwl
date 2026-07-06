#pragma once

#include <Windows.h>

#include <string>
#include <string_view>
#include <vector>

#include "..\..\status\result.h"
#include "..\handle\handle_view.h"
#include "..\handle\registry_handle.h"
#include "value.h"

namespace mwl::windows::registry
{

/// <summary>
/// 부모 키 아래에 서브 키를 생성합니다. 이미 존재하면 해당 키를 엽니다(RegCreateKeyExW).
/// key에는 HKEY 상수나 UniqueRegistryHandle을 그대로 전달할 수 있습니다.
/// </summary>
/// <param name="key">부모 키 핸들.</param>
/// <param name="subkey">생성하거나 열 서브 키의 상대 경로.</param>
/// <param name="options">키 옵션. 기본값은 비휘발성 키(REG_OPTION_NON_VOLATILE).</param>
/// <param name="desired">키에 대한 액세스 권한. 기본값은 KEY_ALL_ACCESS.</param>
/// <returns>성공 시 생성/열린 키의 UniqueRegistryHandle, 실패 시 Error.</returns>
[[nodiscard]] Result<handle::UniqueRegistryHandle> CreateKey(handle::RegistryHandleView key, std::wstring_view subkey,
                                                             DWORD options = REG_OPTION_NON_VOLATILE,
                                                             DWORD desired = KEY_ALL_ACCESS);

/// <summary>
/// 부모 키 아래의 기존 서브 키를 엽니다(RegOpenKeyExW). 존재하지 않으면 실패합니다.
/// </summary>
/// <param name="key">부모 키 핸들.</param>
/// <param name="subkey">열 서브 키의 상대 경로.</param>
/// <param name="desired">키에 대한 액세스 권한(예: KEY_READ).</param>
/// <returns>성공 시 열린 키의 UniqueRegistryHandle, 실패 시 Error.</returns>
[[nodiscard]] Result<handle::UniqueRegistryHandle> OpenKey(handle::RegistryHandleView key, std::wstring_view subkey,
                                                           DWORD desired);

/// <summary>
/// 부모 키 아래의 서브 키를 삭제합니다(RegDeleteKeyW). 하위 서브 키가 없는 키만 삭제할 수 있습니다.
/// </summary>
/// <param name="key">부모 키 핸들.</param>
/// <param name="subkey">삭제할 서브 키의 상대 경로.</param>
/// <returns>성공 시 값 없는 Result, 실패 시 Error.</returns>
[[nodiscard]] Result<void> DeleteKey(handle::RegistryHandleView key, std::wstring_view subkey);

/// <summary>
/// 지정한 키에서 REG_DWORD 값을 읽습니다(RegGetValueW).
/// </summary>
/// <param name="key">기준 키 핸들.</param>
/// <param name="subKey">key 아래의 추가 서브 키 경로. 빈 문자열이면 key 자체에서 읽습니다.</param>
/// <param name="valueName">읽을 값의 이름.</param>
/// <returns>성공 시 읽은 DWORD 값, 실패 시 Error.</returns>
[[nodiscard]] Result<DWORD> ReadDword(handle::RegistryHandleView key, std::wstring_view subKey,
                                      std::wstring_view valueName);

/// <summary>
/// 지정한 키에서 REG_QWORD 값을 읽습니다(RegGetValueW).
/// </summary>
/// <param name="key">기준 키 핸들.</param>
/// <param name="subKey">key 아래의 추가 서브 키 경로. 빈 문자열이면 key 자체에서 읽습니다.</param>
/// <param name="valueName">읽을 값의 이름.</param>
/// <returns>성공 시 읽은 DWORD64 값, 실패 시 Error.</returns>
[[nodiscard]] Result<DWORD64> ReadQword(handle::RegistryHandleView key, std::wstring_view subKey,
                                        std::wstring_view valueName);

/// <summary>
/// 지정한 키에서 REG_SZ 문자열 값을 읽습니다(RegGetValueW).
/// </summary>
/// <param name="key">기준 키 핸들.</param>
/// <param name="subKey">key 아래의 추가 서브 키 경로. 빈 문자열이면 key 자체에서 읽습니다.</param>
/// <param name="valueName">읽을 값의 이름.</param>
/// <returns>성공 시 읽은 문자열(std::wstring), 실패 시 Error.</returns>
[[nodiscard]] Result<std::wstring> ReadString(handle::RegistryHandleView key, std::wstring_view subKey,
                                              std::wstring_view valueName);

/// <summary>
/// 지정한 키에 REG_DWORD 값을 씁니다(RegSetKeyValueW). 값이 없으면 새로 만들고, 있으면 덮어씁니다.
/// </summary>
/// <param name="key">기준 키 핸들.</param>
/// <param name="subKey">key 아래의 추가 서브 키 경로. 빈 문자열이면 key 자체에 씁니다.</param>
/// <param name="valueName">쓸 값의 이름.</param>
/// <param name="data">저장할 DWORD 값.</param>
/// <returns>성공 시 값 없는 Result, 실패 시 Error.</returns>
[[nodiscard]] Result<void> WriteDword(handle::RegistryHandleView key, std::wstring_view subKey,
                                      std::wstring_view valueName, DWORD data);

/// <summary>
/// 지정한 키에 REG_QWORD 값을 씁니다(RegSetKeyValueW). 값이 없으면 새로 만들고, 있으면 덮어씁니다.
/// </summary>
/// <param name="key">기준 키 핸들.</param>
/// <param name="subKey">key 아래의 추가 서브 키 경로. 빈 문자열이면 key 자체에 씁니다.</param>
/// <param name="valueName">쓸 값의 이름.</param>
/// <param name="data">저장할 DWORD64 값.</param>
/// <returns>성공 시 값 없는 Result, 실패 시 Error.</returns>
[[nodiscard]] Result<void> WriteQword(handle::RegistryHandleView key, std::wstring_view subKey,
                                      std::wstring_view valueName, DWORD64 data);

/// <summary>
/// 지정한 키에 REG_SZ 문자열 값을 씁니다(RegSetKeyValueW). 값이 없으면 새로 만들고, 있으면 덮어씁니다.
/// </summary>
/// <param name="key">기준 키 핸들.</param>
/// <param name="subKey">key 아래의 추가 서브 키 경로. 빈 문자열이면 key 자체에 씁니다.</param>
/// <param name="valueName">쓸 값의 이름.</param>
/// <param name="data">저장할 문자열. 널 종료 문자를 포함해 기록됩니다.</param>
/// <returns>성공 시 값 없는 Result, 실패 시 Error.</returns>
[[nodiscard]] Result<void> WriteString(handle::RegistryHandleView key, std::wstring_view subKey,
                                       std::wstring_view valueName, std::wstring_view data);

/// <summary>
/// 지정한 키 바로 아래의 값을 삭제합니다(RegDeleteValueW).
/// </summary>
/// <param name="key">값이 속한 키 핸들.</param>
/// <param name="valueName">삭제할 값의 이름.</param>
/// <returns>성공 시 값 없는 Result, 실패 시 Error.</returns>
[[nodiscard]] Result<void> DeleteValue(handle::RegistryHandleView key, std::wstring_view valueName);

#define TEST_ENUM_CALLBACKS 1
#if TEST_ENUM_CALLBACKS

// 키 나열
[[nodiscard]] Result<std::vector<std::wstring>> EnumSubKeys(handle::RegistryHandleView key);

// 값 나열
[[nodiscard]] Result<std::vector<Value>> EnumValues(handle::RegistryHandleView key, std::wstring_view subKey);

#endif

} // namespace mwl::windows::registry
