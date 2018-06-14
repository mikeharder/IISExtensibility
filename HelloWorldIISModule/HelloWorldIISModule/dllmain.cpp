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

class HelloWorldGlobalModule : public CGlobalModule
{
public:
    HelloWorldGlobalModule()
    {
        _eventLog = RegisterEventSource(NULL, L"HelloWorldGlobalModule");
        WriteEvent(L"HelloWorldGlobalModule()");
    }

    ~HelloWorldGlobalModule()
    {
        WriteEvent(L"~HelloWorldGlobalModule()");
        DeregisterEventSource(_eventLog);
    }

    VOID Terminate()
    {
        WriteEvent(L"Terminate()");
        delete this;
    }

    GLOBAL_NOTIFICATION_STATUS OnGlobalPreBeginRequest(IN IPreBeginRequestProvider * provider)
    {
        WriteEvent(L"OnGlobalPreBeginRequest()");
        return GL_NOTIFICATION_CONTINUE;
    }
private:
    HANDLE _eventLog;

    BOOL WriteEvent(std::wstring message)
    {
        auto s = message.c_str();
        return ReportEvent(_eventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &s, NULL);
    }
};

class HelloWorldHttpModule : public CHttpModule
{
public:
    HelloWorldHttpModule()
    {
        _eventLog = RegisterEventSource(NULL, L"HelloWorldHttpModule");
        WriteEvent(L"HelloWorldHttpModule()");
    }

    ~HelloWorldHttpModule()
    {
        WriteEvent(L"~HelloWorldHttpModule()");
        DeregisterEventSource(_eventLog);
    }

    REQUEST_NOTIFICATION_STATUS OnBeginRequest(IN IHttpContext * httpContext, IN IHttpEventProvider * provider)
    {
        WriteEvent(L"OnBeginRequest()");

        IHttpResponse * httpResponse = httpContext->GetResponse();

        httpResponse->Clear();
        httpResponse->SetHeader(HttpHeaderContentType, "text/plain", (USHORT)strlen("text/plain"), TRUE);

        PCSTR buffer = "Hello World!";
        HTTP_DATA_CHUNK dataChunk;
        dataChunk.DataChunkType = HttpDataChunkFromMemory;
        DWORD bytesSent;

        dataChunk.FromMemory.pBuffer = (PVOID)buffer;
        dataChunk.FromMemory.BufferLength = (USHORT)strlen(buffer);
        HRESULT hr = httpResponse->WriteEntityChunks(&dataChunk, 1, FALSE, TRUE, &bytesSent);

        return RQ_NOTIFICATION_FINISH_REQUEST;
    }

private:
    HANDLE _eventLog;

    BOOL WriteEvent(std::wstring message)
    {
        auto s = message.c_str();
        return ReportEvent(_eventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &s, NULL);
    }
};

class HelloWorldHttpModuleFactory : public IHttpModuleFactory
{
public:
    HelloWorldHttpModuleFactory()
    {
        _eventLog = RegisterEventSource(NULL, L"HelloWorldHttpModuleFactory");
        WriteEvent(L"HelloWorldHttpModuleFactory()");
    }

    ~HelloWorldHttpModuleFactory()
    {
        WriteEvent(L"~HelloWorldHttpModuleFactory()");
        DeregisterEventSource(_eventLog);
    }

    HRESULT GetHttpModule(OUT CHttpModule ** module, IN IModuleAllocator * allocator) {
        WriteEvent(L"GetHttpModule()");
        *module = std::make_unique<HelloWorldHttpModule>().release();
        return S_OK;
    }

    VOID Terminate()
    {
        WriteEvent(L"Terminate()");
        delete this;
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
    auto message = L"Hello from RegisterModule";

    HANDLE eventLog = RegisterEventSource(NULL, L"RegisterModule");
    ReportEvent(eventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &message, NULL);
    DeregisterEventSource(eventLog);

    HRESULT hr = moduleInfo->SetGlobalNotifications(std::make_unique<HelloWorldGlobalModule>().release(), GL_PRE_BEGIN_REQUEST);
    if (FAILED(hr)) return hr;

    return moduleInfo->SetRequestNotifications(std::make_unique<HelloWorldHttpModuleFactory>().release(), RQ_BEGIN_REQUEST, 0);
}
