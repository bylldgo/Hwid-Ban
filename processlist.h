#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <iostream>

class ProcessList {
public:
    static void TerminateProcesses() {
        std::vector<std::wstring> processList = {
            L"http toolkit.exe",
            L"httptoolkit.exe",
            L"httpdebuggerui.exe",
            L"wireshark.exe",
            L"fiddler.exe",
            L"charles.exe",
            L"HTTPDebuggerPro.exe",
            L"debugger.exe",
            L"processhacker.exe",
            L"ida64.exe",
            L"ollydbg.exe",
            L"x96dbg.exe",
            L"x64dbg.exe",
            L"x32dbg.exe",
        };

        for (const auto& process : processList) {
            TerminateProcessByName(process.c_str());
        }
    }

private:
    static void TerminateProcessByName(const wchar_t* processName) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return;
        }

        if (!Process32First(hSnapshot, &pe32)) {
            CloseHandle(hSnapshot);
            return;
        }

        do {
            if (_wcsicmp(pe32.szExeFile, processName) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                if (hProcess != NULL) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    std::wcout << L"Terminated process: " << processName << std::endl;
                }
            }
        } while (Process32Next(hSnapshot, &pe32));

        CloseHandle(hSnapshot);
    }
};