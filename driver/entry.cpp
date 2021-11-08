#include <includes.hpp>

UNICODE_STRING DeviceName, SymbolicLink;

enum request_type_e : int
{
	read,
	write,
	base_address
};

typedef struct _request_t {
	request_type_e type;
	int pid;
	uintptr_t address;
	uintptr_t buffer;
	size_t size;
} request_t;

NTSTATUS 
Unsupported(
	_In_ PDEVICE_OBJECT DeviceObject, 
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	DbgPrint("dumpr: got unsupported request");

	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS 
CreateClose(
	_In_ PDEVICE_OBJECT DeviceObject, 
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	DbgPrint("dumpr: got create/close request");

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS
Dispatch(
	_In_ PDEVICE_OBJECT DeviceObject,
	_Inout_ PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	std::uint64_t data = 0;
	request_t* request = reinterpret_cast<request_t*>(Irp->AssociatedIrp.SystemBuffer);

	DbgPrint("dumpr: request at %p (%i)", request, request->type);

	if (request->type >= 0 && request->type <= 2)
	{
		switch (request->type)
		{
		case request_type_e::read: {
			if (request->pid && request->address && request->size && request->buffer)
			{
				PEPROCESS process;
				if (PsLookupProcessByProcessId((void*)request->pid, &process) == STATUS_SUCCESS)
				{
					size_t read;
					MmCopyVirtualMemory(process, PVOID(request->address), PsGetCurrentProcess(), PVOID(request->buffer), request->size, KernelMode, &read);
					DbgPrint("dumpr: finished read operation");
				}

				uintptr_t* data_ptr = reinterpret_cast<uintptr_t*>(Irp->AssociatedIrp.SystemBuffer);

				*data_ptr = STATUS_SUCCESS;
				data = sizeof(*data_ptr);
			}
			break;
		}
		case request_type_e::write: {
			if (request->pid && request->address && request->size && request->buffer)
			{
				PEPROCESS process;
				if (PsLookupProcessByProcessId((void*)request->pid, &process) == STATUS_SUCCESS)
				{
					size_t written;
					MmCopyVirtualMemory(PsGetCurrentProcess(), PVOID(request->buffer), process, PVOID(request->address), request->size, KernelMode, &written);
					DbgPrint("dumpr: finished write operation");
				}

				uintptr_t* data_ptr = reinterpret_cast<uintptr_t*>(Irp->AssociatedIrp.SystemBuffer);

				*data_ptr = STATUS_SUCCESS;
				data = sizeof(*data_ptr);
			}
			break;
		}
		case request_type_e::base_address: {
			if (request->pid)
			{
				PEPROCESS process;
				if (PsLookupProcessByProcessId((void*)request->pid, &process) == STATUS_SUCCESS)
				{
					uintptr_t* data_ptr = reinterpret_cast<uintptr_t*>(Irp->AssociatedIrp.SystemBuffer);

					*data_ptr = (uintptr_t)PsGetProcessSectionBaseAddress(process);
					data = sizeof(*data_ptr);
				}
			}

			break;
		}
		}
	}

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = data;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

void
DriverUnload(
	_In_ PDRIVER_OBJECT DriverObject
)
{
	DbgPrint("dumpr: DriverUnload called");
	IoDeleteSymbolicLink(&SymbolicLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS
Initialize(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS Status = STATUS_FAILED_DRIVER_ENTRY;

	PDEVICE_OBJECT DeviceObject;

	RtlInitUnicodeString(&DeviceName, L"\\Device\\DaDumpr");
	RtlInitUnicodeString(&SymbolicLink, L"\\DosDevices\\DaDumpr");

	Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(Status)) return Status;

	DriverObject->DriverUnload = &DriverUnload;
	
	Status = IoCreateSymbolicLink(&SymbolicLink, &DeviceName);
	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(DeviceObject);
		return Status;
	}

	DeviceObject->Flags |= DO_BUFFERED_IO;

	for (std::uint32_t i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = &Unsupported;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = &CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = &CreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &Dispatch;
	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return Status;
}

NTSTATUS 
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS Status = STATUS_FAILED_DRIVER_ENTRY;

	DbgPrint("dumpr: DriverEntry called");

	Status = IoCreateDriver(NULL, &Initialize);
	// uh yes that's DriverEntry

	return Status;
}