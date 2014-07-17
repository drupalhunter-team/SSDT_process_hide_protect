#ifndef __SSDT_HOOK_H__
#define __SSDT_HOOK_H__


#ifdef __cplusplus
extern "C" 
{
#endif

#include <ntddk.h>

#ifdef __cplusplus
}
#endif


//���� SSDT(ϵͳ����������) �з�������������Ŀ
//���ﶨ��Ϊ 1024 ����ʵ������ XP SP3 �� 0x0128 ��
#define MAX_SYSTEM_SERVICE_NUMBER 1024


//=====================================================================================//
//Name: KSYSTEM_SERVICE_TABLE �� KSERVICE_TABLE_DESCRIPTOR					           //
//                                                                                     //
//Descripion: �������� SSDT �ṹ												  	       //
//            				                            						       //
//=====================================================================================//
typedef struct _KSYSTEM_SERVICE_TABLE
{
	PULONG  ServiceTableBase;					// SSDT (System Service Dispatch Table)�Ļ���ַ
	PULONG  ServiceCounterTableBase;			// ���� checked builds, ���� SSDT ��ÿ�����񱻵��õĴ���
	ULONG   NumberOfService;					// �������ĸ���, NumberOfService * 4 ����������ַ��Ĵ�С
	ULONG   ParamTableBase;						// SSPT(System Service Parameter Table)�Ļ���ַ

} KSYSTEM_SERVICE_TABLE, *PKSYSTEM_SERVICE_TABLE;


typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
	KSYSTEM_SERVICE_TABLE   ntoskrnl;			// ntoskrnl.exe �ķ�����
	KSYSTEM_SERVICE_TABLE   win32k;				// win32k.sys �ķ�����(GDI32.dll/User32.dll ���ں�֧��)
	KSYSTEM_SERVICE_TABLE   notUsed1;
	KSYSTEM_SERVICE_TABLE   notUsed2;

} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;


//������ ntoskrnl.exe �������� SSDT
extern PKSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable;


//���� Zw_ServiceFunction ��ȡ Zw_ServiceFunction �� SSDT ������Ӧ�ķ����������
#define SYSCALL_INDEX(ServiceFunction) (*(PULONG)((PUCHAR)ServiceFunction + 1))


//���� Zw_ServiceFunction ����÷����� SSDT �е������ţ�
//Ȼ����ͨ��������������ȡ Nt_ServiceFunction�ĵ�ַ
#define SYSCALL_FUNCTION(ServiceFunction) KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[SYSCALL_INDEX(ServiceFunction)]


//�������� SSDT �����еľɵķ������ĵ�ַ
ULONG oldSysServiceAddr[MAX_SYSTEM_SERVICE_NUMBER];

//��ֹд�뱣����Ҳ���ǻָ���ֻ��
VOID DisableWriteProtect(ULONG oldAttr);

//����д�뱣����Ҳ��������Ϊ��д
VOID EnableWriteProtect(PULONG pOldAttr);

//���� SSDT ������ϵͳ����ĵ�ַ
VOID BackupSysServicesTable();

//��װ Hook
NTSTATUS InstallSysServiceHook(ULONG oldService, ULONG newService);

//��� Hook
NTSTATUS UnInstallSysServiceHook(ULONG oldService);


#endif