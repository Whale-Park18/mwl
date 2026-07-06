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

        const CharType* chars = reinterpret_cast<const CharType*>(data.data());
        size_t charCount = data.size() / sizeof(CharType);
        if (charCount > 0 &&
            chars[charCount - 1] == L'\0') // null-terminated 문자열이므로 마지막 null 문자를 제거합니다.
        {
            --charCount;
        }

        data_ = String(chars, charCount);
        break;
    }
    case ValueType::MultiString: {
        using CharType = MultiString::value_type::traits_type::char_type;

        assert(data.size() % sizeof(CharType) == 0);

        const CharType* chars = reinterpret_cast<const CharType*>(data.data());
        size_t charCount = data.size() / sizeof(CharType);
        MultiString strings{};
        size_t start = 0;
        for (size_t i = 0; i < charCount; ++i)
        {
            if (chars[i] == L'\0')
            {
                if (i > start)
                    strings.emplace_back(chars + start, i - start);
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
