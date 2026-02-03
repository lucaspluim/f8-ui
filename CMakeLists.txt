cmake_minimum_required(VERSION 3.15)
project(XYControl VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Optimization flags for Release builds (no quality degradation)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /Ob2 /Ot /GL)
    else()
        add_compile_options(-O3 -ffast-math -funroll-loops)
    endif()
endif()

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
)
FetchContent_MakeAvailable(JUCE)

# Create binary data from Resources folder
juce_add_binary_data(GlowResources
    SOURCES
        Resources/glow_blue_layer_0.png
        Resources/glow_blue_layer_1.png
        Resources/glow_blue_layer_2.png
        Resources/glow_blue_layer_3.png
        Resources/glow_blue_layer_4.png
        Resources/glow_red_layer_0.png
        Resources/glow_red_layer_1.png
        Resources/glow_red_layer_2.png
        Resources/glow_red_layer_3.png
        Resources/glow_red_layer_4.png
        Resources/glow_black_layer_0.png
        Resources/glow_black_layer_1.png
        Resources/glow_black_layer_2.png
        Resources/glow_black_layer_3.png
        Resources/glow_black_layer_4.png
)

# Create the GUI application
juce_add_gui_app(XYControl
    PRODUCT_NAME "XY Control"
    COMPANY_NAME "YourCompany"
)

target_sources(XYControl PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/XYControlComponent.cpp
)

# Add platform-specific native dialog implementations
if(APPLE)
    target_sources(XYControl PRIVATE Source/NativeDialogs.mm)
else()
    target_sources(XYControl PRIVATE Source/NativeDialogs.cpp)
endif()

target_compile_definitions(XYControl PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:XYControl,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:XYControl,JUCE_VERSION>"
)

target_link_libraries(XYControl PRIVATE
    juce::juce_gui_extra
    juce::juce_graphics
    GlowResources
)

# Set macOS specific properties
if(APPLE)
    set_target_properties(XYControl PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.yourcompany.xycontrol
    )
endif()

# Utility to generate glow images (run once during development)
add_executable(GenerateGlowImages GenerateGlowImages.cpp)
target_link_libraries(GenerateGlowImages PRIVATE
    juce::juce_graphics
    juce::juce_core
)

# Utility to generate all preset images (blue, red, black)
add_executable(GenerateAllPresetImages GenerateAllPresetImages.cpp)
target_link_libraries(GenerateAllPresetImages PRIVATE
    juce::juce_graphics
    juce::juce_core
)

# Create the VST3 plugin
juce_add_plugin(XYControlPlugin
    PRODUCT_NAME "XY Control"
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Ycom
    PLUGIN_CODE Xyct
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(XYControlPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/XYControlComponent.cpp
    Source/XYControlComponent.h
    Source/NativeDialogs.h
)

# Add platform-specific native dialog implementations
if(APPLE)
    target_sources(XYControlPlugin PRIVATE Source/NativeDialogs.mm)
else()
    target_sources(XYControlPlugin PRIVATE Source/NativeDialogs.cpp)
endif()

target_compile_definitions(XYControlPlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(XYControlPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    GlowResources
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
