#include <pch.h>

#include <Windows.h>
#include <mwl/windows/handle.h>

TEST(WindowsHandle, HandleTypeConcepts)
{
	static_assert(mwl::windows::handle_type<HANDLE>, "mwl::windows::handle_type concepts, HANDLE");
	static_assert(mwl::windows::handle_type<mwl::windows::handle>, "mwl::windows::handle_type concepts, mwl::windows::handle");

	static_assert(mwl::windows::handle_type<HKEY>, "mwl::windows::handle_type concepts, HKEY");
	static_assert(mwl::windows::handle_type<mwl::windows::hkey>, "mwl::windows::handle_type concepts, mwl::windows::hkey");

	//static_assert(mwl::windows::handle_type<int>, "mwl::windows::handle_type concepts, int");
	//static_assert(mwl::windows::handle_type<int*>, "mwl::windows::handle_type concepts, int*");

	EXPECT_TRUE(true);
}

TEST(WindowsHandle, UniqueHandleDeleter)
{
	mwl::windows::handle handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	auto uniqueHandle = mwl::windows::make_unique_handle(handle);

	EXPECT_TRUE(true);
}

TEST(WindowsHandle, SharedHandleDeleter)
{
	mwl::windows::handle handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
	auto shared_handle = mwl::windows::make_shared_handle(handle);

	EXPECT_TRUE(true);
}