# Third-party components

| Component | Used for | License |
|---|---|---|
| [OpenXR SDK](https://github.com/KhronosGroup/OpenXR-SDK) (Khronos Group), release 1.1.62 | headers the layer is built against; the loader (`openxr_loader.dll`) for the end-to-end test only | Apache-2.0 (some files MIT); see the SDK's `LICENSE` |

The OpenXR SDK is downloaded at configure time (CMake FetchContent) and is not part of this repository. The released layer
(`ets2_ats_vr_controls.dll`) contains no third-party code beyond the OpenXR header definitions and links only Windows
system libraries (user32, gdi32; d3d11 is used through the game's own device).
