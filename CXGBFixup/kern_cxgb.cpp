//
//  kern_cxgb.cpp
//  CXGBFixup
//
//  Copyright © 2023 Alex James.
//  SPDX-License-Identifier: BSD-3-Clause
//

#include "kern_cxgb.hpp"

CXGBFX *CXGBFX::callbackCXGBFX { nullptr };

const char *CXGBFX::kextPaths[] { "/Library/Extensions/cxgb.kext" };

IOReturn CXGBFX::setPowerState(void *that, unsigned long state, IOService *service) {
    auto ret = FunctionCast(setPowerState, callbackCXGBFX->orgSetPowerState)(that, state, service);
    DBGLOG("cxgbfx", "setPowerState(state=%lu) -> %#x", state, ret);

    if (callbackCXGBFX->savedMTU != 0 && ret == kIOPMAckImplied && state == 2) {
        auto ret2 = FunctionCast(cxgb_change_mtu, callbackCXGBFX->orgChangeMTU)(that, callbackCXGBFX->savedMTU);
        if (ret2 != 0) {
            SYSLOG("cxgbfx", "cxgb_change_mtu(%d) failed (%d)", callbackCXGBFX->savedMTU, ret2);
        }
    }

    return ret;
}

int CXGBFX::cxgb_change_mtu(void *that, int mtu) {
    auto ret = FunctionCast(cxgb_change_mtu, callbackCXGBFX->orgChangeMTU)(that, mtu);
    DBGLOG("cxgbfx", "cxgb_change_mtu(mtu=%d) -> %d", mtu, ret);

    if (ret == 0) {
        callbackCXGBFX->savedMTU = mtu;
    }

    return ret;
}

void CXGBFX::processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t slide, size_t size) {
    if (index != kextList[Kext::CXGB4].loadIndex) {
        return;
    }

    // Skip over "MSI vectors enabled < reported" check in com_chelsio_driver_cxgb4::check_msi
    auto offset = patcher.solveSymbol(index, "__ZN24com_chelsio_driver_cxgb49check_msiEP9IOServicePi");
    if (offset == 0) {
        SYSLOG("cxgbfx", "failed to solve com_chelsio_driver_cxgb4::check_msi (%d)", patcher.getError());
        return;
    }

    const uint8_t find[] {
        0x45, 0x39, 0xf5, // cmp r13d, r14d
        0x7d, 0x38        // jge loc_36ce
    };

    const uint8_t replace[] {
        0x0f, 0x1f, 0x00, // nop dword [rax]
        0xeb, 0x38        // jmp loc_36ce
    };

    static_assert(sizeof(find) == sizeof(replace));
    const KernelPatcher::LookupPatch patch {
        .kext    = &kextList[Kext::CXGB4],
        .find    = find,
        .replace = replace,
        .size    = sizeof(find),
        .count   = 1
    };

    patcher.applyLookupPatch(&patch, reinterpret_cast<uint8_t *>(offset), 512);
    if (patcher.getError() != KernelPatcher::Error::NoError) {
        SYSLOG("cxgbfx", "failed to apply MSI patch (%d)", patcher.getError());
        return;
    }

    // Fix custom MTU after wakeup
    KernelPatcher::RouteRequest requests[] {
        {
            /* .symbol = */ "__ZN24com_chelsio_driver_cxgb413setPowerStateEmP9IOService",
            /* .to     = */ setPowerState,
            /* .org    = */ orgSetPowerState
        },
        {
            /* .symbol = */ "__ZN24com_chelsio_driver_cxgb415cxgb_change_mtuEi",
            /* .to     = */ cxgb_change_mtu,
            /* .org    = */ orgChangeMTU
        }
    };

    if (!patcher.routeMultiple(index, requests, slide, size)) {
        SYSLOG("cxgbfx", "failed to route functions (%d)", patcher.getError());
        return;
    }
}

void CXGBFX::init() {
    callbackCXGBFX = this;
    auto err = lilu.onKextLoad(kextList, arrsize(kextList),
                               [](void *user, KernelPatcher &patcher, size_t index, mach_vm_address_t address,
                                  size_t size) {
                                   auto cxgb = static_cast<CXGBFX *>(user);
                                   cxgb->processKext(patcher, index, address, size);
                               }, this);

    if (err != LiluAPI::Error::NoError) {
        SYSLOG("cxgbfx", "failed to register kext load callback (%d)", err);
    }
}
