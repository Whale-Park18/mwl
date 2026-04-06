#pragma once
#include "handle.h"

namespace mwl::windows
{

enum class reg_root : ULONG_PTR
{
    classes_root = (ULONG_PTR)HKEY_CLASSES_ROOT,
    current_config = (ULONG_PTR)HKEY_CURRENT_CONFIG,
    current_user = (ULONG_PTR)HKEY_CURRENT_USER,
    local_machine = (ULONG_PTR)HKEY_LOCAL_MACHINE,
    users = (ULONG_PTR)HKEY_USERS,
};

template <reg_root R>
class reg_handler
{
public:
    reg_handler() = default;

private:
    shared_handle<hkey> hkey;
};

} // namespace mwl::windows