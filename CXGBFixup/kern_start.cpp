//
//  kern_start.hpp
//  CXGBFixup
//
//  Copyright © 2023 Alex James.
//  SPDX-License-Identifier: BSD-3-Clause
//

#include <Headers/plugin_start.hpp>

#include "kern_cxgb.hpp"

static CXGBFX cxgbfx;

static const char *bootargOff[] {
    "-cxgbfxoff"
};

static const char *bootargDebug[] {
    "-cxgbfxdbg"
};

static const char *bootargBeta[] {
    "-cxgbfxbeta"
};

PluginConfiguration ADDPR(config) {
    .product       = xStringify(PRODUCT_NAME),
    .version       = parseModuleVersion(xStringify(MODULE_VERSION)),
    .runmode       = LiluAPI::AllowNormal | LiluAPI::AllowSafeMode,
    .disableArg    = bootargOff,
    .disableArgNum = arrsize(bootargOff),
    .debugArg      = bootargDebug,
    .debugArgNum   = arrsize(bootargDebug),
    .betaArg       = bootargBeta,
    .betaArgNum    = arrsize(bootargBeta),
    .minKernel     = KernelVersion::Catalina,
    .maxKernel     = KernelVersion::Ventura,
    .pluginStart   = []() {
        cxgbfx.init();
    }
};
