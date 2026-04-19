# Killer B Synth

JUCE-based synthesizer plugin project for the `Killer B Synth` UI and DSP prototype.

## Build

Requirements:
- CMake 3.22+
- Visual Studio 2022

Configure and build:

```powershell
cmake -S . -B build
cmake --build build --config Release --target KillerBSynth_VST3
```

The VST3 target is configured to copy/install the plugin after build.

## Project Layout

- `Source/` plugin DSP and editor code
- `JUCE/` vendored JUCE framework
- `Progression/` UI progression references
- `FINAL VERSION.png` target UI reference

## Notes

- Branding images are embedded with JUCE `BinaryData`.
- The current editor layout includes placeholder-only controls for parameters not yet exposed in the processor.
