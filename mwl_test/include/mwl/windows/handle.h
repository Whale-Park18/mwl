#pragma once
#include <Windows.h>

#include <memory>
#include <concepts>
#include <type_traits>

namespace mwl::windows
{
	using handle = HANDLE;
	using hkey = HKEY;

	template<class H>
	concept handle_type = std::same_as<handle, H> || std::same_as<hkey, H>;

	template<handle_type H>
	struct handle_deleter
	{
		void operator()(H handle)
		{
			if constexpr (std::same_as<handle, H>)
			{
				if (handle != nullptr)
				{
					::CloseHandle(handle);
					handle = nullptr;
				}
			}
			else if(std::same_as<hkey, H>)
			{
				if (handle != nullptr)
				{
					::RegCloseKey(handle);
					handle = nullptr;
				}
			}
		}
	};

	template<handle_type H>
	using unique_handle = std::unique_ptr<std::remove_pointer_t<H>, handle_deleter<H>>;

	template<handle_type H>
	using shared_handle = std::shared_ptr<std::remove_pointer_t<H>>;

	template<handle_type H>
	using weak_handle = std::weak_ptr<std::remove_pointer_t<H>>;

	template<handle_type H>
	inline unique_handle<H> make_unique_handle(H handle) { return unique_handle<H>(handle); }

	template<handle_type H>
	inline shared_handle<H> make_shared_handle(H handle) { return shared_handle<H>(handle, handle_deleter<H>{}); }

	template<handle_type H>
	inline weak_handle<H> make_weak_handle(H shared_handle) { return weak_handle<H>(shared_handle); }
}