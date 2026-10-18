# In-app WebGPU adapter investigation — 18 October 2026

## Restored quality

Removed the experimental 450,000-pixel WebGL2 Automatic ceiling and its
backend-specific quality state. Automatic again uses resolution scale 1,
maximum DPR 1, 8,294,400 pixels and maximum dimension 8192 on both backends.
Restored the previous settings description. Mobile/High retain their original
policies. The active port 8082 server serves `build/web-beach-local-release/site`.
The restored live canvas was verified at 2187x963 rather than 1010x445.

GPU diagnostic additions remain local and profiling-only except for the bounded
20-message renderer status history. Adapter information is serialized explicitly
because GPUAdapterInfo properties are not enumerable.

## Findings

- The in-app page is a secure context and exposes `navigator.gpu`.
- Default, low-power and high-performance adapter requests all return null.
- The engine reports adapter selection failure, before device creation.
- Its WebGL2 fallback uses Intel UHD Graphics 770 through ANGLE/D3D11.
- Standalone Chrome successfully obtains WebGPU adapters for all three
  preferences on Intel `gen-12lp`, not a software adapter. Therefore the working
  result is not explained by using the NVIDIA GPU instead of Intel.
- Installed hardware: Intel UHD Graphics 770, driver 32.0.101.7088; NVIDIA
  GeForce RTX 5060 Ti, driver 32.0.16.1088.
- The installed app is OpenAI.Codex 26.1013.4894.0 with Chromium runtime manifest
  155.0.8059.80. Standalone Chrome installation is also 155.0.8059.80.
- The running app has no explicit disable-gpu/disable-webgpu graphics switch.
  Its GPU preferences string matches the running standalone Chrome processes.
- The app's GPU process loads dxcompiler, d3d12, D3D12Core, and Intel D3D12 driver
  DLLs. Its packaged dxcompiler signature verifies successfully.
- Read-only inspection of packaged JavaScript and graphics-related persisted
  settings found no explicit WebGPU-disable setting. This does not rule out a
  native Chromium restriction, blocklist, initialization error or cached failure.

## User-supplied native diagnostics and isolated reproduction

A minimal HTML page, without the game or engine, also returned null for default,
low-power, high-performance and compatibility adapter requests. It was visible,
secure, and exposed navigator.gpu. Evidence:
`build/evidence/quality-restored-20261018/engine-free-adapter-probe.json`.

The user supplied the in-app chrome://gpu report:

- WebGPU and WebGL both report Hardware accelerated.
- The disabled feature is `webgpu_on_vk_via_gl_interop`, not all WebGPU.
- Dawn information collection was skipped because Vulkan DLLs were not loaded.
  This is not evidence that those files are missing or that D3D12 is disabled.
- GPU process 26964 logs repeated `SharedTextureMemory::BeginAccess() failed`
  in d3d_image_backing.cc and `Error creating wgpu::Texture` in
  shared_image_representation.cc at 11:18 and 11:27.
- Earlier Skia messages report nonexistent shared-image mailboxes.
- ANGLE reports emulated dynamic vector/matrix indexing. This is a potential
  WebGL shader performance lead, not an explanation for adapter selection.

The shared-texture failures identify a browser GPU resource-access failure,
but these process-wide logs do not attribute it to Paws or establish that it
caused the null adapters. No adapter-enumeration error is present. Current
upstream Chromium uses D3D12 by default on Windows, selects ANGLE's adapter
LUID, and filters candidates for blocklist and external-image support. Thus
changing power preference need not switch to a different physical GPU.
Source: https://chromium.googlesource.com/chromium/src/+/lkgr/gpu/command_buffer/service/webgpu_decoder_impl.cc
This source explains the mechanism; it is not proof of the installed app's
specific failure branch.

## Remaining diagnostic

The exact native rejection reason is still unknown. Browser automation blocks
navigation to internal chrome:// pages; the native report above was provided
manually by the user. No browser-policy workaround was attempted. The failure
is reproducible outside the game in the embedded browser while standalone
Chrome succeeds on the same Intel GPU. A full app exit/restart followed by the
minimal probe is the next discriminating check for stale GPU-process state;
it has not been performed because it interrupts this session. If it persists,
the embedded runtime needs native adapter-enumeration diagnostics from its
maintainer. Do not label a specific driver, blocklist, or shared-texture bug as
the proven cause. No app settings, driver files, binaries or safety switches
were changed.

## Validation

`scripts/gpu-path-smoke.cjs` checks the generated site in isolated standalone
Chrome with actual WebGPU and forced adapter absence. It asserts full 2187x963
Automatic resolution on both paths and switching High back to Automatic.
Evidence: `build/evidence/quality-restored-20261018/report.json`.
Live in-app evidence is in the same directory. Historical reduced-quality
performance measurements remain in `build/evidence/live-performance-20261018`;
they are not the current rendering policy.
