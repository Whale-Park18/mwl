# 에러 처리 아키텍처

## 개요

`mwl::error`는 C++ 예외(`throw`/`catch`) 없이 반환값 기반으로 에러를 전파하는 아키텍처다. Google Abseil의 `absl::Status`/`absl::StatusOr` 패턴과 게임 엔진(Unreal Engine, id Software)의 에러 처리 방식을 벤치마킹하여 설계했다.

**설계 원칙:**
- C++ 예외를 사용하지 않는다 — 에러는 반환값으로만 전파한다
- 커스텀 에러 코드 enum 대신 STL의 `std::error_code` / `std::errc`를 재사용한다
- 성공 경로에서 힙 할당이 발생하지 않는다
- 에러 발생 위치(`std::source_location`)를 자동으로 캡처한다

---

## 아키텍처 구조

```
mwl/include/mwl/error/
├── error.h       — Error 클래스, hresult_category(), 팩토리 함수 선언
└── result.h      — Result<T> 클래스 템플릿 (헤더 전용)

mwl/src/error/
└── error.cpp     — HresultCategory, Error, 팩토리 함수 구현
```

---

## 레이어 설계

에러 코드 소스에 따라 두 레이어로 구분한다.

```
┌─────────────────────────────────────────────────────┐
│  상위 레이어 (API 경계)                               │
│  std::errc — POSIX 표준 에러 코드                    │
│  InvalidArgumentError, NotFoundError, ...            │
├─────────────────────────────────────────────────────┤
│  하위 레이어 (WinAPI 래핑)                            │
│  std::system_category()  — Win32 / WSA / LSTATUS    │
│  hresult_category()      — 순수 COM HRESULT          │
│  LastWindowsError, LastWsaError, LstatusError,     │
│  HresultError                                        │
└─────────────────────────────────────────────────────┘
```

### Windows 에러 소스별 대응

| 에러 소스 | 타입 | error_category | 팩토리 함수 |
|---|---|---|---|
| `GetLastError()` | `DWORD` | `std::system_category()` | `LastWindowsError` |
| `WSAGetLastError()` | `int` | `std::system_category()` | `LastWsaError` |
| `LSTATUS` | `LONG` | `std::system_category()` | `LstatusError` |
| `HRESULT` (`FACILITY_WIN32`) | `LONG` | `std::system_category()` | `HresultError` (내부 분기) |
| `HRESULT` (순수 COM) | `LONG` | `hresult_category()` | `HresultError` (내부 분기) |

`GetLastError()`, `WSAGetLastError()`, `LSTATUS`는 모두 Win32 에러 코드 도메인이므로 `std::system_category()`로 동일하게 처리된다.

---

## `Error` 클래스

`mwl::error::Error`는 성공 또는 에러를 나타내는 값 타입이다.

### 내부 구조

```
Error
└── std::unique_ptr<ErrorPayload> payload_
    ├── nullptr          → 성공 (ok() == true)
    └── ErrorPayload*    → 에러
        ├── std::error_code ec
        ├── std::string message
        └── std::source_location location
```

성공 상태(`payload_ == nullptr`)에서는 힙 할당이 전혀 발생하지 않는다.  
`sizeof(Error) == sizeof(void*)` — 포인터 하나 크기.

### 주요 인터페이스

```cpp
class Error
{
public:
    Error() noexcept;                                      // 성공 상태 생성

    bool ok() const noexcept;                              // 성공 여부
    explicit operator bool() const noexcept;               // ok()와 동일

    std::error_code code() const noexcept;                 // 에러 코드
    std::string_view message() const noexcept;             // 에러 메시지
    const std::source_location& location() const noexcept; // 발생 위치
    std::string ToString() const;                          // "[file:line] category:value - message"
};
```

### `source_location` 캡처 규칙

팩토리 함수만 `std::source_location::current()` 기본값을 가진다.  
`Error` 생성자는 `source_location`을 명시적으로만 받는다.

```cpp
// ✅ 팩토리 — 호출자 위치 자동 캡처
Error NotFoundError(std::string_view msg,
    std::source_location loc = std::source_location::current());

// ✅ 생성자 — 명시적 전달만
Error(std::error_code ec, std::string_view message, std::source_location location);
```

생성자에 기본값을 두면 `error.cpp` 내부 위치가 캡처되는 버그가 발생한다.

### 에러 코드 0 자동 처리

생성자 내부에서 `if (ec)` 체크를 통해, 에러 코드 값이 0인 경우 payload를 생성하지 않는다.

```cpp
Error::Error(std::error_code ec, std::string_view message, std::source_location location)
{
    if (ec)  // value != 0일 때만 payload 생성
        payload_ = std::make_unique<ErrorPayload>(ec, std::string(message), location);
}
```

따라서 `LstatusError("msg", ERROR_SUCCESS)`는 자동으로 `ok() == true`를 반환한다.

---

## 팩토리 함수

### 상위 레이어 — `std::errc` 기반

| 함수 | `std::errc` 매핑 |
|---|---|
| `NoError()` | — (성공, 힙 할당 없음) |
| `InvalidArgumentError(msg)` | `std::errc::invalid_argument` |
| `NotFoundError(msg)` | `std::errc::no_such_file_or_directory` |
| `PermissionDeniedError(msg)` | `std::errc::permission_denied` |
| `AlreadyExistsError(msg)` | `std::errc::file_exists` |
| `ResourceExhaustedError(msg)` | `std::errc::not_enough_memory` |
| `TimedOutError(msg)` | `std::errc::timed_out` |
| `NotSupportedError(msg)` | `std::errc::not_supported` |

