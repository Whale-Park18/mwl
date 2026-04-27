#include <pch.h>

#include <mwl/windows/handle.h>

TEST(WindowsHandle, UniqueHandle)
{
	auto handle = mwl::windows::make_unique_handle(::CreateEventW(nullptr, FALSE, FALSE, nullptr));
}