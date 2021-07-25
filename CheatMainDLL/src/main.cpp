#include "stdafx.h"

// Here is hooked EndScene Function, I made it become a mainloop
HRESULT APIENTRY Graphics::hkEndScene(LPDIRECT3DDEVICE9 o_pDevice) {
    if (Graphics::renderMutex.try_lock()) {

        if (!Graphics::pDevice)
            Graphics::initD3DDevice(o_pDevice);

        if (pHax->settings.espMode)
            pHax->DrawESP();

        Graphics::renderMutex.unlock();
    }

    return originalEndScene(o_pDevice);
}

void MainThread(HINSTANCE hModule) {
    FILE* f = nullptr;
    try {
        if (!AllocConsole()) throw BHException(__LINE__, __FILE__, (HRESULT)GetLastError());
        errno_t errnum = freopen_s(&f, "CONOUT$", "w", stdout);
        if (errnum != 0) throw BHException(__LINE__, __FILE__, (HRESULT)errnum);
        try {
            Cheat Hax;
            Cheat* pHax = &Hax;

            std::cout << "-------- [HAX Logging] -------" << std::endl;
            while (!(GetAsyncKeyState(VK_ESCAPE) & 0x1)) {
                // Address Relocation
                pHax->modClient.init();
                pHax->modEngine.init();
                pHax->modClient.localPlayer.init();

                // Keypad Handler
                pHax->settings.UpdateKey();
                // Hax Handler
                if (pHax->settings.isBhopOn)
                    pHax->HandleBunnyhop();
                if (pHax->settings.isTriggerbotOn)
                    pHax->HandleTriggerbot();
                if (pHax->settings.isAimbotOn)
                    pHax->Aimbot.run();
                else if (pHax->settings.isRCSOn)
                    pHax->HandleRCS();


                // Reset bitfield
                static DWORD lastResetTime = 0;
                if (GetTickCount() - lastResetTime >= 100) {
                    pHax->modClient.Reset();
                    lastResetTime = GetTickCount();
                }
            }
        }
        catch (BHException& e) {
            throw e;
        }
    }
    catch (BHException& e) {
        e.Report();
    }
    if (f)
        fclose(f);
    if (!FreeConsole()) throw BHException(__LINE__, __FILE__, GetLastError());
    Sleep(200);
    FreeLibraryAndExitThread(hModule, 0);
    CloseHandle(hModule);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    try {
        if (fdwReason == DLL_PROCESS_ATTACH) {
            DisableThreadLibraryCalls(hinstDLL);
            HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, NULL, NULL);
            if (hThread)
                CloseHandle(hThread);
            else
                throw BHException(__LINE__, __FILE__, GetLastError());
        }
    }
    catch (BHException& e) {
        e.Report();
    }
    return TRUE;
}