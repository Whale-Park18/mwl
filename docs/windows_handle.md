# Windows 핸들 아키텍처

## 개요

Windows API 핸들은 커널/GDI/유저 서브시스템이 관리하는 불투명 정수(opaque integer)로, `new`로 생성된 힙 객체가 아니다. 따라서 `std::unique_ptr`의 기본 삭제자(`delete`)를 그대로 사용하면 핸들이 정상적으로 해제되지 않는다.

**설계 원칙:**
- 핸들 종류마다 유효성 판단(`IsValid`)과 해제 함수(`Close`)를 Traits 타입으로 분리한다
- `UniqueHandle<Traits>`는 Traits를 통해 `nullptr`과 `INVALID_HANDLE_VALUE`를 모두 무효 값으로 처리한다
- `HandleView<Traits>`는 소유권 이전 없이 함수에 핸들을 전달하기 위한 비소유 뷰 타입이다
- 핸들 카테고리별로 파일을 분리하고, 필요한 파일만 포함한다

---

## 아키텍처 구조

```
mwl/include/mwl/windows/handle/
├── unique_handle.h      — UniqueHandle<Traits> 템플릿 (소유 타입)
├── handle_view.h        — HandleView<Traits> 템플릿 (비소유 뷰)
├── kernel_handle.h      — HANDLE 계열 (KernelHandleTraits)
├── registry_handle.h    — HKEY (RegistryHandleTraits)
└── (차후 필요에 따라 추가)
```

네임스페이스: `mwl::windows::handle`

---

## 핸들 카테고리

| 카테고리 | 타입 | 해제 함수 | 파일 |
|---|---|---|---|
| 커널 오브젝트 | `HANDLE` | `CloseHandle` | `kernel_handle.h` |
| 레지스트리 | `HKEY` | `RegCloseKey` | `registry_handle.h` |

---

## Traits 구조

핸들 종류마다 유효성 판단과 해제 함수를 Traits 타입으로 분리한다. Traits는 세 가지 정적 멤버를 제공한다.

- `IsValid(h)` — 핸들이 유효한지 판단
- `Empty()` — 빈 상태를 나타내는 canonical 값 반환 (기본 생성/Reset에 사용)
- `Close(h)` — 핸들 해제

```cpp
struct ExampleTraits
{
    using Type = SomeHandleType;

    static bool IsValid(Type handle) noexcept { ... }
    static Type Empty() noexcept { ... }
    static void Close(Type handle) noexcept { ... }
};
```

---

## `unique_handle.h` — 소유 타입

### `UniqueHandle<Traits>`

이동 전용 소유 타입이다. Traits를 통해 유효성을 판단하므로 `nullptr`과 `INVALID_HANDLE_VALUE`를 모두 무효 값으로 처리할 수 있다.

| 멤버 | 설명 |
|---|---|
| `UniqueHandle()` | 빈 상태(`Traits::Empty()`)로 기본 생성 |
| `UniqueHandle(Handle h)` | 핸들을 소유하며 생성 (암묵적 변환 허용) |
| `explicit operator bool()` | `Traits::IsValid(handle_)` |
| `explicit operator Handle()` | 내부 핸들 값으로 변환 |
| `Handle Get()` | 내부 핸들 값 반환 |
| `Handle Release()` | 소유권을 포기하고 핸들 반환 |
| `void Reset(Handle h = Traits::Empty())` | 이전 핸들을 닫고 새 핸들로 교체 |
| `void Swap(UniqueHandle& other)` | 두 인스턴스의 핸들 교환 |

복사 생성자와 복사 대입 연산자는 `= delete`로 금지된다.

---

## `handle_view.h` — 비소유 뷰 타입

### `HandleView<Traits>`

소유권 이전 없이 함수에 핸들을 전달하기 위한 타입이다. `std::string_view`가 `std::string`의 비소유 뷰인 것과 동일한 관계다.

| 멤버 | 설명 |
|---|---|
| `HandleView()` | 빈 상태(`Traits::Empty()`)로 기본 생성 |
| `HandleView(Handle h)` | 핸들 값으로부터 생성 (암묵적 변환 허용) |
| `HandleView(const UniqueHandle<Traits>& h)` | `UniqueHandle`로부터 생성 (암묵적 변환 허용) |
| `explicit operator bool()` | `Traits::IsValid(handle_)` |
| `explicit operator Handle()` | 내부 핸들 값으로 변환 |
| `Handle Get()` | 내부 핸들 값 반환 |

