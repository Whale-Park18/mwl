#pragma once
#include <Windows.h>

#include "unique_handle.h"
#include "handle_view.h"

namespace mwl::windows::handle
{

using RegistryHandle = HKEY;

struct RegistryHandleTraits
{
    using Type = RegistryHandle;

    static bool IsValid(Type handle) noexcept
    {
        return handle != nullptr;
    }

    static Type Empty() noexcept
    {
        return nullptr;
    }

    static void Close(Type handle) noexcept
    {
        if (!IsValid(handle))
        {
            return;
        }

        ::RegCloseKey(handle);
    }
};

using UniqueRegistryHandle = UniqueHandle<RegistryHandleTraits>;
using RegistryHandleView = HandleView<RegistryHandleTraits>;

} // namespace mwl::windows::handle
