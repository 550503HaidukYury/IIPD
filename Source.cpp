#include <cstdio>
#include <stdio.h>
#include <windows.h>
#include <SetupAPI.h>
#include <devguid.h>
#include <regstr.h>
#include <locale.h>
#include <iostream>
#include <fstream>
#include <string>
int main()
{
	setlocale(LC_ALL, "RUS");
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i;
	// Create a HDEVINFO with all present devices.
	hDevInfo = SetupDiGetClassDevs(NULL,
		REGSTR_KEY_PCIENUM, // Enumerator
		0,
		DIGCF_PRESENT | DIGCF_ALLCLASSES);

	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		// Insert error handling here.
		return 1;
	}
	// Enumerate through all devices in Set.
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i,
		&DeviceInfoData); i++)
	{
		DWORD DataT;
		LPTSTR buffer = NULL;
		DWORD buffersize = 0;
		//
		// Call function with null to begin with, 
		// then use the returned buffer size (doubled)
		// to Alloc the buffer. Keep calling until
		// success or an unknown failure.
		//
		//  Double the returned buffersize to correct
		//  for underlying legacy CM functions that 
		//  return an incorrect buffersize value on 
		//  DBCS/MBCS systems.
		// 
		wchar_t *hardwareID;
		/*// First get requiredLength
		SetupDiGetDeviceRegistryProperty(deviceInfoList, &deviceInfoData, SPDRP_HARDWAREID, NULL, NULL, 0, &requiredLength);

		hardwareID = (wchar_t*)(new char[requiredLength]());

		// Second call to populate hardwareID
		SetupDiGetDeviceRegistryProperty(deviceInfoList, &deviceInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)hardwareID, requiredLength, NULL);
		*/
		while (!SetupDiGetDeviceRegistryProperty(
			hDevInfo,
			&DeviceInfoData,
			SPDRP_DEVICEDESC|SPDRP_HARDWAREID,
			&DataT,
			(PBYTE)buffer,
			buffersize,
			&buffersize))
		{
			if (GetLastError() ==
				ERROR_INSUFFICIENT_BUFFER)
			{
				// Change the buffer size.
				if (buffer) LocalFree(buffer);
				// Double the size to avoid problems on 
				// W2k MBCS systems per KB 888609. 
				buffer = (LPTSTR)LocalAlloc(LPTR, buffersize * 2);
			}
			else
			{
				// Insert error handling here.
				break;
			}
		}

		std::string deviceID;
		std::string vendorID;
		std::string Str = buffer;
		vendorID=Str.substr(8, 4);
		deviceID = Str.substr(17, 4);

		std::ifstream file("pci.ids");
		std::string line;
		for (;;){
			std::getline(file, line);
			if (line.length() > 5)
			{
				if (line.substr(0, 4) == vendorID)
				{
					std::cout << "vendor: " + vendorID + " ";
					std::cout << line.substr(4, line.length() - 4) << std::endl;
					for (;;)
					{
						if (std::getline(file, line))
						if (line.length() > 5)
						{
							if (line.substr(1, 4) == deviceID){

								std::cout << "\t device: " + deviceID + " ";
								std::cout << line.substr(5, line.length() - 5) << std::endl;
								file.close();
								break;
							}
						}
					}break;
				}
			}
		}
		if (buffer) LocalFree(buffer);
	}
	if (GetLastError() != NO_ERROR &&
		GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		// Insert error handling here.
		return 1;
	}
	//  Cleanup
	SetupDiDestroyDeviceInfoList(hDevInfo);
	system("pause");
	getchar();
	return 0;
}