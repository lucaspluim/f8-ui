# XY Control VST3 Plugin

A physics-based XY control interface built with JUCE, featuring smooth spring animations, Gaussian blur effects, and a native macOS preset system.

![XY Control Demo](demo.gif)

## Features

### UI/UX
- **Spring Physics**: Ball smoothly springs back to center when released
- **Breathing Animation**: Subtle pulsing effect for visual interest
- **Three Color Presets**: Blue (default), Red, and Black themes
- **Disperse Effect**: Particles explode outward when changing presets
- **Hold Indicator**: Growing blue ring shows 3-second hold progress
- **Smooth Animations**: 60fps rendering with optimized performance

### Presets
- **Double-click outside**: Cycle through color presets
- **Hold 3 seconds outside**: Open native macOS preset browser
- **Native File Dialogs**: Save/load presets using system file browser
- **JSON Format**: Presets stored in `~/Documents/XYControl Presets/`

### Plugin Features
- **VST3 Format**: Works in any DAW (tested in Ableton Live)
- **State Saving**: XY position persists with project
- **Audio Pass-through**: Currently passes audio unchanged (ready for DSP)
- **Cross-platform**: macOS (ARM64) with fallback for other platforms

## Build Instructions

### Prerequisites
- CMake 3.15 or higher
- Xcode (macOS) or Visual Studio (Windows)
- C++17 compiler

### macOS Build

```bash
# Clone the repository
git clone <repository-url>
cd xy-control-juce

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build standalone app
make XYControl -j8

# Build VST3 plugin
make XYControlPlugin_VST3 -j8

# VST3 is automatically copied to: ~/Library/Audio/Plug-Ins/VST3/
```

### Generating Glow Images

The project includes pre-generated glow images, but you can regenerate them:

```bash
cd build
./GenerateAllPresetImages
```

This creates 15 glow images (5 layers × 3 presets) in the `Resources/` folder.

## Project Structure

```
xy-control-juce/
├── Source/
│   ├── Main.cpp                    # Standalone app entry point
│   ├── PluginProcessor.cpp/h       # VST3 audio processor
│   ├── PluginEditor.cpp/h          # VST3 editor wrapper
│   ├── MainComponent.cpp/h         # UI container with preset system
│   ├── XYControlComponent.cpp/h    # XY pad with physics engine
│   └── NativeDialogs.mm/h          # macOS native file browsers
├── Resources/
│   └── glow_*.png                  # Pre-rendered Gaussian blur layers
├── CMakeLists.txt                  # Build configuration
├── GenerateGlowImages.cpp          # Utility to create glow images
└── GenerateAllPresetImages.cpp     # Utility for all 3 presets
```

## For Plugin Developers

### Adding Audio Processing

The plugin currently passes audio through unchanged. To add DSP:

1. **Add Parameters** in `PluginProcessor.h`:
```cpp
juce::AudioProcessorValueTreeState parameters;
```

2. **Link Parameters to UI** in `PluginEditor.cpp`:
```cpp
// Get XY position from MainComponent
auto xyPos = mainComponent.getXYPosition();

// Update processor parameters
audioProcessor.setParameter("x", xyPos.x);
audioProcessor.setParameter("y", xyPos.y);
```

3. **Process Audio** in `PluginProcessor::processBlock()`:
```cpp
void XYControlAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, 
                                          juce::MidiBuffer& midiMessages)
{
    float x = xyPosition.x;  // 0.0 to 1.0
    float y = xyPosition.y;  // 0.0 to 1.0
    
    // Your DSP code here
    // Example: x could control filter cutoff, y could control resonance
}
```

### Adding MIDI Output

To send MIDI CC messages based on XY position:

1. In `CMakeLists.txt`, set:
```cmake
NEEDS_MIDI_OUTPUT TRUE
```

2. In `PluginProcessor::processBlock()`:
```cpp
midiMessages.addEvent(juce::MidiMessage::controllerEvent(
    1,              // MIDI channel
    74,             // CC number
    int(xyPosition.x * 127)  // CC value (0-127)
), 0);
```

## Technical Details

### Physics Engine
- Spring constant: 0.20
- Damping: 1.13
- Mass: 1.6
- Update rate: 60Hz

### Rendering Optimization
- Pre-rendered Gaussian blur (5 layers per preset)
- Layered compositing for smooth glow effects
- `setOpaque(true)` for faster repaints
- Double-buffered rendering

### Color Presets
- **Blue**: RGB(0,122,255) on white background
- **Red**: RGB(255,69,58) on dark red background
- **Black**: White glow on black background (smaller glow radius for balance)

## License

[Your License Here]

## Credits

Built with [JUCE Framework](https://juce.com/)
