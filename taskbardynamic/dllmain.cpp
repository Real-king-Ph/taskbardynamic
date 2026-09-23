// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "PrimoCache.h"


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		// 进程退出：通知 PrimoCache 采样线程停止（不等待，避免在 DllMain 中阻塞）
		StopPrimoCacheMonitor();
		break;
	}
	return TRUE;
}

