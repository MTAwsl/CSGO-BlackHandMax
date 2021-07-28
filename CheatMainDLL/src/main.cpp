#include "stdafx.h"

namespace ImGui {
    extern IMGUI_API void PushDisabled(bool disabled);
    extern IMGUI_API void PopDisabled();
}

void Graphics::DrawImGui() {

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    // BlackHandMax Console
    {
        ImGui::Begin("BlackHandMax Console");
        ImGui::Checkbox("Aimbot", &pHax->settings.isAimbotOn);

        ImGui::SameLine();
        ImGui::Checkbox("RCS", &pHax->settings.isRCSOn);

        ImGui::SameLine();
        ImGui::Checkbox("Bhop", &pHax->settings.isBhopOn);

        ImGui::SameLine();
        ImGui::Checkbox("TriggerBot", &pHax->settings.isTriggerbotOn);
        
        ImGui::PushDisabled(!pHax->settings.isAimbotOn);
        ImGui::SliderFloat("Smoothness", &pHax->settings.Aimbot.smoothness, 1.0f, 100.0f, "%.3f");
        ImGui::Checkbox("FriendlyFire", &pHax->settings.Aimbot.friendlyFire);
        ImGui::PopDisabled();

        ImGui::Text("ESP Modes");
        ImGui::RadioButton("Off", &pHax->settings.espMode, 0); ImGui::SameLine();
        ImGui::RadioButton("2D", &pHax->settings.espMode, 1); ImGui::SameLine();
        ImGui::RadioButton("3D", &pHax->settings.espMode, 2);

        ImGui::PushDisabled(!pHax->settings.espMode);
        ImGui::Checkbox("Snaplines", &pHax->settings.isSnaplineOn); ImGui::SameLine();
        ImGui::Checkbox("ESP Text", &pHax->settings.isESPTextOn);
        ImGui::PopDisabled();

        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

// Here is hooked EndScene Function, I made it become a mainloop
HRESULT APIENTRY Graphics::hkEndScene(LPDIRECT3DDEVICE9 o_pDevice) {
    if (Graphics::renderMutex.try_lock()) {

        if (!Graphics::pDevice)
            Graphics::initD3DDevice(o_pDevice);

        if (pHax->settings.espMode)
            pHax->DrawESP();

        if (pHax->settings.guiVisible)
            Graphics::DrawImGui();

        Graphics::renderMutex.unlock();
    }

    return originalEndScene(o_pDevice);
}

void MainThread(HINSTANCE hModule) {
    try {
        Cheat Hax;
        Cheat* pHax = &Hax;

        while (!(GetAsyncKeyState(VK_END) & 0x1)) {
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
        e.Report();
    }
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