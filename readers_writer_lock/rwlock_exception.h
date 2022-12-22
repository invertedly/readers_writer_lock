#pragma once
#include <exception>
#include <string>

namespace rwlock
{
	class rwlock_exception final : public std::exception
	{
	public:
		explicit rwlock_exception(const std::string& message) : exception(message.data()) {}
	};
}
