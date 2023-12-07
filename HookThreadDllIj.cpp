#include <Windows.h>
#include <tlhelp32.h>
#include <iostream>

typedef struct _PARAM
{
	WCHAR dllPath[MAX_PATH];
	DWORD dwThreadId;
} PARAM;

HMODULE g_hModule;
HHOOK g_hHook = NULL;

DWORD GetProcessIdByName(WCHAR* szName)
{
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!Process32First(hSnapshot, &pe)) return NULL;

	do
	{
		if (!_wcsicmp(pe.szExeFile, szName)) return pe.th32ProcessID;
	} while (Process32Next(hSnapshot, &pe));

	CloseHandle(hSnapshot);
	return NULL;
}

DWORD GetMainThreadIdByName(WCHAR* szName)
{
	DWORD processId = GetProcessIdByName(szName);
	if (!processId) return NULL;

	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (!Thread32First(hSnapshot, &te)) return NULL;

	do
	{
		if (processId == te.th32OwnerProcessID) return te.th32ThreadID;
	} while (Thread32Next(hSnapshot, &te));

	CloseHandle(hSnapshot);
	return NULL;
}

void HookByThreadId(DWORD dwThreadId, WCHAR dllPath[])
{
	if (g_hHook) return;

	HMODULE g_hModule = LoadLibrary(dllPath);
	if (!g_hModule)
	{
		std::cout << "LoadLibrary Fail..." << std::endl;
		return;
	}

	HOOKPROC pfn = (HOOKPROC)GetProcAddress(g_hModule, "MouseProc");

	g_hHook = SetWindowsHookEx(WH_MOUSE, pfn, g_hModule, dwThreadId);
	std::cout << "Hook" << std::endl;

	// 处理消息,否则程序会卡死
	MSG msg;
	while (1)
	{
		if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
			Sleep(200);
	}
}

void UnHook()
{
	if (g_hHook)
	{
		UnhookWindowsHookEx(g_hHook);
		g_hHook = NULL;
		FreeLibrary(g_hModule);
		std::cout << "UnHook" << std::endl;
	}
}

DWORD WINAPI ThreadProc(PVOID pParam)
{
	PARAM param = *(PARAM*)pParam;
	HookByThreadId(param.dwThreadId, param.dllPath);

	return 0;
}

int main(int argc, char* argv[])
{
	PARAM pParam;
	swprintf(pParam.dllPath, MAX_PATH, L"%hs", argv[1]);

	// Get ThreadId By argv[2]
	WCHAR szName[50];
	swprintf(szName, 50, L"%hs", argv[2]);
	DWORD dwThreadId = GetMainThreadIdByName(szName);
	if (!dwThreadId)
	{
		std::cout << "ThreadId IS NULL!" << std::endl;
		return 0;
	}
	pParam.dwThreadId = dwThreadId;

	CloseHandle(CreateThread(NULL, 0, ThreadProc, &pParam, 0, NULL));

	system("pause");
	UnHook();

	return 0;
}