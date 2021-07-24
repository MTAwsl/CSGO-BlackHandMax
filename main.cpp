#include "stdafx.h"
#define NOCONSOLE

// Here is hooked EndScene Function, I made it become a mainloop
HRESULT APIENTRY Graphics::hkEndScene(LPDIRECT3DDEVICE9 o_pDevice) {
    if (Graphics::renderMutex.try_lock()) {

        if (!Graphics::pDevice)
            Graphics::initD3DDevice(o_pDevice);

        // Address Relocation
        pHax->modClient.init();
        pHax->modEngine.init();
        pHax->modClient.localPlayer.init();

        // Keypad Handler
        if (GetAsyncKeyState(VK_INSERT) & 0x1) {
            pHax->isRCSOn = !pHax->isRCSOn;
#ifndef NOCONSOLE
            const char* logmsg = Hax.isRCSOn ? "RCS is on." : "RCS is off.";
            std::cout << logmsg << std::endl;
#endif
        }
        if (GetAsyncKeyState(VK_DELETE) & 0x1) {
            pHax->isBhopOn = !pHax->isBhopOn;
#ifndef NOCONSOLE
            const char* logmsg = Hax.isBhopOn ? "Bhop is on." : "Bhop is off.";
            std::cout << logmsg << std::endl;
#endif
        }
        if (GetAsyncKeyState(VK_PAUSE) & 0x1) {
            pHax->isTriggerbotOn = !pHax->isTriggerbotOn;
#ifndef NOCONSOLE
            const char* logmsg = Hax.isTriggerbotOn ? "Triggerbot is on." : "Triggerbot is off.";
            std::cout << logmsg << std::endl;
#endif
        }
        if (GetAsyncKeyState(VK_HOME) & 0x1) {
            pHax->isESPOn = !pHax->isESPOn;
#ifndef NOCONSOLE
            const char* logmsg = Hax.isESPOn ? "ESP is on." : "Triggerbot is off.";
            std::cout << logmsg << std::endl;
#endif
        }
        // Hax Handler
        if (pHax->isBhopOn)
            pHax->HandleBunnyhop();
        if (pHax->isTriggerbotOn)
            pHax->HandleTriggerbot();
        if (pHax->isRCSOn)
            pHax->HandleRCS();

        static DWORD lastResetTime = 0;
        if (GetTickCount() - lastResetTime >= 100) {
            pHax->modClient.Reset();
            lastResetTime = GetTickCount();
        }

        if (pHax->isESPOn)
            pHax->DrawESP();

        Graphics::renderMutex.unlock();
    }

    return originalEndScene(o_pDevice);
}

void MainThread(HINSTANCE hModule) {
#ifndef NOCONSOLE
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
#endif
    try {
        Game Hax;
        
#ifndef NOCONSOLE
        std::cout << "-------- [HAX Logging] -------" << std::endl;
#endif
        while (!(GetAsyncKeyState(VK_ESCAPE) & 0x1)) {
            
        }
    }
    catch(const char* msg){
        MessageBoxA(NULL, msg, "Error!", MB_OK | MB_ICONERROR);
    }
#ifndef NOCONSOLE
    fclose(f);
    FreeConsole();
#endif
    FreeLibraryAndExitThread(hModule, 0);
    CloseHandle(hModule);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, NULL, NULL);
        if (hThread)
            CloseHandle(hThread);
    }
    return TRUE;
}