`HandleView`는 원본 `UniqueHandle`의 수명 안에서만 유효하다. 함수 매개변수 전달 전용으로 설계되었으며, 저장하거나 반환해서는 안 된다.

생성자가 `explicit`이 아니므로, `HandleView`를 받는 함수에 원시 핸들 값(`HKEY_CURRENT_USER` 등)이나 `UniqueHandle`을 별도 변환 없이 그대로 넘길 수 있다.

---

## `kernel_handle.h` — 커널 오브젝트 핸들

프로세스, 스레드, 파일, 동기화 오브젝트 등 `CreateFile`, `OpenProcess` 계열 API가 반환하는 `HANDLE`을 관리한다.

`KernelHandleTraits::IsValid`는 `nullptr`과 `INVALID_HANDLE_VALUE`를 모두 무효 값으로 처리한다.

```
KernelHandle (HANDLE)
└── 해제: CloseHandle
    ├── nullptr 검사
    └── INVALID_HANDLE_VALUE 검사 (CreateFile 실패 시 반환값)
```

### 제공 타입

```cpp
using KernelHandle       = HANDLE;
struct KernelHandleTraits { ... };
using UniqueKernelHandle = UniqueHandle<KernelHandleTraits>;
```

---

## `registry_handle.h` — 레지스트리 핸들

`RegOpenKeyEx` / `RegCreateKeyEx` 계열 API가 반환하는 `HKEY`를 관리한다. `HKEY_LOCAL_MACHINE` 등 predefined 키는 `RegCloseKey`를 호출해도 안전하다(ERROR_SUCCESS 반환).

```
RegistryHandle (HKEY)
└── 해제: RegCloseKey
    └── nullptr 검사
```

### 제공 타입

```cpp
using RegistryHandle       = HKEY;
struct RegistryHandleTraits { ... };
using UniqueRegistryHandle = UniqueHandle<RegistryHandleTraits>;
```

---

## 사용 예시

### 단독 소유 — `UniqueKernelHandle`

```cpp
// CreateFile: 실패 시 INVALID_HANDLE_VALUE 반환
HANDLE raw = ::CreateFile(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
if (raw == INVALID_HANDLE_VALUE)
    return mwl::LastWindowsError("CreateFile failed");

UniqueKernelHandle file(raw);
// 스코프 종료 시 CloseHandle 자동 호출
```

```cpp
// OpenProcess: 실패 시 nullptr 반환
HANDLE raw = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
if (raw == nullptr)
    return mwl::LastWindowsError("OpenProcess failed");

UniqueKernelHandle process(raw);
```

### 매개변수 전달 — `HandleView`

소유권을 유지한 채 함수에 핸들을 전달한다.

```cpp
// 함수 시그니처: 소유하지 않음을 명시
mwl::Result<std::wstring> QueryImageName(HandleView<KernelHandleTraits> process)
{
    WCHAR buf[MAX_PATH];
    DWORD size = MAX_PATH;
    if (!::QueryFullProcessImageNameW(process.Get(), 0, buf, &size))
        return mwl::LastWindowsError("QueryFullProcessImageNameW failed");
    return std::wstring(buf, size);
}

// 호출: UniqueKernelHandle → HandleView 암묵적 변환 (생성자가 explicit이 아님)
UniqueKernelHandle process(raw);
auto name = QueryImageName(process);
```

### 레지스트리 키 — `UniqueRegistryHandle`

```cpp
HKEY raw_key = nullptr;
LSTATUS st = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\...", 0, KEY_READ, &raw_key);
if (st != ERROR_SUCCESS)
    return mwl::LstatusError("RegOpenKeyExW failed", st);

UniqueRegistryHandle key(raw_key);
// 스코프 종료 시 RegCloseKey 자동 호출
```

---

## 확장 가이드

새로운 핸들 카테고리를 추가할 때는 아래 패턴을 따른다.

1. `handle/` 디렉토리에 `xxx_handle.h` 파일 생성
2. `#include "unique_handle.h"` 포함
3. Traits 구조체(`struct XxxHandleTraits`) 구현 — `Type`, `IsValid`, `Empty`, `Close` 제공
4. `UniqueXxxHandle` 별칭 선언
