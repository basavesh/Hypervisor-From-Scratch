// MyHypervisorApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include <iostream>  
#include <vector>  
#include <bitset>  
#include <array>  
#include <string>  
#include <intrin.h>  
#include <strsafe.h>

using namespace std;

// IOCTL Codes and Its meanings

//
// Device type           -- in the "User Defined" range."
//
#define SIOCTL_TYPE 40000
//
// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
//
#define IOCTL_SIOCTL_METHOD_IN_DIRECT \
    CTL_CODE( SIOCTL_TYPE, 0x900, METHOD_IN_DIRECT, FILE_ANY_ACCESS  )

#define IOCTL_SIOCTL_METHOD_OUT_DIRECT \
    CTL_CODE( SIOCTL_TYPE, 0x901, METHOD_OUT_DIRECT , FILE_ANY_ACCESS  )

#define IOCTL_SIOCTL_METHOD_BUFFERED \
    CTL_CODE( SIOCTL_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS  )

#define IOCTL_SIOCTL_METHOD_NEITHER \
    CTL_CODE( SIOCTL_TYPE, 0x903, METHOD_NEITHER , FILE_ANY_ACCESS  )

string GetCpuID()
{
	//Initialize used variables
	char SysType[13]; //Array consisting of 13 single bytes/characters
	string CpuID; //The string that will be used to add all the characters to
				  //Starting coding in assembly language
	_asm
	{
		//Execute CPUID with EAX = 0 to get the CPU producer
		XOR EAX, EAX
		CPUID
		//MOV EBX to EAX and get the characters one by one by using shift out right bitwise operation.
		MOV EAX, EBX
		MOV SysType[0], al
		MOV SysType[1], ah
		SHR EAX, 16
		MOV SysType[2], al
		MOV SysType[3], ah
		//Get the second part the same way but these values are stored in EDX
		MOV EAX, EDX
		MOV SysType[4], al
		MOV SysType[5], ah
		SHR EAX, 16
		MOV SysType[6], al
		MOV SysType[7], ah
		//Get the third part
		MOV EAX, ECX
		MOV SysType[8], al
		MOV SysType[9], ah
		SHR EAX, 16
		MOV SysType[10], al
		MOV SysType[11], ah
		MOV SysType[12], 00
	}
	CpuID.assign(SysType, 12);
	return CpuID;
}


bool VMX_Support_Detection()
{

	bool VMX = false;
	__asm {
		xor    eax, eax
		inc    eax
		cpuid
		bt     ecx, 0x5
		jc     VMXSupport
		VMXNotSupport :
		jmp     NopInstr
			VMXSupport :
		mov    VMX, 0x1
			NopInstr :
			nop
	}

	return VMX;
}

void PrintAppearance() {
	printf("\n"


		"    _   _                             _                  _____                      ____                 _       _     \n"
		"   | | | |_   _ _ __   ___ _ ____   _(_)___  ___  _ __  |  ___| __ ___  _ __ ___   / ___|  ___ _ __ __ _| |_ ___| |__  \n"
		"   | |_| | | | | '_ \\ / _ \\ '__\\ \\ / / / __|/ _ \\| '__| | |_ | '__/ _ \\| '_ ` _ \\  \\___ \\ / __| '__/ _` | __/ __| '_ \\ \n"
		"   |  _  | |_| | |_) |  __/ |   \\ V /| \\__ \\ (_) | |    |  _|| | | (_) | | | | | |  ___) | (__| | | (_| | || (__| | | |\n"
		"   |_| |_|\\__, | .__/ \\___|_|    \\_/ |_|___/\\___/|_|    |_|  |_|  \\___/|_| |_| |_| |____/ \\___|_|  \\__,_|\\__\\___|_| |_|\n"
		"          |___/|_|                                                                                                     \n"



		"\n\n");
}