### 하위 레이어 — WinAPI 기반

| 함수 | 동작 |
|---|---|
| `LastWindowsError(msg)` | `GetLastError()`를 자동 캡처 |
| `LastWsaError(msg)` | `WSAGetLastError()`를 자동 캡처 |
| `LstatusError(msg, LSTATUS)` | `LSTATUS` 값을 직접 전달 |
| `HresultError(msg, HRESULT)` | `SUCCEEDED(hr)`이면 `NoError()` 반환 |

### `HresultError` 내부 분기

```
HRESULT hr
├── SUCCEEDED(hr)                          → NoError()
├── HRESULT_FACILITY(hr) == FACILITY_WIN32 → std::system_category(), HRESULT_CODE(hr)
└── 그 외 (순수 COM)                        → hresult_category(), hr 원본 값
```

---

## `Result<T>` 클래스 템플릿

`mwl::error::Result<T>`는 값 `T` 또는 에러 `Error`를 보유하는 타입이다.  
C++23의 `std::expected<T, E>`에 대응하며, `std::variant<T, Error>`로 구현한다.

### 주요 인터페이스

```cpp
template<class T>
class Result
{
public:
    Result(T value);       // 값 보유 — ok() == true
    Result(Error error);   // 에러 보유 — 전제: !error.ok()

    bool ok() const noexcept;
    explicit operator bool() const noexcept;

    const T& value() const&;   // 전제: ok()
    T& value() &;
    T value() &&;              // 이동 추출

    const Error& error() const noexcept;  // 전제: !ok()
};
```

**전제 조건 위반 시 동작:** `assert()`로 디버그 빌드에서 즉시 중단한다. 릴리스 빌드에서는 UB로, 호출자의 책임이다.

### 이동 전용 타입 지원

`T`가 이동 전용(`std::unique_ptr` 등)이면, `Result<T>`도 이동 전용이 된다 — `std::variant`의 자동 처리.

---

## 사용 예시

### 상위 레이어 — 입력 검증

```cpp
mwl::error::Error ValidatePath(std::string_view path)
{
    if (path.empty())
        return mwl::error::InvalidArgumentError("path must not be empty");
    if (path.size() > MAX_PATH)
        return mwl::error::InvalidArgumentError("path exceeds MAX_PATH");
    return mwl::error::NoError();
}

// 호출
if (auto e = ValidatePath(user_input); !e.ok())
{
    mwl::internal::Log(e.ToString().c_str());
    return e;
}
```

### 하위 레이어 — GetLastError

```cpp
mwl::error::Result<mwl::windows::UniqueHandle<HANDLE>>
    OpenProcessHandle(DWORD pid)
{
    HANDLE h = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (h == nullptr)
        return mwl::error::LastWindowsError("OpenProcess failed");
    return mwl::windows::MakeUniqueHandle(h);
}

// 호출
auto result = OpenProcessHandle(pid);
if (!result.ok())
{
    // result.error().code().value() == Win32 error code
    return result.error();
}
auto& handle = result.value();
```

### 하위 레이어 — LSTATUS

```cpp
mwl::error::Result<mwl::windows::UniqueHandle<HKEY>>
    OpenRegKey(HKEY root, std::wstring_view sub_key)
{
    HKEY key = nullptr;
    LSTATUS st = ::RegOpenKeyExW(root, sub_key.data(), 0, KEY_READ, &key);
    if (st != ERROR_SUCCESS)
        return mwl::error::LstatusError("RegOpenKeyExW failed", st);
    return mwl::windows::MakeUniqueHandle(key);
}
```

### 하위 레이어 — HRESULT (COM)

```cpp
mwl::error::Error InitializeCom()
{
    HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    // SUCCEEDED(hr)이면 내부적으로 NoError() 반환
    return mwl::error::HresultError("CoInitializeEx failed", hr);
}
```

### 에러 전파

```cpp
mwl::error::Error DoWork()
{
    if (auto e = Step1(); !e.ok()) return e;
    if (auto e = Step2(); !e.ok()) return e;
    return mwl::error::NoError();
}
```

---

## `hresult_category` — 커스텀 에러 카테고리

`FACILITY_WIN32` 외의 COM HRESULT는 `std::system_category()`가 의미 있는 메시지를 제공하지 못한다. `HresultCategory`는 `std::error_category`를 상속하며, `FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM)`를 사용해 COM HRESULT에 대한 인간이 읽을 수 있는 메시지를 반환한다.

```cpp
// error_category 비교
e.code().category() == std::system_category()   // Win32 / FACILITY_WIN32 HRESULT
e.code().category() == mwl::error::hresult_category()  // 순수 COM HRESULT
```

---

## 검증 기준

| 검증 항목 | 방법 |
|---|---|
| OK 경로 힙 할당 없음 | `static_assert(sizeof(Error) == sizeof(void*))` |
| `source_location` 정확도 | `NotFoundError("x").location().file_name()`이 `error_test.cpp`를 가리키는지 |
| Win32 에러 카테고리 | `LastWindowsError(...).code().category() == std::system_category()` |
| HRESULT FACILITY_WIN32 분기 | `HresultError("", HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED)).code().category() == std::system_category()` |
| HRESULT COM 분기 | `HresultError("", E_NOINTERFACE).code().category() == hresult_category()` |
| HRESULT 성공 처리 | `HresultError("", S_OK).ok() == true` |
