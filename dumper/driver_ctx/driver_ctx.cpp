#include <driver_ctx.hpp>

std::uint64_t dumpr::driver_ctx::send_request(dumpr::request_t request)
{
	if (this->handle == INVALID_HANDLE_VALUE) return 0;

	std::uintptr_t output = 0;
	if (DeviceIoControl(this->handle, CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1000, METHOD_BUFFERED, FILE_SPECIAL_ACCESS), &request, sizeof(request), &output, sizeof(output), 0, 0)) 
		return output;

	return 0;
}

dumpr::driver_ctx::driver_ctx(std::uint32_t pid)
{
	if (OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid) != INVALID_HANDLE_VALUE)
	{
		this->pid = pid;
	}

	this->handle = CreateFileA("\\\\.\\DaDumpr", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
}

std::uint64_t dumpr::driver_ctx::base_address()
{
	dumpr::request_t request;
	request.type = dumpr::request_type_e::base_address;
	request.pid = this->pid;

	return this->send_request(request);
}

void dumpr::driver_ctx::memcpy(void* dest, std::uint64_t src, std::size_t size)
{
	dumpr::request_t request;
	request.type = dumpr::request_type_e::read;
	request.pid = this->pid;
	request.address = src;
	request.buffer = std::uint64_t(dest);
	request.size = size;

	this->send_request(request);
}

std::uint32_t dumpr::find_process(const char* name)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
	{
		while (Process32Next(snapshot, &entry))
		{
			if (!strcmp(entry.szExeFile, name))
			{
				CloseHandle(snapshot);
				return entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);

	return 0;
}