int main()
{
	PrintAppearance();
	string CpuID;
	CpuID = GetCpuID();
	cout << "[*] The CPU Vendor is : " << CpuID << endl;
	if (CpuID == "GenuineIntel")
	{
		cout << "[*] The Processor virtualization technology is VT-x. \n";
	}
	else
	{
		cout << "[*] This program is not designed to run in a non-VT-x environemnt !\n";
		return 1;
	}

	if (VMX_Support_Detection())
	{
		cout << "[*] VMX Operation is supported by your processor .\n";
	}
	else
	{
		cout << "[*] VMX Operation is not supported by your processor .\n";
		return 1;
	}


	HANDLE handle = CreateFile("\\\\.\\MyHypervisorDevice",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE,
		NULL, /// lpSecurityAttirbutes
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL |
		FILE_FLAG_OVERLAPPED,
		NULL); /// lpTemplateFile 

	if (handle == INVALID_HANDLE_VALUE)
	{
		DWORD errNum = GetLastError();
		printf("[*] CreateFile failed : %d\n", errNum);
		return 1;

	}
	/**

	char OutputBuffer[1000];
	char InputBuffer[1000];
	ULONG bytesReturned;
	BOOL Result;

	//
	// Performing METHOD_BUFFERED
	//

	StringCbCopy(InputBuffer, sizeof(InputBuffer),
		"This String is from User Application; using METHOD_BUFFERED");

	printf("\nCalling DeviceIoControl METHOD_BUFFERED:\n");

	memset(OutputBuffer, 0, sizeof(OutputBuffer));

	Result = DeviceIoControl(handle,
		(DWORD)IOCTL_SIOCTL_METHOD_BUFFERED,
		&InputBuffer,
		(DWORD)strlen(InputBuffer) + 1,
		&OutputBuffer,
		sizeof(OutputBuffer),
		&bytesReturned,
		NULL
	);

	if (!Result)
	{
		printf("Error in DeviceIoControl : %d", GetLastError());
		return 1;

	}
	printf("    OutBuffer (%d): %s\n", bytesReturned, OutputBuffer);

	//
	// Performing METHOD_NIETHER
	//

	printf("\nCalling DeviceIoControl METHOD_NEITHER\n");

	StringCbCopy(InputBuffer, sizeof(InputBuffer),
		"This String is from User Application; using METHOD_NEITHER");
	memset(OutputBuffer, 0, sizeof(OutputBuffer));

	Result = DeviceIoControl(handle,
		(DWORD)IOCTL_SIOCTL_METHOD_NEITHER,
		&InputBuffer,
		(DWORD)strlen(InputBuffer) + 1,
		&OutputBuffer,
		sizeof(OutputBuffer),
		&bytesReturned,
		NULL
	);

	if (!Result)
	{
		printf("Error in DeviceIoControl : %d\n", GetLastError());
		return 1;

	}

	printf("    OutBuffer (%d): %s\n", bytesReturned, OutputBuffer);

	//
	// Performing METHOD_IN_DIRECT
	//

	printf("\nCalling DeviceIoControl METHOD_IN_DIRECT\n");

	StringCbCopy(InputBuffer, sizeof(InputBuffer),
		"This String is from User Application; using METHOD_IN_DIRECT");
	StringCbCopy(OutputBuffer, sizeof(OutputBuffer),
		"This String is from User Application in OutBuffer; using METHOD_IN_DIRECT");

	Result = DeviceIoControl(handle,
		(DWORD)IOCTL_SIOCTL_METHOD_IN_DIRECT,
		&InputBuffer,
		(DWORD)strlen(InputBuffer) + 1,
		&OutputBuffer,
		sizeof(OutputBuffer),
		&bytesReturned,
		NULL
	);

	if (!Result)
	{
		printf("Error in DeviceIoControl : %d", GetLastError());
		return 1;
	}

	printf("    Number of bytes transfered from OutBuffer: %d\n",
		bytesReturned);

	//
	// Performing METHOD_OUT_DIRECT
	//

	printf("\nCalling DeviceIoControl METHOD_OUT_DIRECT\n");
	StringCbCopy(InputBuffer, sizeof(InputBuffer),
		"This String is from User Application; using METHOD_OUT_DIRECT");
	memset(OutputBuffer, 0, sizeof(OutputBuffer));
	Result = DeviceIoControl(handle,
		(DWORD)IOCTL_SIOCTL_METHOD_OUT_DIRECT,
		&InputBuffer,
		(DWORD)strlen(InputBuffer) + 1,
		&OutputBuffer,
		sizeof(OutputBuffer),
		&bytesReturned,
		NULL
	);

	if (!Result)
	{
		printf("Error in DeviceIoControl : %d", GetLastError());
		return 1;
	}

	printf("    OutBuffer (%d): %s\n", bytesReturned, OutputBuffer);

	CloseHandle(handle);


	*/

	CloseHandle(handle);
	_getch();
	return 0;
}

