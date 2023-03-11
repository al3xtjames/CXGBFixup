CXGBFixup
=========

[Lilu][1] plugin to fix various issues with Chelsio's network driver for macOS.

#### Features

- Works around `MSI vectors enabled < reported` initialization failure
- Fixes link state after wake when using custom MTU

This plugin only supports [cxgb.kext v1.24.5b0][2] (aka cxgb4, which supports
Chelsio Terminator 4/5/6 Ethernet controllers). cxgb3.kext (for Terminator 3)
isn't supported.

#### Boot arguments

- Add `-cxgbfxbeta` to enable CXGBFixup on unsupported OS versions (10.15-13 are
  supported by default)
- Add `-cxgbfxdbg` to enable debug logs (only available in DEBUG binaries and
  requires a DEBUG build of Lilu)
- Add `-cxgbfxoff` to disable CXGBFixup

#### Compilation

Place a DEBUG build of [Lilu.kext][1] and [MacKernelSDK][3] in the root
directory of this repo, then build with `xcodebuild`.

#### Credits

- [Chelsio][4] for porting their cxgb3/cxgb4 drivers to macOS
- [vit9696][5] for writing and maintaining Lilu

[1]: https://github.com/acidanthera/Lilu
[2]: https://service.chelsio.com//store2/T5//Network%20Driver%20(NIC)//Mac%20OS%20X//cxgb-1.24.5b0/cxgb-1.24.5b0_T6_Catalina.dmg
[3]: https://github.com/acidanthera/MacKernelSDK
[4]: https://www.chelsio.com/
[5]: https://github.com/vit9696
