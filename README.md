# StepDistort

A distortion audio plugin (VST3 / AU) built with [JUCE](https://juce.com). Instead of
one fixed distortion sound, it runs an 8-step sequencer that cycles through a
different distortion type and drive level per step, synced to your DAW's tempo.

Built to run inside FL Studio (or any VST3/AU host) as an effect on a mixer track.

## What it does

- 8 steps, each with:
  - On/off
  - Distortion type: Clean, Soft Clip, Hard Clip, Foldback, Bitcrush
  - Drive amount
- Step rate (1/4, 1/8, 1/16, 1/8 triplet, 1/16 triplet, 1/32), synced to host BPM
- Global dry/wet Mix and Output Gain
- The UI highlights whichever step is currently playing

This is a v1 — 8 steps are fixed for now, and there's no preset save/load beyond
what your DAW project already stores (plugin state is saved with the project).

## 1. Download the code

This code lives in this GitHub repo. On the computer you'll build on (your Mac),
get a copy with **one** of these:

**Option A — using git (recommended):**

```bash
git clone https://github.com/lana6478/fx_plugin_1.git
cd fx_plugin_1
```

If you already have a local clone, just update it:

```bash
cd fx_plugin_1
git pull
```

**Option B — without git:**

1. Go to https://github.com/lana6478/fx_plugin_1 in your browser
2. Click the green **Code** button → **Download ZIP**
3. Unzip it, then open a Terminal in the unzipped folder

## 2. Install the build tools (one-time setup, macOS)

You need Xcode's command line tools, Homebrew, and CMake.

```bash
# Xcode command line tools (installs a popup — click "Install")
xcode-select --install

# Homebrew, if you don't already have it: https://brew.sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# CMake
brew install cmake
```

## 3. Build the plugin

From inside the `fx_plugin_1` folder:

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

The first build will take a while — CMake automatically downloads JUCE itself
(via `FetchContent`), then compiles everything. Later builds are much faster.

## 4. Install it

The build is configured to copy the plugin into the standard macOS plugin
folders automatically, so after step 3 you should already find:

- `~/Library/Audio/Plug-Ins/VST3/StepDistort.vst3`
- `~/Library/Audio/Plug-Ins/Components/StepDistort.component` (AU)

If they're not there, copy them manually from inside `build/StepDistort_artefacts/Release/`.

## 5. Use it in FL Studio

1. Open FL Studio
2. **Options → Manage Plugins**, then click **Find plugins** (or **Find more
   plugins**) to rescan
3. Once it's found, add **StepDistort** as an effect on a mixer insert
   (right-click an empty effect slot → select it from the list)
4. Press play in FL Studio — the step highlight in the plugin UI should move
   in time with the transport, and the sound passing through that insert will
   be distorted differently per step

There's also a **Standalone** app built alongside the plugin
(`build/StepDistort_artefacts/Release/Standalone/StepDistort.app`) you can
launch directly to hear it without opening FL Studio — useful for quick
tweaking. With no host transport it free-runs at 120 BPM.

## Project layout

```
CMakeLists.txt          Build configuration (fetches JUCE, defines the plugin targets)
Source/
  PluginProcessor.h/.cpp   Audio engine: parameters, step sequencer clock, DSP
  PluginEditor.h/.cpp      UI: step grid, top bar controls
  Distortion.h             The actual distortion algorithms
```

## Ideas for later

- Variable step count (4/16/32) instead of fixed at 8
- Per-step stereo width / filter
- Swing
- Preset save/load UI (beyond DAW project state)
