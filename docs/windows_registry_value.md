# Windows 레지스트리 값(Value) 타입

## 개요

레지스트리 값은 이름마다 타입(`REG_SZ`, `REG_DWORD`, `REG_BINARY` 등)이 다를 수 있다. `EnumValues`처럼 한 키 아래의 모든 값을 나열하거나, `ReadValue`/`WriteValue`처럼 타입별 Read*/Write* 함수를 하나로 통합하려면, 서로 다른 타입의 값을 동일한 타입의 객체로 다룰 수 있어야 한다. `Value`/`ValueType`은 이 문제를 해결하기 위한 타입이다.

**설계 원칙:**
- `Value`는 비템플릿 클래스이며 `std::variant`로 런타임에 결정되는 값의 종류를 표현한다 — 컴파일타임 템플릿 매개변수로는 `std::vector<Value>`에 이질적인 타입을 담을 수 없다
- `ValueType` enum은 `REG_*` WinAPI 상수와 1:1 대응하며, `Value`의 `data_`가 어떤 variant 대체자를 사용하는지 결정한다
- `String`(`REG_SZ`)과 `ExpandString`(`REG_EXPAND_SZ`)처럼 C++ 표현이 동일한(`std::wstring`) 타입은 `ValueType` 파라미터로 구분한다
- `data<T>()`로 variant를 직접 노출하지 않고 타입 안전하게 접근한다 — `T`가 활성 대체자가 아니면 `std::nullopt`를 반환하는 `std::optional<T>` 값 복사본을 돌려준다 (레지스트리 값은 대부분 작아 복사 비용이 무시할 만하다)

---

## 아키텍처 구조

```
mwl/include/mwl/windows/registry/
└── value.h             — ValueType enum, 타입 별칭, Value 클래스 선언

mwl/src/windows/registry/
└── value.cpp           — 구현
```

네임스페이스: `mwl::windows::registry`

의존성:

| 의존 대상 | 용도 |
|---|---|
| `<Windows.h>` | `DWORD`, `DWORD64`, `BYTE`, `REG_*` 상수 |
| `<variant>` | `Value::Data`(`std::variant`)로 런타임 타입 표현 |
| `<string>`, `<vector>` | `String`(`std::wstring`), `MultiString`/`Binary`(`std::vector<...>`) |

---

## ValueType 열거형

`ValueType`의 각 enumerator는 동일한 이름의 WinAPI `REG_*` 상수 값을 가지며, `Value::Data`의 특정 variant 대체자에 대응한다.

| ValueType | WinAPI 상수 | variant 대체자 (별칭) | 설명 |
|---|---|---|---|
| `None` | `REG_NONE` | `std::monostate` | 값 없음 |
| `String` | `REG_SZ` | `String` (`std::wstring`) | null-terminated 문자열 |
| `ExpandString` | `REG_EXPAND_SZ` | `String` (`std::wstring`) | `%VAR%` 형태의 환경 변수 참조를 포함하는 문자열. raw 데이터 표현은 `String`과 동일하며, 읽기/쓰기 시 OS의 확장 처리 여부만 다르다 |
| `MultiString` | `REG_MULTI_SZ` | `MultiString` (`std::vector<std::wstring>`) | null로 구분되고 전체가 이중 null로 끝나는 문자열 배열 |
| `Binary` | `REG_BINARY` | `Binary` (`std::vector<BYTE>`) | 임의 길이의 이진 데이터 |
| `Dword` | `REG_DWORD` | `Dword` (`DWORD`) | 32비트 정수 |
| `Qword` | `REG_QWORD` | `Qword` (`DWORD64`) | 64비트 정수 |

타입 별칭(`Dword`, `Qword`, `String`, `MultiString`, `Binary`)은 `ValueType`의 동명 enumerator와 짝을 이루도록 선언되어 있다. `enum class`이므로 `ValueType::Dword`와 네임스페이스 스코프의 `Dword` 별칭은 서로 다른 식별자라 충돌하지 않는다.

---

## Value 클래스

### `Data` — 값을 저장하는 variant

```cpp
using Data = std::variant<std::monostate, Dword, Qword, String, MultiString, Binary>;
```

`type_`(`ValueType`)이 `data_`(`Data`)의 활성 대체자가 무엇인지를 나타내는 태그 역할을 한다. `String`/`ExpandString`처럼 동일한 대체자(`std::wstring`)를 공유하는 타입은 `type_`으로만 구분된다.

### 생성자

| 생성자 | 용도 | 상태 |
|---|---|---|
| `Value()` | 기본 생성. `type_ = ValueType::None`, `data_ = std::monostate{}` | 구현됨 |
| `Value(name, Dword data)` | `REG_DWORD` 값 생성 (Write용) | 구현됨 |
| `Value(name, Qword data)` | `REG_QWORD` 값 생성 (Write용) | 구현됨 |
| `Value(name, MultiString data)` | `REG_MULTI_SZ` 값 생성 (Write용) | 구현됨 |
| `Value(name, Binary data)` | `REG_BINARY` 값 생성 (Write용) | 구현됨 |
| `Value(name, String data, ValueType type = ValueType::String)` | `REG_SZ`/`REG_EXPAND_SZ` 값 생성 (Write용). `type`은 `ValueType::String` 또는 `ValueType::ExpandString`만 허용하며, 그 외 값은 `assert`로 막는다 | **설계 결정 — `value.h` 미반영** |
| `Value(name, ValueType type, std::span<const Byte> data)` | `RegEnumValueW`/`RegGetValueW`가 반환한 raw 바이트 버퍼로부터 생성 (Read/Enum용). `type`에 따라 `data`의 바이트를 적절한 `Data` 대체자로 파싱한다. `data`는 호출 후 실제로 기록된 바이트 수로 길이를 맞춘 view여야 한다 (버퍼의 할당 크기가 아님) | 구현됨 |

