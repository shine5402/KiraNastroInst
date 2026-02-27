# Building KiraNastro VSTi

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| C++ compiler | C++17 capable | See platform notes below |
| CMake | ≥ 3.22 | [cmake.org/download](https://cmake.org/download/) |
| Ninja | any | Recommended. [ninja-build.org](https://ninja-build.org/) |
| Git | any | For submodule management |

**Platform compiler notes:**
- **macOS**: Xcode Command Line Tools — `xcode-select --install`
- **Windows**: Visual Studio 2022 (with "Desktop development with C++" workload), or LLVM/clang-cl
- **Linux**: GCC ≥ 9 or Clang ≥ 10, plus `libfreetype-dev`, `libasound2-dev`, `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libgl-dev` (exact package names vary by distro)

The JUCE submodule must be initialised before configuring:

```bash
git submodule update --init --recursive
```

---

## Quick Start (CMake Presets)

Configure and build the Standalone app in one step:

```bash
cmake --preset relwithdebinfo
cmake --build --preset standalone
```

The Standalone app lands at:

```
build/relwithdebinfo/KiraNastro_artefacts/RelWithDebInfo/Standalone/
```

---

## Available Presets

### Configure presets

| Preset | Generator | Build type | Platform |
|--------|-----------|------------|----------|
| `relwithdebinfo` | Ninja | RelWithDebInfo — **primary dev preset** | all |
| `debug` | Ninja | Debug | all |
| `release` | Ninja | Release | all |
| `relwithdebinfo-makefiles` | Unix Makefiles | RelWithDebInfo | macOS / Linux |

The `relwithdebinfo-makefiles` preset is a fallback for when Ninja is not
available. On Windows, use the Ninja presets (Ninja is available via
[winget](https://learn.microsoft.com/en-us/windows/package-manager/) or the
Visual Studio installer) or configure manually — see below.

### Build presets

| Preset | What it builds | Platform |
|--------|----------------|----------|
| `relwithdebinfo` | All targets | all |
| `debug` | All targets (debug) | all |
| `release` | All targets (release) | all |
| `standalone` | Standalone app only (fastest iteration) | all |
| `vst3` | VST3 plugin only | all |
| `au` | AU plugin only | **macOS only** |

All presets automatically set `CMAKE_EXPORT_COMPILE_COMMANDS=ON`, so
`compile_commands.json` is always generated in the build directory.

---

## Building Specific Formats

```bash
# Standalone (fastest — use this during active development)
cmake --build --preset standalone

# VST3
cmake --build --preset vst3

# AU (macOS only)
cmake --build --preset au

# Everything
cmake --build --preset relwithdebinfo
```

---

## clangd / IDE Setup

A `.clangd` file is committed to the repository that points clangd at the
`relwithdebinfo` build directory:

```yaml
# .clangd  (already in repo — no action needed)
CompileFlags:
  CompilationDatabase: build/relwithdebinfo
```

After running `cmake --preset relwithdebinfo`, clangd will resolve all JUCE
headers correctly and IDE diagnostics will be accurate. No symlinks or extra
setup required on any platform.

### VS Code

Install the **CMake Tools** and **clangd** extensions. CMake Tools reads
`CMakePresets.json` automatically — select the `relwithdebinfo` configure
preset when prompted on first open. Create the `.clangd` file above for
accurate diagnostics.

### CLion

CLion reads `CMakePresets.json` natively. Open the project root and CLion will
offer to load the presets. Select `relwithdebinfo` as the active profile.
CLion manages indexing internally; no `.clangd` file is needed.

### Visual Studio 2022

Open the project root folder directly (`File → Open → Folder`). Visual Studio
reads `CMakePresets.json` and offers the configured presets in the toolbar.
IntelliSense is driven by `compile_commands.json` automatically.

---

## Manual CMake (without presets)

If your environment does not support presets, configure directly:

```bash
# macOS / Linux
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target KiraNastro_Standalone

# Windows (Visual Studio generator)
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --target KiraNastro_Standalone --config RelWithDebInfo
```

---

## Build Artifacts

After a successful build, artifacts are in:

```
build/<preset>/KiraNastro_artefacts/<BuildType>/
├── Standalone/   KiraNastro VSTi.app        (macOS)
│                 KiraNastro VSTi.exe        (Windows)
│                 KiraNastro VSTi            (Linux)
├── VST3/         KiraNastro VSTi.vst3       (all platforms)
├── AU/           KiraNastro VSTi.component  (macOS only)
└── LV2/          KiraNastro VSTi.lv2        (Linux)
```

Plugins are **not** installed to system directories automatically.
Copy them manually to test:

**macOS**
```
~/Library/Audio/Plug-Ins/VST3/
~/Library/Audio/Plug-Ins/Components/    (AU)
```

**Windows**
```
C:\Program Files\Common Files\VST3\
```

**Linux**
```
~/.vst3/
~/.lv2/
```
