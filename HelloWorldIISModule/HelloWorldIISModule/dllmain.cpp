// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

class HelloWorldIISModule : public CGlobalModule
{
public:
    HelloWorldIISModule()
    {
        _eventLog = RegisterEventSource(NULL, L"HelloWorldIISModule");
        WriteEvent(L"HelloWorldIISModule()");
    }

    ~HelloWorldIISModule()
    {
        WriteEvent(L"~HelloWorldIISModule()");
        DeregisterEventSource(_eventLog);
    }

    VOID Terminate()
    {
        WriteEvent(L"~Terminate()");
        delete this;
    }

    GLOBAL_NOTIFICATION_STATUS OnGlobalPreBeginRequest(IN IPreBeginRequestProvider * provider)
    {
        WriteEvent(L"OnGlobalPreBeginRequest()");
        return GL_NOTIFICATION_CONTINUE;
    }
private:
    HANDLE _eventLog;

    BOOL WriteEvent(LPCWSTR message)
    {
        return ReportEvent(_eventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &message, NULL);
    }
};

HRESULT __stdcall RegisterModule(DWORD serverVersion, IHttpModuleRegistrationInfo * moduleInfo, IHttpServer * globalInfo)
{
    LPCWSTR message = L"Hello from RegisterModule";
    
    HANDLE eventLog = RegisterEventSource(NULL, L"RegisterModule");
    ReportEvent(eventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &message, NULL);
    DeregisterEventSource(eventLog);

    HelloWorldIISModule * module = new HelloWorldIISModule;
    return moduleInfo->SetGlobalNotifications(module, GL_PRE_BEGIN_REQUEST);
}
