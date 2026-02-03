# XY Control - Cross-Platform JUCE Implementation

High-performance XY control pad with organic spring physics and premium blur effects.

## Features

✨ **Organic Spring Physics**
- Real mass, stiffness, and damping simulation
- Heavy, fluid glow that lags behind cursor
- 6 independent spring layers for depth

🎨 **Premium Blur Quality**
- 5-layer Gaussian blur system
- Pre-rendered for instant startup (~50ms)
- Sky blue gradient with excellent color depth

⚡ **Performance**
- 60 FPS locked
- <0.5ms per frame render time
- Minimal CPU usage (~1-2%)

🎯 **Production Ready**
- Instant GUI startup for VST/AU/AAX
- Cross-platform (macOS, Windows, Linux)
- Industry-standard JUCE framework

## Quick Start

### macOS
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
open "XYControl_artefacts/XY Control.app"
```

### Windows
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
.\XYControl_artefacts\Release\XY Control.exe
```

### Linux
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
./XYControl_artefacts/XY\ Control
```

## Documentation

- **BUILD_INSTRUCTIONS_WINDOWS.md** - Detailed Windows build guide
- **PERFORMANCE_OPTIMIZATION.md** - How the blur system works
- **QUICKSTART.md** - Usage guide and customization tips

## What's Included

- Full source code
- Pre-rendered blur images (PNG)
- Image generator utility
- CMake build system
- Cross-platform JUCE integration

## Integration into Your Plugin

1. Copy `XYControlComponent.h` and `XYControlComponent.cpp`
2. Copy `Resources/` folder
3. Add to your CMakeLists.txt:
```cmake
juce_add_binary_data(GlowResources
    SOURCES Resources/glow_layer_0.png
            Resources/glow_layer_1.png
            Resources/glow_layer_2.png
            Resources/glow_layer_3.png
            Resources/glow_layer_4.png
)
target_link_libraries(YourPlugin PRIVATE GlowResources)
```

4. Use in your editor:
```cpp
addAndMakeVisible(xyControl);
xyControl.setBounds(x, y, size, size);
```

## Customization

Edit physics values in `XYControlComponent.cpp` (lines 4-11):
```cpp
SpringLayer(stiffness, damping, mass)
// Lower stiffness = slower response
// Higher damping = smoother motion
// Higher mass = heavier feel
```

Edit glow colors in `XYControlComponent.cpp` (lines 12-17):
```cpp
{size, opacity, Color, {}}
```

## Requirements

- CMake 3.15+
- C++17 compiler
- Internet connection (first build downloads JUCE)

## License

MIT License - Free for commercial use

## Credits

Built with JUCE framework (https://juce.com)
Developed as a reference implementation for VST plugin UIs
