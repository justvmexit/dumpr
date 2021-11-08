#include <includes.hpp>

std::uint32_t main(std::uint32_t argc, char** argv)
{	
	if (argc < 2)
	{
		std::printf("dumper: proper usage\n\t%s processname.exe", argv[0]);
		return 0;
	}

	std::uint32_t pid = dumpr::find_process(argv[1]);
	dumpr::driver_ctx driver = dumpr::driver_ctx(pid);

	std::printf("%s: process id (%i)\n", argv[1], pid);
	std::printf("%s: base address (%p)\n", argv[1], driver.base_address());

	IMAGE_DOS_HEADER dos = driver.read<IMAGE_DOS_HEADER>(driver.base_address());
	if (dos.e_magic != IMAGE_DOS_SIGNATURE)
	{
		std::printf("%s: invalid dos header\n", argv[1]);
		return 0;
	}

	IMAGE_NT_HEADERS nt = driver.read<IMAGE_NT_HEADERS>(driver.base_address() + dos.e_lfanew);
	if (nt.Signature != IMAGE_NT_SIGNATURE)
	{
		std::printf("%s: invalid nt header\n", argv[1]);
		return 0;
	}

	std::printf("%s: fetched nt headers\n", argv[1]);

	char* buffer = reinterpret_cast<char*>(malloc(nt.OptionalHeader.SizeOfImage));
	driver.memcpy(buffer, driver.base_address(), nt.OptionalHeader.SizeOfImage);
	PIMAGE_DOS_HEADER new_dos = reinterpret_cast<PIMAGE_DOS_HEADER>(buffer);
	if (new_dos->e_magic != IMAGE_DOS_SIGNATURE)
	{
		std::printf("%s: invalid dos header\n", argv[1]);
		return 0;
	}

	PIMAGE_NT_HEADERS new_nt = reinterpret_cast<PIMAGE_NT_HEADERS>(buffer + dos.e_lfanew);
	if (nt.Signature != IMAGE_NT_SIGNATURE)
	{
		std::printf("%s: invalid nt header\n", argv[1]);
		return 0;
	}

	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(new_nt);
	for (int i = 0; i < new_nt->FileHeader.NumberOfSections; i++, section++)
	{
		section->SizeOfRawData = section->Misc.VirtualSize;
		section->PointerToRawData = section->VirtualAddress;
	}

	std::printf("%s: fixed sections\n", argv[1]);

	char* filename = reinterpret_cast<char*>(malloc(strlen(argv[1]) + 10));
	sprintf(filename, "%s_dump.exe", argv[1]);

	std::printf("%s: writing dump file\n", argv[1]);

	std::ofstream dump(filename, std::ios::binary);
	dump.write(buffer, nt.OptionalHeader.SizeOfImage);
	dump.close();

	std::printf("%s: wrote dump file\n", argv[1]);

	free(buffer);

	return 0;
}
