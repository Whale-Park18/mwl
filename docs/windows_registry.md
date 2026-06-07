# Windows 레지스트리 API

## 개요

Windows 레지스트리 키/값을 다루는 `Reg*` WinAPI를 얇게 감싼 자유 함수(free function) 모음이다. 원시 API의 번거로운 부분(핸들 수명 관리, `LSTATUS` 오류 코드 처리, 2단계 버퍼 크기 조회)을 라이브러리 공통 타입으로 정리한다.

**설계 원칙:**
- 키 핸들은 `UniqueRegistryHandle`(RAII)로 반환해 `RegCloseKey` 호출을 자동화한다 — [windows_handle.md](windows_handle.md) 참조
- 부모 키는 `RegistryHandleView`로 받아, `HKEY` 상수(`HKEY_CURRENT_USER` 등)나 `UniqueRegistryHandle`을 그대로 넘길 수 있다
- 모든 함수는 성공 값 또는 오류를 담은 `Result<T>` / `Result<void>`를 반환한다 (예외를 던지지 않음) — [error_handling_architecture.md](error_handling_architecture.md) 참조
- 모든 함수에 `[[nodiscard]]`가 붙어 반환된 결과를 무시할 수 없다

---

## 아키텍처 구조

```
mwl/include/mwl/windows/registry/
└── registry.h          — 함수 선언 + doc comment

mwl/src/windows/registry/
└── registry.cpp        — 구현 (Advapi32.lib 링크)
```

네임스페이스: `mwl::windows::registry`

의존성:

| 의존 대상 | 용도 |
|---|---|
| `handle::RegistryHandleView`, `handle::UniqueRegistryHandle` | 부모 키 전달 / 결과 핸들 소유 |
| `mwl::Result<T>`, `mwl::Error` | 결과·오류 전파 |
| `Advapi32.lib` | `Reg*` API (`#pragma comment`으로 링크) |

---

## API

### 키 관리

| 함수 | WinAPI | 설명 |
|---|---|---|
| `Result<UniqueRegistryHandle> CreateKey(key, subkey, options = REG_OPTION_NON_VOLATILE, desired = KEY_ALL_ACCESS)` | `RegCreateKeyExW` | 서브 키를 생성한다. 이미 있으면 연다. |
| `Result<UniqueRegistryHandle> OpenKey(key, subkey, desired)` | `RegOpenKeyExW` | 기존 서브 키를 연다. 없으면 실패. |
| `Result<void> DeleteKey(key, subkey)` | `RegDeleteKeyW` | 서브 키를 삭제한다. 하위 서브 키가 없는 키만 삭제 가능. |

### 값 읽기

| 함수 | WinAPI | 설명 |
|---|---|---|
| `Result<DWORD> ReadDword(key, subKey, valueName)` | `RegGetValueW` (`RRF_RT_REG_DWORD`) | `REG_DWORD` 값을 읽는다. |
| `Result<DWORD64> ReadQword(key, subKey, valueName)` | `RegGetValueW` (`RRF_RT_REG_QWORD`) | `REG_QWORD` 값을 읽는다. |
| `Result<std::wstring> ReadString(key, subKey, valueName)` | `RegGetValueW` (`RRF_RT_REG_SZ`) | `REG_SZ` 문자열을 읽는다. 버퍼 크기를 먼저 조회한 뒤 본 읽기를 수행한다. |

### 값 쓰기 / 삭제

| 함수 | WinAPI | 설명 |
|---|---|---|
| `Result<void> WriteDword(key, subKey, valueName, DWORD data)` | `RegSetKeyValueW` (`REG_DWORD`) | `REG_DWORD` 값을 쓴다. 없으면 생성, 있으면 덮어씀. |
| `Result<void> WriteQword(key, subKey, valueName, DWORD64 data)` | `RegSetKeyValueW` (`REG_QWORD`) | `REG_QWORD` 값을 쓴다. |
| `Result<void> WriteString(key, subKey, valueName, wstring_view data)` | `RegSetKeyValueW` (`REG_SZ`) | `REG_SZ` 문자열을 쓴다. 널 종료 문자를 포함해 기록한다. |
| `Result<void> DeleteValue(key, valueName)` | `RegDeleteValueW` | `key` 바로 아래의 값을 삭제한다. (`subKey` 인자 없음) |

