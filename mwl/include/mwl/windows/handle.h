#pragma once
#include <Windows.h>

#include <concepts>
#include <memory>
#include <type_traits>

namespace mwl::windows
{

using Handle = HANDLE;
using Hkey = HKEY;

template<class HandleT>
concept HandleType = std::same_as<Handle, HandleT> || std::same_as<Hkey, HandleT>;

template<HandleType HandleT> struct HandleDeleter
{
    void operator()(HandleT handle)
    {
        if constexpr (std::same_as<HandleT, mwl::windows::Handle>)
        {
            if (handle != nullptr)
            {
                ::CloseHandle(handle);
                handle = nullptr;
            }
        }
        else if (std::same_as<HandleT, mwl::windows::Hkey>)
        {
            if (handle != nullptr)
            {
                ::RegCloseKey(handle);
                handle = nullptr;
            }
        }
    }
};

template<HandleType HandleT>
using UniqueHandle = std::unique_ptr<std::remove_pointer_t<HandleT>, HandleDeleter<HandleT>>;

template<HandleType HandleT> using SharedHandle = std::shared_ptr<std::remove_pointer_t<HandleT>>;

template<HandleType HandleT> using WeakHandle = std::weak_ptr<std::remove_pointer_t<HandleT>>;

template<HandleType HandleT> inline UniqueHandle<HandleT> MakeUniqueHandle(HandleT handle)
{
    return UniqueHandle<HandleT>(handle);
}

template<HandleType HandleT> inline SharedHandle<HandleT> MakeSharedHandle(HandleT handle)
{
    return SharedHandle<HandleT>(handle, HandleDeleter<HandleT>{});
}

template<HandleType HandleT> inline WeakHandle<HandleT> MakeWeakHandle(HandleT shared_handle)
{
    return WeakHandle<HandleT>(shared_handle);
}

} // namespace mwl::windows
