#include "pch.h"
#include "hooks.h"
#include "offsets.h"
#include "reclass.h"
#include "drawing.h"
#include "logger.h"

#include <gl/GL.h>
#include <MinHook.h>
#include <vector>
#include <string>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl2.h"

#pragma comment(lib, "opengl32.lib")

struct Settings {
    bool bMenuOpen = true;

    // Combat
    bool bAimbot = true;
    float fSmooth = 5.0f;
    float fovSize = 100.0f;
    bool bShowFov = true;
    bool bVisCheck = true;
    bool bTeamCheck = true;

    // Visuals
    bool bEsp = true;
    bool bBoxes = true;
    bool bHealthBar = true;
    bool bNames = true;
    bool bSnaplines = false;

    // Misc
    bool bGodMode = false;
    bool bInfAmmo = false;
    bool bNoRecoil = false;

    // UI State
    bool bShowEntityList = false;
} cfg;

Settings lastCfg;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool hooks::g_ShuttingDown = false;
WNDPROC oWndProc = nullptr;
twglSwapBuffers owglSwapBuffers = nullptr;
HWND g_hWnd = nullptr;


float GetDistance2D(Vector2 src, Vector2 dst) {
    return sqrtf(powf(dst.x - src.x, 2) + powf(dst.y - src.y, 2));
}

void SetNoRecoil(bool state) {
    static bool isPatched = false;
    uintptr_t recoilAddr = 0x4C2EC3;

    if (state && !isPatched) {
        Logger::Trace("[MEM] Attempting to patch NoRecoil at 0x%p", recoilAddr);
        BYTE patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
        DWORD old;
        if (VirtualProtect((void*)recoilAddr, 5, PAGE_EXECUTE_READWRITE, &old)) {
            memcpy((void*)recoilAddr, patch, 5);
            VirtualProtect((void*)recoilAddr, 5, old, &old);
            isPatched = true;
            Logger::Log("[MEM] NoRecoil Patched Successfully.");
        }
        else {
            Logger::Error("[MEM] Failed to change protection for NoRecoil!");
        }
    }
    else if (!state && isPatched) {
        Logger::Trace("[MEM] Restoring original recoil instructions...");
        BYTE original[] = { 0xF3, 0x0F, 0x11, 0x56, 0x38 };
        DWORD old;
        if (VirtualProtect((void*)recoilAddr, 5, PAGE_EXECUTE_READWRITE, &old)) {
            memcpy((void*)recoilAddr, original, 5);
            VirtualProtect((void*)recoilAddr, 5, old, &old);
            isPatched = false;
            Logger::Log("[MEM] Recoil Restored.");
        }
    }
}

void LogSettingsChanges() {
    if (cfg.bAimbot != lastCfg.bAimbot) Logger::Log("[CFG] Aimbot: %s", cfg.bAimbot ? "ON" : "OFF");
    if (cfg.bEsp != lastCfg.bEsp) Logger::Log("[CFG] ESP: %s", cfg.bEsp ? "ON" : "OFF");
    if (cfg.bGodMode != lastCfg.bGodMode) Logger::Log("[CFG] GodMode: %s", cfg.bGodMode ? "ON" : "OFF");
    if (cfg.bInfAmmo != lastCfg.bInfAmmo) Logger::Log("[CFG] InfAmmo: %s", cfg.bInfAmmo ? "ON" : "OFF");
    if (cfg.bNoRecoil != lastCfg.bNoRecoil) Logger::Log("[CFG] NoRecoil: %s", cfg.bNoRecoil ? "ON" : "OFF");

    lastCfg = cfg;
}

void ApplySmoothAngle(Player* local, float targetYaw, float targetPitch, float smooth) {
    if (smooth <= 1.0f) {
        local->yaw = targetYaw;
        local->pitch = targetPitch;
        return;
    }

    float diffYaw = targetYaw - local->yaw;
    if (diffYaw > 180.0f) diffYaw -= 360.0f;
    if (diffYaw < -180.0f) diffYaw += 360.0f;

    local->yaw += diffYaw / smooth;
    local->pitch += (targetPitch - local->pitch) / smooth;
}

