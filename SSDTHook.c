#include "SSDTHook.h"


//=====================================================================================//
//Name: VOID DisableWriteProtect()												       //
//                                                                                     //
//Descripion: ����ȥ���ڴ�Ŀ�д���ԣ��Ӷ�ʵ���ڴ�ֻ��								  	       //
//            				                            						       //
//=====================================================================================//
VOID DisableWriteProtect(ULONG oldAttr)
{
	_asm
	{
		mov eax, oldAttr
		mov cr0, eax
		sti;
	}
}


//=====================================================================================//
//Name: VOID EnableWriteProtect()												       //
//                                                                                     //
//Descripion: ����ȥ���ڴ��ֻ���������Ӷ�ʵ�ֿ���д�ڴ�							  	       //
//            				                            						       //
//=====================================================================================//
VOID EnableWriteProtect(PULONG pOldAttr)
{
	ULONG uAttr; 

	_asm 
	{ 
		cli;
		mov  eax, cr0; 
		mov  uAttr, eax; 
		and  eax, 0FFFEFFFFh; // CR0 16 BIT = 0 
		mov  cr0, eax; 
	}; 

	//����ԭ�е� CRO ���� 
	*pOldAttr = uAttr; 
}


//=====================================================================================//
//Name: VOID BackupSysServicesTable()											       //
//                                                                                     //
//Descripion: �������� SSDT ��ԭ�з���ĵ�ַ����Ϊ�����ڽ�� Hook ʱ��Ҫ��ԭ SSDT ��ԭ�е�ַ     //
//            				                            						       //
//=====================================================================================//
VOID BackupSysServicesTable()
{
	ULONG i;

	for(i = 0; (i < KeServiceDescriptorTable->ntoskrnl.NumberOfService) && (i < MAX_SYSTEM_SERVICE_NUMBER); i++)
	{
		oldSysServiceAddr[i] = KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[i];
		//oldSysServiceAddr[i] = *(PULONG)((ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTableBase + 4 * i);

		KdPrint(("\nBackupSysServicesTable - Function Information { Number: 0x%04X , Address: %08X}", i, oldSysServiceAddr[i]));
	}
}


//=====================================================================================//
//Name: NTSTATUS InstallSysServiceHook()										       //
//                                                                                     //
//Descripion: ʵ�� Hook �İ�װ����Ҫ���� SSDT ���� newService ���滻�� oldService		       //
//            				                            						       //
//=====================================================================================//
NTSTATUS InstallSysServiceHook(ULONG oldService, ULONG newService)
{
	ULONG uOldAttr = 0;

	EnableWriteProtect(&uOldAttr);

	SYSCALL_FUNCTION(oldService) = newService;
	//KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[SYSCALL_INDEX(oldService)] = newService;

	DisableWriteProtect(uOldAttr);

	return STATUS_SUCCESS;
}


//=====================================================================================//
//Name: NTSTATUS UnInstallSysServiceHook()										       //
//                                                                                     //
//Descripion: ʵ�� Hook �Ľ������Ҫ���� SSDT ���ñ����µķ����ַ���滻�� oldService		   //
//            				                            						       //
//=====================================================================================//
NTSTATUS UnInstallSysServiceHook(ULONG oldService)
{
	ULONG uOldAttr = 0;

	EnableWriteProtect(&uOldAttr);

	SYSCALL_FUNCTION(oldService) = oldSysServiceAddr[SYSCALL_INDEX(oldService)];
	//KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[SYSCALL_INDEX(oldService)] = oldSysServiceAddr[SYSCALL_INDEX(oldService)];

	DisableWriteProtect(uOldAttr);

	return STATUS_SUCCESS;
}
