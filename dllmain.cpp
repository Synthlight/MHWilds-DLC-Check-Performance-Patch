#include "pch.h"

#include "base-dllmain.h" // Don't forget to add `version.lib` to Linker -> Input -> Additional Dependencies. Linker will give you unresolved symbols without it.
#include "Common.h"
#include "Logger.h"
#include "ScanMemory.h"
#include "Util.h"

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD ulReasonForCall, const LPVOID lpReserved) {
    SetupLog(GetLogPathAsCurrentDllDotLog());
    EmptyDllProxy proxy;
    return BaseDllMain(hModule, ulReasonForCall, lpReserved, proxy);
}

// app.dlc.DlcService::isAvailableDLC(app.dlc.DlcProductId.ID)
typedef bool (*   IsAvailableDlcDef)(void* vmctx, void* self, int32_t id);
IsAvailableDlcDef originalIsAvailableDlc;

std::unordered_map<int32_t, bool> dlcCache;

DECLSPEC_NOINLINE bool IsAvailableDlc(void* vmctx, void* self, const int32_t id) {
    const auto match = dlcCache.find(id);
    if (match == dlcCache.end()) {
        const auto ogResult = originalIsAvailableDlc(vmctx, self, id);
        dlcCache[id]        = ogResult;
        return ogResult;
    }
    return match->second;
}

// app.GUIManager::isNewBenefit()
typedef bool (* IsNewBenefitDef)(void* vmctx, void* self);
IsNewBenefitDef originalIsNewBenefit;

DECLSPEC_NOINLINE bool IsNewBenefit(void* vmctx, void* self) {
    return false;
}

bool MakeHook(const std::string& moduleName, const UINT64 moduleAddress, const std::string& targetName, const LPVOID newMethod, LPVOID* original, const std::string& scanString, const int32_t offset, LogBuffer* logBuffer) {
    LOG_BUFFER("");
    LOG_BUFFER("Scanning for " << targetName << " bytes.");
    const auto addresses = ScanMemory(moduleName, scanString, false, true, nullptr, logBuffer);
    LOG_BUFFER("Found " << addresses.size() << " match(es).");

    if (addresses.empty()) {
        LOG_BUFFER("AoB scan returned no results, aborting.");
        return false;
    }

    const auto injectAddress = const_cast<BYTE*>(addresses[0] + offset);
    LOG_BUFFER("Inject address: " << PRINT_RELATIVE_ADDRESS(moduleName, moduleAddress, injectAddress));

    const auto createHookResult = MH_CreateHook(injectAddress, newMethod, original);
    if (createHookResult != MH_OK) { // NOLINT(clang-diagnostic-microsoft-cast)
        LOG_BUFFER("MH_CreateHook failed: " << createHookResult);
        return false;
    }

    const auto enableHookResult = MH_EnableHook(injectAddress);
    if (enableHookResult != MH_OK) {
        LOG_BUFFER("MH_EnableHook failed: " << enableHookResult);
        return false;
    }

    LOG_BUFFER("Hooked " << targetName << ".");
    return true;
}

bool DoHook(const std::string& moduleName, const PTR_SIZE moduleAddress, LogBuffer* logBuffer) {
    // Initialize MinHook.
    const auto initializeResult = MH_Initialize();
    if (initializeResult != MH_OK) {
        LOG_BUFFER("MH_Initialize failed: " << initializeResult);
        return false;
    }

    return MakeHook(moduleName, moduleAddress, "app.dlc.DlcService::isAvailableDLC(app.dlc.DlcProductId.ID)", &IsAvailableDlc, reinterpret_cast<LPVOID*>(&originalIsAvailableDlc), // NOLINT(clang-diagnostic-microsoft-cast)
                    "5D 41 5E 41 5F C3 CC CC 41 57 41 56 41 54 56 57 55 53 48 83 EC 50 48 8B", 8, logBuffer)
           && MakeHook(moduleName, moduleAddress, "app.GUIManager::isNewBenefit", &IsNewBenefit, reinterpret_cast<LPVOID*>(&originalIsNewBenefit), // NOLINT(clang-diagnostic-microsoft-cast)
                       "55 41 57 41 56 41 55 41 54 56 57 53 48 83 EC 18 48 8D 6C 24 10 48 89 CE 48 8B 05 ?? ?? ?? ?? 48 31 E8 48 89 45 00 48 8B 05 ?? ?? ?? ?? 48 8B 90 B0 02 00 00 48 83 EC 20 41 B8 05 00 00 00 E8 ?? ?? ?? ?? 48 83 C4 20 84 C0", 0, logBuffer);
}

void DoInjection() {
    const auto moduleName   = "MonsterHunterWilds.exe";
    const auto moduleHandle = GetModuleHandle(moduleName);

    if (moduleHandle == nullptr) {
        LOG("Unable to find module, aborting.");
        return;
    }

    const auto moduleAddress = reinterpret_cast<const PTR_SIZE>(moduleHandle);
    LOG("Module base address: " << std::uppercase << std::hex << moduleAddress);

    const auto moduleInfo = GetModuleInfo(GetCurrentProcess(), moduleHandle);
    LOG("Module size: " << std::uppercase << std::hex << moduleInfo.SizeOfImage);

    /*
    AllocateMemory allocator;
    if (!allocator.AllocateGlobalAddresses(moduleName, moduleAddress)) {
        MessageBoxW(nullptr, L"Patching failed.", L"Patching Failed", MB_ICONERROR | MB_OK);
        return;
    }

    const std::vector<PatchFunction> injectorFunctions{
        &IsAvailableDlcPatch,
    };

    const auto result = DoPatchFunctionsAsync(moduleName, moduleAddress, &allocator, injectorFunctions);

    LOG("");

    if (result) {
        //MessageBoxW(nullptr, L"Patching done!", L"Patching Done", MB_ICONINFORMATION | MB_OK);
    } else {
        MessageBoxW(nullptr, L"Patching failed.", L"Patching failed", MB_ICONERROR | MB_OK);
    }

    LOG("Patching done!");
    */

    LOG("");

    if (!DoHook(moduleName, moduleAddress, nullptr)) {
        MessageBoxW(nullptr, L"DLC-Check-Performance-Patch Hooking failed.\nSee the log for details.", L"Hooking Failed", MB_ICONERROR | MB_OK);
    }

    LOG("");
    LOG("Hooking done!");
}