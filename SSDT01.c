#include "SSDT01.h"


//=====================================================================================//
//Name: ULONG ValidateProcessNeedHide()											       //
//                                                                                     //
//Descripion: ���� uPID �����������б��е�����������ý����������б��в����ڣ��򷵻� -1		   //
//            				                            						       //
//=====================================================================================//
ULONG ValidateProcessNeedHide(ULONG uPID)
{
	ULONG i = 0;

	if(uPID == 0)
	{
		return -1;
	}

	for(i=0; i<g_currHideArrayLen && i<MAX_PROCESS_ARRARY_LENGTH; i++)
	{
		if(g_PIDHideArray[i] == uPID)
		{
			return i;
		}
	}
	return -1;
}


//=====================================================================================//
//Name: ULONG ValidateProcessNeedProtect()										       //
//                                                                                     //
//Descripion: ���� uPID �����ڱ����б��е�����������ý����ڱ����б��в����ڣ��򷵻� -1		   //
//            				                            						       //
//=====================================================================================//
ULONG ValidateProcessNeedProtect(ULONG uPID)
{
	ULONG i = 0;

	if(uPID == 0)
	{
		return -1;
	}

	for(i=0; i<g_currProtectArrayLen && i<MAX_PROCESS_ARRARY_LENGTH;i++)
	{
		if(g_PIDProtectArray[i] == uPID)
		{
			return i;
		}
	}
	return -1;
}


//=====================================================================================//
//Name: ULONG InsertHideProcess()												       //
//                                                                                     //
//Descripion: �ڽ��������б��в����µĽ��� ID											   //
//            				                            						       //
//=====================================================================================//
ULONG InsertHideProcess(ULONG uPID)
{
	if(ValidateProcessNeedHide(uPID) == -1 && g_currHideArrayLen < MAX_PROCESS_ARRARY_LENGTH)
	{
		g_PIDHideArray[g_currHideArrayLen++] = uPID;

		return TRUE;
	}

	return FALSE;
}


//=====================================================================================//
//Name: ULONG RemoveHideProcess()												       //
//                                                                                     //
//Descripion: �ӽ��������б����Ƴ����� ID												   //
//            				                            						       //
//=====================================================================================//
ULONG RemoveHideProcess(ULONG uPID)
{
	ULONG uIndex = ValidateProcessNeedHide(uPID);
	if(uIndex != -1)
	{
		g_PIDHideArray[uIndex] = g_PIDHideArray[g_currHideArrayLen--];

		return TRUE;
	}
	return FALSE;
}


//=====================================================================================//
//Name: ULONG InsertProtectProcess()											       //
//                                                                                     //
//Descripion: �ڽ��̱����б��в����µĽ��� ID											   //
//            				                            						       //
//=====================================================================================//
ULONG InsertProtectProcess(ULONG uPID)
{
	if(ValidateProcessNeedProtect(uPID) == -1 && g_currProtectArrayLen < MAX_PROCESS_ARRARY_LENGTH)
	{
		g_PIDProtectArray[g_currProtectArrayLen++] = uPID;

		return TRUE;
	}
	return FALSE;
}


//=====================================================================================//
//Name: ULONG RemoveProtectProcess()											       //
//                                                                                     //
//Descripion: �ڽ��̱����б����Ƴ�һ������ ID											   //
//            				                            						       //
//=====================================================================================//
ULONG RemoveProtectProcess(ULONG uPID)
{
	ULONG uIndex = ValidateProcessNeedProtect(uPID);
	if(uIndex != -1)
	{
		g_PIDProtectArray[uIndex] = g_PIDProtectArray[g_currProtectArrayLen--];

		return TRUE;
	}
	return FALSE;
}


//=====================================================================================//
//Name: NTSTATUS HookNtQuerySystemInformation()									       //
//                                                                                     //
//Descripion: �Զ���� NtQuerySystemInformation������ʵ�� Hook Kernel API				   //
//            				                            						       //
//=====================================================================================//
NTSTATUS HookNtQuerySystemInformation (
	__in SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
	__in ULONG SystemInformationLength,
	__out_opt PULONG ReturnLength
	)
{
	NTSTATUS rtStatus;

	pOldNtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)oldSysServiceAddr[SYSCALL_INDEX(ZwQuerySystemInformation)];

	rtStatus = pOldNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
	if(NT_SUCCESS(rtStatus))
	{
		if(SystemProcessInformation == SystemInformationClass)
		{
			PSYSTEM_PROCESS_INFORMATION pPrevProcessInfo = NULL;
			PSYSTEM_PROCESS_INFORMATION pCurrProcessInfo = (PSYSTEM_PROCESS_INFORMATION)SystemInformation; 

			while(pCurrProcessInfo != NULL)
			{
				//��ȡ��ǰ������ SYSTEM_PROCESS_INFORMATION �ڵ�Ľ������ƺͽ��� ID
				ULONG uPID = (ULONG)pCurrProcessInfo->UniqueProcessId;
				UNICODE_STRING strTmpProcessName = pCurrProcessInfo->ImageName;

				//�жϵ�ǰ��������������Ƿ�Ϊ��Ҫ���صĽ���
				if(ValidateProcessNeedHide(uPID) != -1)
				{
					if(pPrevProcessInfo)
					{
						if(pCurrProcessInfo->NextEntryOffset)
						{
							//����ǰ�������(��Ҫ���صĽ���)�� SystemInformation ��ժ��(��������ƫ��ָ��ʵ��)
							pPrevProcessInfo->NextEntryOffset += pCurrProcessInfo->NextEntryOffset;
						}
						else
						{
							//˵����ǰҪ���ص���������ǽ��������е����һ��
							pPrevProcessInfo->NextEntryOffset = 0;
						}
					}
					else
					{
						//��һ���������ý��̾�����Ҫ���صĽ���
						if(pCurrProcessInfo->NextEntryOffset)
						{
							(PCHAR)SystemInformation += pCurrProcessInfo->NextEntryOffset;
						}
						else
						{
							SystemInformation = NULL;
						}
					}
				}

				//������һ�� SYSTEM_PROCESS_INFORMATION �ڵ�
				pPrevProcessInfo = pCurrProcessInfo;

				//��������
				if(pCurrProcessInfo->NextEntryOffset)
				{
					pCurrProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(((PCHAR)pCurrProcessInfo) + pCurrProcessInfo->NextEntryOffset);
				}
				else
				{
					pCurrProcessInfo = NULL;
				}
			}
		}
	}
	return rtStatus;
}


