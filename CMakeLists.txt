cmake_minimum_required(VERSION 3.20)
project(ets2_ats_vr_controls VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")   # static CRT: no VC++ redistributable needed

# --- OpenXR SDK: headers for the layer, the loader (openxr_loader.dll) for the end-to-end test only. The layer never links
# the loader: the loader loads the layer. Fetched at configure time (git + internet on the first configure).
include(FetchContent)
set(BUILD_API_LAYERS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_CONFORMANCE_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SDK_TESTS OFF CACHE BOOL "" FORCE)
set(DYNAMIC_LOADER ON CACHE BOOL "" FORCE)
FetchContent_Declare(openxr_sdk
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
    GIT_TAG        release-1.1.62
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(openxr_sdk)

set(VRC_WARN_FLAGS /W4 /permissive- /Zc:__cplusplus /utf-8)
set(VRC_LAYER_NAME "XR_APILAYER_JRDEVO_ets2_ats_vr_controls")

# Version resource for every binary we ship (Explorer / antivirus tools see who made it)
function(ck_add_version_info target description original)
    set(VRC_RC_DESC "${description}")
    set(VRC_RC_ORIGINAL "${original}")
    set(VRC_RC_INTERNAL "${target}")
    set(VRC_RC_VER_DOT "${PROJECT_VERSION}")
    set(VRC_RC_VER_COMMA "${PROJECT_VERSION_MAJOR},${PROJECT_VERSION_MINOR},${PROJECT_VERSION_PATCH},0")
    set(VRC_RC_FILETYPE "VFT_DLL")
    configure_file(${CMAKE_SOURCE_DIR}/cmake/version.rc.in ${CMAKE_BINARY_DIR}/version_${target}.rc @ONLY)
    target_sources(${target} PRIVATE ${CMAKE_BINARY_DIR}/version_${target}.rc)
endfunction()

# --- the layer: ets2_ats_vr_controls.dll + its manifest next to it
add_library(ets2_ats_vr_controls SHARED
    src/layer.cpp
    src/pad_input.cpp
    src/profiles.cpp
    src/output.cpp
    src/config.cpp
    src/log.cpp
    src/vr_menu.cpp
    src/overlay.cpp
    src/scs_input.cpp)
target_include_directories(ets2_ats_vr_controls PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(ets2_ats_vr_controls PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
    VRC_VERSION="${PROJECT_VERSION}" VRC_LAYER_NAME="${VRC_LAYER_NAME}")
target_compile_options(ets2_ats_vr_controls PRIVATE ${VRC_WARN_FLAGS})
target_link_libraries(ets2_ats_vr_controls PRIVATE OpenXR::headers user32 gdi32)
ck_add_version_info(ets2_ats_vr_controls "ETS2 ATS VR Controls (OpenXR API layer)" "ets2_ats_vr_controls.dll")
set(VRC_LAYER_DLL "ets2_ats_vr_controls.dll")
configure_file(${CMAKE_SOURCE_DIR}/src/${VRC_LAYER_NAME}.json.in ${CMAKE_BINARY_DIR}/${VRC_LAYER_NAME}.json @ONLY)
add_custom_command(TARGET ets2_ats_vr_controls POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/${VRC_LAYER_NAME}.json $<TARGET_FILE_DIR:ets2_ats_vr_controls>/${VRC_LAYER_NAME}.json)

# --- tests
# unit test of the button mapping (no OpenXR)
add_executable(pad_map_test tests/pad_map_test.cpp)
target_include_directories(pad_map_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(pad_map_test PRIVATE ${VRC_WARN_FLAGS})
target_compile_definitions(pad_map_test PRIVATE NOMINMAX)

# unit test of the profile table + the simple_controller translation (no OpenXR)
add_executable(profiles_test tests/profiles_test.cpp src/profiles.cpp)
target_include_directories(profiles_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(profiles_test PRIVATE ${VRC_WARN_FLAGS})

# unit test of config.ini parsing (run with the repo's config.ini: tests/run_unit.ps1)
add_executable(config_test tests/config_test.cpp src/config.cpp)
target_include_directories(config_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(config_test PRIVATE ${VRC_WARN_FLAGS})
target_compile_definitions(config_test PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)

# unit test of the VR menu + keyboard (made-up frames, no OpenXR; writes a config.ini and the panel pictures into a folder)
add_executable(vr_menu_test tests/vr_menu_test.cpp src/vr_menu.cpp src/config.cpp src/log.cpp)
target_include_directories(vr_menu_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_options(vr_menu_test PRIVATE ${VRC_WARN_FLAGS})
target_compile_definitions(vr_menu_test PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(vr_menu_test PRIVATE user32 gdi32)

# end to end: a fake game (the real OpenXR loader) -> the layer -> a mock runtime with scripted controller input
add_library(mock_runtime SHARED tests/mock_runtime.cpp)
target_compile_options(mock_runtime PRIVATE ${VRC_WARN_FLAGS})
target_compile_definitions(mock_runtime PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(mock_runtime PRIVATE OpenXR::headers d3d11)
configure_file(${CMAKE_SOURCE_DIR}/tests/mock_runtime.json.in ${CMAKE_BINARY_DIR}/mock_runtime.json @ONLY)
add_custom_command(TARGET mock_runtime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/mock_runtime.json $<TARGET_FILE_DIR:mock_runtime>/mock_runtime.json)

add_executable(fake_game tests/fake_game.cpp)
target_compile_options(fake_game PRIVATE ${VRC_WARN_FLAGS})
target_compile_definitions(fake_game PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(fake_game PRIVATE OpenXR::openxr_loader OpenXR::headers d3d11)
add_custom_command(TARGET fake_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:OpenXR::openxr_loader> $<TARGET_FILE_DIR:fake_game>)
add_dependencies(fake_game ets2_ats_vr_controls mock_runtime)
