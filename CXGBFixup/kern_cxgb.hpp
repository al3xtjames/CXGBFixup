//
//  kern_cxgb.hpp
//  CXGBFixup
//
//  Copyright © 2023 Alex James.
//  SPDX-License-Identifier: BSD-3-Clause
//

#include <Headers/kern_api.hpp>

#include <IOKit/IOService.h>

class CXGBFX {
public:
    void init();

private:
    void processKext(KernelPatcher &patcher, size_t index, mach_vm_address_t slide, size_t size);

    // Wrapped com_chelsio_driver_cxgb4::setPowerState function
    static IOReturn setPowerState(void *that, unsigned long state, IOService *service);

    // Wrapped com_chelsio_driver_cxgb4::cxgb_change_mtu function
    static int cxgb_change_mtu(void *that, int mtu);

    enum Kext {
        CXGB4,
        NumKexts
    };

    static const char *kextPaths[Kext::NumKexts];
    KernelPatcher::KextInfo kextList[Kext::NumKexts] {
        [Kext::CXGB4] = {
            .id      = "com.chelsio.driver.cxgb",
            .paths   = kextPaths,
            .pathNum = arrsize(kextPaths)
        }
    };

    // Saved MTU from last successful cxgb_change_mtu call
    int savedMTU {};

    // Self instance for callbacks
    static CXGBFX *callbackCXGBFX;

    // Original com_chelsio_driver_cxgb4::setPowerState function address
    mach_vm_address_t orgSetPowerState {};

    // Original com_chelsio_driver_cxgb4::cxgb_change_mtu function address
    mach_vm_address_t orgChangeMTU {};
};