//=====================================================================================//
//Name: NTSTATUS HookNtTerminateProcess()										       //
//                                                                                     //
//Descripion: �Զ���� NtTerminateProcess������ʵ�� Hook Kernel API					   //
//            				                            						       //
//=====================================================================================//
NTSTATUS HookNtTerminateProcess(
	__in_opt HANDLE ProcessHandle,
	__in NTSTATUS ExitStatus
	)
{
	ULONG uPID;
	NTSTATUS rtStatus;
	PCHAR pStrProcName;
	PEPROCESS pEProcess;
	ANSI_STRING strProcName;

	//ͨ�����̾������øý�������Ӧ�� FileObject �������������ǽ��̶�����Ȼ��õ��� EPROCESS ����
	rtStatus = ObReferenceObjectByHandle(ProcessHandle, FILE_READ_DATA, NULL, KernelMode, &pEProcess, NULL);
	if(!NT_SUCCESS(rtStatus))
	{
		return rtStatus;
	}

	//���� SSDT ��ԭ���� NtTerminateProcess ��ַ
	pOldNtTerminateProcess = (NTTERMINATEPROCESS)oldSysServiceAddr[SYSCALL_INDEX(ZwTerminateProcess)];

	//ͨ���ú������Ի�ȡ���������ƺͽ��� ID���ú������ں���ʵ���ǵ�����(�� WRK �п��Կ���)
	//���� ntddk.h �в�û�е�����������Ҫ�Լ���������ʹ��
	uPID = (ULONG)PsGetProcessId(pEProcess);
	pStrProcName = (PCHAR)PsGetProcessImageFileName(pEProcess);

	//ͨ������������ʼ��һ�� ASCII �ַ���
	RtlInitAnsiString(&strProcName, pStrProcName);

	if(ValidateProcessNeedProtect(uPID) != -1)
	{
		//ȷ�������߽����ܹ�����(������Ҫ��ָ taskmgr.exe)
		if(uPID != (ULONG)PsGetProcessId(PsGetCurrentProcess()))
		{
			//����ý������������ĵĽ��̵Ļ����򷵻�Ȩ�޲������쳣����
			return STATUS_ACCESS_DENIED;
		}
	}

	//���ڷǱ����Ľ��̿���ֱ�ӵ���ԭ�� SSDT �е� NtTerminateProcess ����������
	rtStatus = pOldNtTerminateProcess(ProcessHandle, ExitStatus);

	return rtStatus;
}


//=====================================================================================//
//Name: NTSTATUS DriverEntry()													       //
//                                                                                     //
//Descripion: ��ں����������������� SSDT �Լ���װ Kernel API Hook						   //
//            				                            						       //
//=====================================================================================//
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING  pRegistryPath)
{
	ULONG i;
	NTSTATUS status;
	UNICODE_STRING strDeviceName;
	UNICODE_STRING strSymbolLinkName;
	PDEVICE_OBJECT pDeviceObject;

	pDeviceObject = NULL;

	RtlInitUnicodeString(&strDeviceName, DEVICE_NAME_PROCESS);
	RtlInitUnicodeString(&strSymbolLinkName, SYMBOLINK_NAME_PROCESS);

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = SSDT01GeneralDispatcher;
	}

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = SSDT01CreateDispatcher;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = SSDT01CloseDispatcher;
	pDriverObject->MajorFunction[IRP_MJ_READ] = SSDT01ReadDispatcher;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = SSDT01WriteDispatcher;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SSDT01DeviceIoControlDispatcher;
	
	pDriverObject->DriverUnload = SSDT01DriverUnload;

	status = IoCreateDevice(pDriverObject, 0, &strDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	if (!pDeviceObject)
	{
		return STATUS_UNEXPECTED_IO_ERROR;
	}

	//ʹ��ֱ�� IO ��д��ʽ
	pDeviceObject->Flags |= DO_DIRECT_IO;
	pDeviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
	status = IoCreateSymbolicLink(&strSymbolLinkName, &strDeviceName);

	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	
	//������Ҫ����ԭ���� SSDT ϵͳ���������������з���ĵ�ַ����Щ��ַ��Ҫ����ʵ�ֽ�� Hook
	BackupSysServicesTable();

	//��װ Hook
	InstallSysServiceHook((ULONG)ZwQuerySystemInformation, (ULONG)HookNtQuerySystemInformation);

	InstallSysServiceHook((ULONG)ZwTerminateProcess, (ULONG)HookNtTerminateProcess);

	return STATUS_SUCCESS;
}


