#include <cstring>

#include "mwl/windows/definition.h"
#include "mwl/windows/registry/value.h"

namespace mwl::windows::registry
{

Value::Value(std::wstring_view name, Dword data) : name_(name), type_(ValueType::Dword), data_(data)
{
}

Value::Value(std::wstring_view name, Qword data) : name_(name), type_(ValueType::Qword), data_(data)
{
}

Value::Value(std::wstring_view name, String data, bool isExpand)
    : name_(name), type_(isExpand ? ValueType::ExpandString : ValueType::String), data_(data)
{
}

Value::Value(std::wstring_view name, MultiString data) : name_(name), type_(ValueType::MultiString), data_(data)
{
}

Value::Value(std::wstring_view name, Binary data) : name_(name), type_(ValueType::Binary), data_(data)
{
}

Value::Value(std::wstring_view name, ValueType type, std::span<const Byte> data) : name_(name), type_(type)
{
    switch (type_)
    {
    case ValueType::Dword: {
        assert(data.size() == sizeof(Dword));

        Dword value{};
        std::memcpy(&value, data.data(), sizeof(Dword));
        data_ = value;
        break;
    }
    case ValueType::Qword: {
        assert(data.size() == sizeof(Qword));

        Qword value{};
        std::memcpy(&value, data.data(), sizeof(Qword));
        data_ = value;
        break;
    }
    case ValueType::String:
    case ValueType::ExpandString: {
        using CharType = String::traits_type::char_type;

        assert(data.size() % sizeof(CharType) == 0);

        const CharType* buffer = reinterpret_cast<const CharType*>(data.data());
        std::size_t bufSize = data.size() / sizeof(CharType);

        // String(std::wstring)은 객체 스스로 null-terminated 문자열을 보장함으로, buffer의 마지막 null 문자를
        // 제거합니다.
        if (0 < bufSize && buffer[bufSize - 1] == L'\0')
        {
            --bufSize;
        }

        data_ = String(buffer, bufSize);
        break;
    }
    case ValueType::MultiString: {
        using CharType = MultiString::value_type::traits_type::char_type;

        assert(data.size() % sizeof(CharType) == 0);

        const CharType* buffer = reinterpret_cast<const CharType*>(data.data());
        std::size_t bufSize = data.size() / sizeof(CharType);

        MultiString strings{};
        std::size_t start = 0;

        for (std::size_t i = 0; i < bufSize; ++i)
        {
            // MULTI_SZ는 null 문자로 각 문자열을 구분하고, 마지막에는 두 개의 null 문자가 연속으로 옵니다.
            // 따라서, null 문자를 만나면 현재까지의 문자열을 추출합니다.
            // 예: "ABC\0DEF\0\0" -> "ABC", "DEF"
            if (buffer[i] == L'\0')
            {
                if (start < i)
                {
                    // buffer + start부터 i - start 길이만큼의 문자열을 추출하여 MultiString에 추가합니다.
                    strings.emplace_back(buffer + start, i - start);
                }

                start = i + 1;
            }
        }

        data_ = std::move(strings);
        break;
    }
    case ValueType::Binary:
    default:
        data_ = Binary(data.begin(), data.end());
        break;
    }
}

} // namespace mwl::windows::registry