void SetModernTheme() {
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.13f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
}

LRESULT __stdcall hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (hooks::g_ShuttingDown) return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);

    if (msg == WM_KEYDOWN && wParam == VK_INSERT) {
        cfg.bMenuOpen = !cfg.bMenuOpen;
        Logger::Trace("[INPUT] Menu Toggle: %s", cfg.bMenuOpen ? "OPEN" : "CLOSED");
    }

    if (cfg.bMenuOpen) {
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        return 1;
    }

    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

// main hook
BOOL __stdcall hkwglSwapBuffers(HDC hDc) {
    if (hooks::g_ShuttingDown) return owglSwapBuffers(hDc);

    static bool init = false;
    if (!init) {
        g_hWnd = WindowFromDC(hDc);
        Logger::Trace("[RENDER] Initializing ImGui. HDC: 0x%p, HWND: 0x%p", hDc, g_hWnd);
        if (g_hWnd) {
            ImGui::CreateContext();
            ImGui_ImplWin32_Init(g_hWnd);
            ImGui_ImplOpenGL2_Init();
            SetModernTheme();
            oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
            init = true;
            Logger::Log("[HOOK] ImGui and WndProc Initialized.");
        }
    }

    LogSettingsChanges();

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (cfg.bMenuOpen) {
        ImGui::SetNextWindowSize(ImVec2(450, 350), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("EtherBird Internal", &cfg.bMenuOpen)) {
            if (ImGui::BeginTabBar("Tabs")) {
                if (ImGui::BeginTabItem("Combat")) {
                    ImGui::Checkbox("Aimbot", &cfg.bAimbot);
                    ImGui::SliderFloat("Smooth", &cfg.fSmooth, 1.0f, 20.0f);
                    ImGui::SliderFloat("FOV", &cfg.fovSize, 10.0f, 500.0f);
                    ImGui::Checkbox("Vis Check", &cfg.bVisCheck);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Visuals")) {
                    ImGui::Checkbox("ESP", &cfg.bEsp);
                    ImGui::Checkbox("Boxes", &cfg.bBoxes);
                    ImGui::Checkbox("Health", &cfg.bHealthBar);
                    ImGui::Checkbox("Names", &cfg.bNames);
                    ImGui::Checkbox("Snaplines", &cfg.bSnaplines);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Misc")) {
                    ImGui::Checkbox("Godmode", &cfg.bGodMode);
                    ImGui::Checkbox("Inf Ammo", &cfg.bInfAmmo);
                    ImGui::Checkbox("No Recoil", &cfg.bNoRecoil);
                    if (ImGui::Button("Show Entity List", ImVec2(-1, 25))) cfg.bShowEntityList = !cfg.bShowEntityList;
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    if (cfg.bShowEntityList) {
        ImGui::Begin("AC Entity List", &cfg.bShowEntityList);
        if (ImGui::BeginTable("EntTable", 4, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Name"); ImGui::TableSetupColumn("HP"); ImGui::TableSetupColumn("Team"); ImGui::TableSetupColumn("Dist");
            ImGui::TableHeadersRow();
        }
    }

    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);
    Player* local = *(Player**)(moduleBase + 0x18AC00);

    if (local && (uintptr_t)local > 0x1000) {
        if (cfg.bGodMode) local->health = 1337;
        if (cfg.bInfAmmo) { local->ammo = 999; local->mag = 999; }
        SetNoRecoil(cfg.bNoRecoil);

        GLint vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
        float sW = (float)vp[2], sH = (float)vp[3];
        Vector2 sCenter = { sW / 2.0f, sH / 2.0f };

        int* nPlayers = (int*)(moduleBase + 0x18AC0C);
        uintptr_t entList = *(uintptr_t*)(moduleBase + 0x18AC04);
        float* vMatrix = (float*)0x57DFD0;
        int frame = *(int*)0x57F10C;

        Player* bestTarget = nullptr;
        float bestFov = cfg.fovSize;

        for (int i = 0; i < *nPlayers; i++) {
            Player* enemy = *(Player**)(entList + (i * 4));
            if (!enemy || (uintptr_t)enemy < 0x1000 || enemy == local) continue;

            // Entity List
            if (cfg.bShowEntityList) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("%s", enemy->name);
                ImGui::TableSetColumnIndex(1); ImGui::Text("%d", enemy->health);
                ImGui::TableSetColumnIndex(2); ImGui::Text("%s", enemy->team == 0 ? "CLA" : "RVSF");
                ImGui::TableSetColumnIndex(3); ImGui::Text("%.1fm", GetDistance2D({ local->pos.x, local->pos.y }, { enemy->pos.x, enemy->pos.y }));
            }

            if (enemy->state != 0) continue;
            if (cfg.bTeamCheck && enemy->team == local->team) continue;

            bool isVis = enemy->lastVisibleFrame >= frame;
            if (cfg.bVisCheck && !isVis) continue;

            Vector2 sHead, sFeet;
            if (WorldToScreen(enemy->headPos, sHead, vMatrix, (int)sW, (int)sH) &&
                WorldToScreen(enemy->pos, sFeet, vMatrix, (int)sW, (int)sH)) {

                ImU32 col = isVis ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
                float h = sFeet.y - sHead.y;
                float w = h / 2.0f;

                if (cfg.bEsp) {
                    if (cfg.bBoxes) ImGui::GetBackgroundDrawList()->AddRect(ImVec2(sHead.x - w / 2, sHead.y), ImVec2(sHead.x + w / 2, sFeet.y), col);
                    if (cfg.bNames) ImGui::GetBackgroundDrawList()->AddText(ImVec2(sHead.x - w / 2, sHead.y - 15), IM_COL32(255, 255, 255, 255), enemy->name);
                }

                float fovDist = GetDistance2D(sCenter, sHead);
                if (fovDist < bestFov) {
                    bestFov = fovDist;
                    bestTarget = enemy;
                }
            }
        }

        // Aimbot 
        static Player* lastTarget = nullptr;
        if (cfg.bAimbot && bestTarget && (GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
            if (bestTarget != lastTarget) {
                Logger::Trace("[AIM] New Target Acquired: %s", bestTarget->name);
                lastTarget = bestTarget;
            }
            Vector3 delta = bestTarget->headPos - local->headPos;
            float dist = sqrtf(delta.x * delta.x + delta.y * delta.y);
            float targetYaw = (float)(atan2f(delta.y, delta.x) * 180.0f / 3.14159f) + 90.0f;
            float targetPitch = (float)(atan2f(delta.z, dist) * 180.0f / 3.14159f);
            ApplySmoothAngle(local, targetYaw, targetPitch, cfg.fSmooth);
        }
        else if (lastTarget != nullptr) {
            lastTarget = nullptr;
            Logger::Trace("[AIM] Target released.");
        }
    }

    if (cfg.bShowEntityList) { ImGui::EndTable(); ImGui::End(); }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    return owglSwapBuffers(hDc);
}

void hooks::Initialize() {
    Logger::Trace("[INIT] Initializing Hooks...");
    HMODULE hOpengl = GetModuleHandleA("opengl32.dll");
    if (!hOpengl) {
        Logger::Error("[INIT] Failed to find opengl32.dll");
        return;
    }

    void* wglSwapBuffersAddr = (void*)GetProcAddress(hOpengl, "wglSwapBuffers");
    if (MH_CreateHook(wglSwapBuffersAddr, &hkwglSwapBuffers, reinterpret_cast<LPVOID*>(&owglSwapBuffers)) == MH_OK) {
        MH_EnableHook(wglSwapBuffersAddr);
        Logger::Log("[INIT] MinHook: wglSwapBuffers enabled.");
    }
    else {
        Logger::Error("[INIT] MinHook: Failed to hook wglSwapBuffers.");
    }
}

void hooks::Unhook() {
    Logger::Log("[EXIT] Unhooking resources...");
    g_ShuttingDown = true;
    Sleep(100);
    if (g_hWnd && oWndProc) {
        SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        Logger::Trace("[EXIT] WndProc Restored.");
    }
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    Logger::Log("[EXIT] Clean shutdown complete.");
}