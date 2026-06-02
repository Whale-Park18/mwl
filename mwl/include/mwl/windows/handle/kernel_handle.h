#pragma once
#include <Windows.h>

#include "unique_handle.h"

namespace mwl::windows::handle
{

using KernelHandle = HANDLE;

struct KernelHandleTraits
{
    using Type = KernelHandle;

    static bool IsValid(Type handle) noexcept
    {
        // Windows API에서 INVALID_HANDLE_VALUE은 일반적으로 파일 핸들에 대한 오류를 나타내며, 다른 유형의 핸들에서는
        // nullptr이 유효하지 않은 핸들을 나타냅니다.
        return handle != nullptr && handle != INVALID_HANDLE_VALUE;
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

        ::CloseHandle(handle);
    }
};

using UniqueKernelHandle = UniqueHandle<KernelHandleTraits>;

} // namespace mwl::windows::handle