> 매개변수 `key`의 타입은 모두 `handle::RegistryHandleView`이다.

---

## 매개변수 규칙

### `key` — 기준/부모 키

`HKEY` 상수나 `UniqueRegistryHandle`을 별도 변환 없이 전달할 수 있다.

```cpp
// HKEY 상수 직접 전달
OpenKey(RegistryHandleView{ HKEY_CURRENT_USER }, L"SOFTWARE\\App", KEY_READ);

// UniqueRegistryHandle 전달 (암묵 변환)
auto app = OpenKey(...).value();
ReadDword(app, L"", L"Version");
```

### `subKey` — 추가 서브 키 경로 (값 함수)

값 읽기/쓰기 함수의 `subKey`는 `key` 아래로 더 들어갈 상대 경로다. **빈 문자열(`L""`)이면 `key` 자체**를 대상으로 한다.

```cpp
ReadString(hkcu, L"SOFTWARE\\App", L"Name");  // HKCU\SOFTWARE\App 의 Name
ReadString(app,  L"",             L"Name");   // app 핸들이 가리키는 키의 Name
```

### 반환값

- 키를 여는 함수는 `Result<UniqueRegistryHandle>` — 성공 시 핸들 소유권을 가져온다.
- 부수효과만 있는 함수는 `Result<void>` — 성공 시 값 없이 `ok()`, 실패 시 `Error`. [error_handling_architecture.md](error_handling_architecture.md) 참조.
- 오류는 `RegGetValueW` 등이 반환한 `LSTATUS`를 `mwl::LstatusError(msg, status)`로 감싼다.

---

## 사용 예시

### 키 생성 → 값 쓰기 → 값 읽기 → 정리

```cpp
namespace reg = mwl::windows::registry;
using mwl::windows::handle::RegistryHandleView;

// 1. 키 생성 (이미 있으면 열림)
auto created = reg::CreateKey(RegistryHandleView{ HKEY_CURRENT_USER }, L"SOFTWARE\\MyApp");
if (!created.ok())
    return created.error();
auto key = std::move(created.value());   // UniqueRegistryHandle

// 2. 값 쓰기 (subKey는 빈 문자열 → key 자체)
if (auto r = reg::WriteString(key, L"", L"InstallPath", LR"(C:\Program Files\MyApp)"); !r.ok())
    return r.error();
if (auto r = reg::WriteDword(key, L"", L"Version", 3); !r.ok())
    return r.error();

// 3. 값 읽기
auto path = reg::ReadString(key, L"", L"InstallPath");
auto ver  = reg::ReadDword(key, L"", L"Version");
if (path.ok() && ver.ok())
    std::wcout << path.value() << L" v" << ver.value() << L'\n';

// key 스코프 종료 시 RegCloseKey 자동 호출
```

### 값 / 키 삭제

```cpp
reg::DeleteValue(key, L"Version");                                      // 값 삭제
key.Reset();                                                           // 핸들 먼저 닫고
reg::DeleteKey(RegistryHandleView{ HKEY_CURRENT_USER }, L"SOFTWARE\\MyApp"); // 빈 키 삭제
```

> `DeleteKey`는 하위 서브 키가 없는 키만 삭제한다. 하위 트리를 통째로 지우려면 `RegDeleteTree`를 별도로 감싸야 한다(현재 미제공).

---

## 테스트

`mwl_test/src/windows/registry/registry_test.cpp` — `RegistryTest` 픽스처(GoogleTest)가 `SetUp`에서 `HKCU\SOFTWARE\MwlTest` 키를 만들고 `TearDown`에서 삭제하므로, 외부 사전 조건 없이 CRUD 전체를 검증한다. `HKEY_CURRENT_USER` 하위라 관리자 권한이 필요 없다.

---

## 확장 가이드

새 값 타입/연산을 추가할 때는 기존 패턴을 따른다.

1. `registry.h`에 `[[nodiscard]]` 함수 선언 + 한국어 doc comment(`<summary>`/`<param>`/`<returns>`) 작성
2. `registry.cpp`에 구현 — `Reg*` 호출 결과 `LSTATUS`를 검사해 성공 값 또는 `LstatusError(...)` 반환
3. `registry_test.cpp`에 round-trip 테스트(쓰기 후 읽어 일치 확인) 및 실패 케이스 추가
