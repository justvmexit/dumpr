#pragma once

#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

namespace dumpr
{
	enum request_type_e : int
	{
		read,
		write,
		base_address
	};

	typedef struct _request_t {
		dumpr::request_type_e type;
		std::uint32_t pid;
		std::uint64_t address;
		std::uint64_t buffer;
		std::size_t size;
	} request_t;

	class driver_ctx
	{
	private:

		HANDLE handle;
		std::uint32_t pid;

		std::uint64_t send_request(dumpr::request_t request);
	public:
		driver_ctx(std::uint32_t pid);

		std::uint64_t base_address();

		void memcpy(void* dest, std::uint64_t src, std::size_t size);

		template <class T>
		inline T read(std::uint64_t address)
		{
			T buffer;

			dumpr::request_t request;
			request.type = dumpr::request_type_e::read;
			request.pid = this->pid;
			request.address = address;
			request.buffer = std::uint64_t(&buffer);
			request.size = sizeof(T);

			this->send_request(request);

			return buffer;
		}

		template <class T>
		inline void write(std::uint64_t address, T value)
		{
			dumpr::request_t request;
			request.type = dumpr::request_type_e::write;
			request.pid = this->pid;
			request.address = address;
			request.buffer = std::uintptr_t(&value);
			request.size = sizeof(T);

			this->send_request(request);
		}
	};

	std::uint32_t find_process(const char* name);
}