`String`/`ExpandString` 생성자에 `ValueType` 파라미터를 둔 이유: 두 타입의 raw 데이터(`std::wstring`)가 동일하므로, 별도 enum이나 named factory 없이 기존 `ValueType`을 재사용해 의도를 명시적으로 표현할 수 있다.

raw 버퍼 생성자가 비템플릿인 이유: WinAPI 버퍼는 항상 raw 바이트이며, 그 바이트를 어떤 `Data` 대체자로 해석할지는 오직 `type`(런타임 값)에 의해 결정된다. 템플릿 매개변수 `T`를 두면 호출자가 `T`와 `type`을 일치시켜야 하는 중복된 책임이 생기고, 둘이 불일치해도 컴파일 타임에 잡을 수 없다 — 따라서 `std::span<const Byte>` 단일 비템플릿 오버로드로 설계했다.

### 접근자

| 멤버 | 반환 타입 | 설명 |
|---|---|---|
| `name()` | `const std::wstring&` | 값 이름 |
| `type()` | `ValueType` | 값의 종류 |
| `template<class T> data()` | `std::optional<T>` | `data_`에서 `T` 대체자를 복사본으로 꺼낸다. `T`가 활성 대체자가 아니면 `std::nullopt` |

---

## 사용 예시

### Write용 `Value` 생성

```cpp
using namespace mwl::windows::registry;

Value version(L"Version", Dword{ 3 });
Value installPath(L"InstallPath", String{ LR"(C:\Program Files\MyApp)" });

// ExpandString: ValueType을 명시해야 String과 구분됨 (위 "설계 결정" 적용 형태)
Value tempPath(L"TEMP", String{ LR"(%USERPROFILE%\AppData\Local\Temp)" }, ValueType::ExpandString);

Value dependsOn(L"DependsOn", MultiString{ L"RpcSs", L"Tcpip" });
Value hash(L"Hash", Binary{ 0x01, 0x02, 0x03 });
```

### `EnumValues` 결과 순회

```cpp
auto values = reg::EnumValues(key, L"SOFTWARE\\MyApp");
if (!values.ok())
    return values.error();

for (const Value& value : values.value())
{
    switch (value.type())
    {
    case ValueType::Dword:
        std::wcout << value.name() << L" = " << value.data<Dword>().value() << L"\n";
        break;

    case ValueType::String:
    case ValueType::ExpandString:
        std::wcout << value.name() << L" = " << value.data<String>().value() << L"\n";
        break;

    case ValueType::MultiString:
        for (const auto& s : value.data<MultiString>().value())
            std::wcout << L"  " << s << L"\n";
        break;

    case ValueType::Binary:
        // value.data<Binary>().value() — std::vector<BYTE>
        break;

    default:
        break; // None
    }
}
```

---

## 확장 가이드 / 향후 계획

- **raw 버퍼 생성자** (`Value(name, ValueType, std::span<const Byte>)`) — 구현 완료:
  - `Dword`/`Qword`: `std::memcpy`로 버퍼를 지역 변수에 복사 후 저장한다 (정렬 보장이 없는 raw 바이트 포인터를 `reinterpret_cast`로 직접 역참조하면 UB이므로 memcpy 사용). 크기가 `sizeof(Dword)`/`sizeof(Qword)`와 다르면 `assert`로 검사한다 (내부 불변식 위반 — `Result<T>` 대상이 아님, [error_handling_architecture.md](error_handling_architecture.md) 참조)
  - `String`/`ExpandString`: 바이트를 `wchar_t` 시퀀스로 재해석하고, 끝에 단일 trailing null이 있으면 제거한 뒤 `std::wstring`을 구성한다
  - `MultiString`: `wchar_t` 시퀀스를 null 문자 기준으로 분리해 `std::vector<std::wstring>`을 구성한다. 마지막 구분자 뒤 빈 문자열은 생성하지 않는다 (이중 null 종료 관례를 자연스럽게 처리)
  - `Binary` 및 위 표에 없는 타입(`default`): `std::vector<BYTE>`로 raw 바이트를 그대로 복사해 보존한다
- **`String`/`ExpandString` 생성자에 `ValueType` 파라미터 추가** — 위 "설계 결정"을 `value.h`에 반영
- **API 통합**: `ReadDword`/`ReadQword`/`ReadString` → `ReadValue`, `WriteDword`/`WriteQword`/`WriteString` → `WriteValue`로 통합 — [windows_registry.md](windows_registry.md) 참조
