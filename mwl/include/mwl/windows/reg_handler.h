#pragma once
#include "handle.h"

namespace mwl::windows
{

enum class RegRoot : ULONG_PTR
{
    kClassesRoot = (ULONG_PTR)HKEY_CLASSES_ROOT,
    kCurrentConfig = (ULONG_PTR)HKEY_CURRENT_CONFIG,
    kCurrentUser = (ULONG_PTR)HKEY_CURRENT_USER,
    kLocalMachine = (ULONG_PTR)HKEY_LOCAL_MACHINE,
    kUsers = (ULONG_PTR)HKEY_USERS,
};

template<RegRoot kRoot> class RegHandler
{
public:

    RegHandler() = default;

private:

    SharedHandle<Hkey> hkey_;
};

} // namespace mwl::windows