//=====================================================================================//
//Name: void SSDT01DriverUnload()												       //
//                                                                                     //
//Descripion: ж�غ���������������� Kernel API Hook									   //
//            				                            						       //
//=====================================================================================//
void SSDT01DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING strSymbolLinkName;

	DbgPrint("In SSDT01DriverUnload !");

	RtlInitUnicodeString(&strSymbolLinkName, SYMBOLINK_NAME_PROCESS);
	IoDeleteSymbolicLink(&strSymbolLinkName);
	IoDeleteDevice(pDriverObject->DeviceObject);

	//��� Hook
	UnInstallSysServiceHook((ULONG)ZwQuerySystemInformation);


	UnInstallSysServiceHook((ULONG)ZwTerminateProcess);

	DbgPrint("Out SSDT01DriverUnload !");
}


//=====================================================================================//
//Name: NTSTATUS SSDT01CreateDispatcher()										       //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS SSDT01CreateDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


//=====================================================================================//
//Name: NTSTATUS SSDT01GeneralDispatcher()										       //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS SSDT01CloseDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


//=====================================================================================//
//Name: NTSTATUS SSDT01GeneralDispatcher()										       //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS SSDT01GeneralDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return pIrp->IoStatus.Status;
}


//=====================================================================================//
//Name: NTSTATUS SSDT01ReadDispatcher()											       //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS SSDT01ReadDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	NTSTATUS rtStatus;

	rtStatus = STATUS_NOT_SUPPORTED;

	return rtStatus;
}


//=====================================================================================//
//Name: NTSTATUS SSDT01WriteDispatcher()										       //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS SSDT01WriteDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	NTSTATUS rtStatus;

	rtStatus = STATUS_NOT_SUPPORTED;

	return rtStatus;
}


//=====================================================================================//
//Name: NTSTATUS SSDT01DeviceIoControlDispatcher()								       //
//                                                                                     //
//Descripion: �ַ�����																   //
//            				                            						       //
//=====================================================================================//
NTSTATUS SSDT01DeviceIoControlDispatcher(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	NTSTATUS rtStatus;

	ULONG uPID;
	ULONG uInLen;
	ULONG uOutLen;
	ULONG uCtrlCode;

	PCHAR pInBuffer;

	PIO_STACK_LOCATION pStack;	

	uPID = 0;
	rtStatus = STATUS_SUCCESS;
	pStack = IoGetCurrentIrpStackLocation(pIrp);

	uInLen = pStack->Parameters.DeviceIoControl.InputBufferLength;
	uOutLen = pStack->Parameters.DeviceIoControl.OutputBufferLength;
	uCtrlCode = pStack->Parameters.DeviceIoControl.IoControlCode;

	//ʹ�û�������ʽ��Ӧ�ó������ͨ��
	pInBuffer = (PCHAR)pIrp->AssociatedIrp.SystemBuffer;
	
	if(uInLen >= 4)
	{
		//stdlib.h(atol = Array To LONG)
		uPID = atol(pInBuffer);

		switch(uCtrlCode)
		{
		case IO_INSERT_PROTECT_PROCESS:
			{
				if(InsertProtectProcess(uPID) == FALSE)
				{
					rtStatus = STATUS_PROCESS_IS_TERMINATING;
				}
				break;
			}
		case IO_REMOVE_PROTECT_PROCESS:
			{
				if(RemoveProtectProcess(uPID) == FALSE)
				{
					rtStatus = STATUS_PROCESS_IS_TERMINATING;
				}
				break;
			}
		case IO_INSERT_HIDE_PROCESS:
			{
				if(InsertHideProcess(uPID) == FALSE)
				{
					rtStatus = STATUS_PROCESS_IS_TERMINATING;
				}
				break;
			}
		case IO_REMOVE_HIDE_PROCESS:
			{
				if(RemoveHideProcess(uPID) == FALSE)
				{
					rtStatus = STATUS_PROCESS_IS_TERMINATING;
				}
				break;
			}
		default:
			{
				rtStatus = STATUS_INVALID_VARIANT;
				break;
			}
		}
	}
	else
	{
		rtStatus = STATUS_INVALID_PARAMETER;
	}
	
	//�����Ϣ����Ϊ�գ������������򲻷��������Ϣ
	pIrp->IoStatus.Status = rtStatus;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return rtStatus;
}
