#include "pch.h"
#include "App.h"

int wWinMain(HINSTANCE instance, HINSTANCE instance2, wchar_t* cmdline, int cmdshow)
{
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
    {
        return -1;
    }

    {
        t::App app(instance);
        if (FAILED(app.initialize()))
        {
            return -2;
        }

        ShowWindow(app.get_window(), cmdshow);

        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        app.clean_up();
    }

    CoUninitialize();
    return 0;
}