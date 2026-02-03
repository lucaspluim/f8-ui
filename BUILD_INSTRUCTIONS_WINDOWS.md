# Building XY Control on Windows

## Prerequisites

1. **Visual Studio 2019 or later** with C++ tools
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Select "Desktop development with C++" workload

2. **CMake 3.15 or later**
   - Download from: https://cmake.org/download/
   - Or install via: `winget install Kitware.CMake`

3. **Git** (to clone if needed)
   - Download from: https://git-scm.com/download/win

## Build Steps

### 1. Open Command Prompt or PowerShell

Navigate to the project folder:
```powershell
cd C:\path\to\xy-control-juce
```

### 2. Create Build Directory

```powershell
mkdir build
cd build
```

### 3. Generate Visual Studio Project

```powershell
cmake .. -G "Visual Studio 17 2022"
```

Or for Visual Studio 2019:
```powershell
cmake .. -G "Visual Studio 16 2019"
```

### 4. Build the Project

```powershell
cmake --build . --config Release
```

This will take 2-3 minutes the first time (downloads JUCE automatically).

### 5. Run the Application

```powershell
.\XYControl_artefacts\Release\XY Control.exe
```

Or just double-click the .exe in Windows Explorer:
`build\XYControl_artefacts\Release\XY Control.exe`

## Alternative: Use Visual Studio GUI

1. Open Visual Studio
2. File → Open → CMake
3. Select `CMakeLists.txt` from the project root
4. Visual Studio will automatically configure CMake
5. Build → Build All
6. Debug → Start Without Debugging (or press Ctrl+F5)

## Performance Note

The app opens in ~50ms on macOS. On Windows it should be similar or slightly faster.

## Troubleshooting

### "CMake not found"
Add CMake to your PATH or use full path:
```powershell
"C:\Program Files\CMake\bin\cmake.exe" ..
```

### "Cannot find Visual Studio"
Install Visual Studio with C++ tools, or use a different generator:
```powershell
cmake .. -G "MinGW Makefiles"
```

### "JUCE download fails"
Check your internet connection. CMake downloads JUCE from GitHub automatically.

## What You're Building

This is a high-performance XY control pad with:
- Real spring physics (mass, stiffness, damping)
- 5-layer Gaussian blur glow effect
- Comet tail that reacts to movement speed
- Breathing animation when idle
- 60 FPS locked, <0.5ms per frame
- Pre-rendered blur images for instant startup

All ready to integrate into a VST/AU/AAX plugin.

## Code Structure

```
xy-control-juce/
├── Source/
│   ├── Main.cpp              # App entry point
│   ├── MainComponent.h/.cpp  # Window container
│   └── XYControlComponent.h/.cpp  # The XY control widget
├── Resources/
│   └── glow_layer_*.png      # Pre-rendered blur images
├── GenerateGlowImages.cpp    # Utility to regenerate images
├── CMakeLists.txt            # Build configuration
└── README.md                 # Full documentation
```

## Next Steps

Once it's running, check out:
- `XYControlComponent.cpp` lines 4-11 for physics tuning
- `XYControlComponent.cpp` lines 12-17 for glow colors/sizes
- `PERFORMANCE_OPTIMIZATION.md` for how the blur system works

Enjoy! 🎛️
