# KiraNastro VSTi - AI Coding Agent Guideline

## Project Overview

KiraNastro VSTi is a **JUCE-based VSTi plugin** that helps voice actors and singers record UTAU voicebank data (連続音/VCV) directly within their DAW. It replaces standalone tools like OREMO and RecStar by providing the same guide-BGM-driven recording workflow as a plugin, so users can leverage their existing DAW setup, VST chains, and recording workflows.

**License:** GPLv3

### Core Concept

In the UTAU/vocal synthesis community, recording a VCV (連続音) voicebank involves:
1. Loading a **reclist** (recording list) — a text file listing connected syllable sequences to sing
2. Playing a **guide BGM** — a looping audio file that provides pitch and tempo reference
3. The user sings each reclist entry in time with the BGM, which loops for each entry
4. After recording, the continuous audio is split into individual samples using timing data

KiraNastro VSTi handles steps 1-3 inside a DAW. Step 4 is handled by the companion tool **KiraWavTar** (see Architecture section).

### Why a VSTi?

- Users can use their own microphone chain (EQ, compression, de-noise VSTs)
- DAW provides superior recording, undo, and mixing capabilities
- VSTi outputs the BGM audio directly into the DAW's audio stream
- Users can record on any track while the VSTi track provides the guide

## Build Commands

```bash
# Configure
cmake --preset debug

# Build
cmake --build --preset debug

# Build specific format
cmake --build --preset debug --target KiraNastro_VST3
cmake --build --preset debug --target KiraNastro_AU
cmake --build --preset debug --target KiraNastro_Standalone
```

## Test Commands

```bash
# Run tests
ctest --preset debug
```

## Architecture

### Plugin Type

- **VSTi (Virtual Instrument)**: `IS_SYNTH=TRUE`, `NEEDS_MIDI_INPUT=TRUE`
- **Formats**: VST3, AU (macOS), LV2 (Linux), Standalone (for development)
- **Framework**: JUCE 8.x via CMake, added as a git submodule

### Key Components

#### Audio Layer (PluginProcessor)
- **BGM Playback Engine**: Loads guide BGM WAV files and plays them back through the plugin's audio output, synchronized with timing nodes. The BGM audio is pre-loaded into memory buffers (files are typically short, ~20s).
- **Timing Scheduler**: Tracks playback position against guide BGM timing nodes to determine when to advance to the next reclist entry and when the BGM loops.
- **State Persistence**: Saves/restores current session (reclist path, BGM path, current entry index, settings) via `getStateInformation`/`setStateInformation`.

#### UI Layer (PluginEditor)
- **Main Display**: Shows current reclist entry (large Japanese/romaji text), comment line, next entry preview, progress counter, and a timing indicator (pie chart showing position within current BGM cycle).
- **Bottom Bar**: Brand name "KiraNastro VSTi" with logo, hamburger menu for settings.
- **Settings/Menu**: Load reclist, load BGM, export label file, navigation controls.
- **Design System**: Loosely follows Material Design 3. Uses rounded containers, soft shadows, blue primary color (#1A3FC7 approximate from design).

#### Data Layer
- **ReclistParser**: Loads `.txt` reclist files (Shift-JIS or UTF-8), parses entries and optional comment files.
- **GuideBGMParser**: Loads OREMO-format guide BGM timing description files (`.txt` alongside `.wav`).
- **LabelExporter**: Generates timing label files compatible with KiraWavTar for splitting recorded audio.

### Companion Tool: KiraWavTar

KiraWavTar is a separate Qt 6 application that combines/extracts audio files. After recording in the DAW, users export the continuous recording as a single WAV file, then use KiraWavTar with a label/description file to split it into individual per-entry WAV files suitable for UTAU.

## File Formats

### Reclist Format (`.txt`)

- Plain text, one entry per line (or space-separated on a single line — both are valid)
- Encoding: Shift-JIS (legacy) or UTF-8
- Blank lines separate groups (visual organization only)
- Each entry is a sequence of connected syllables, e.g. `_あんああいあうあ`
- The `_` prefix indicates starting from silence

**Example:**
```
_あんああいあうあ
_いんいいういえい
_うんううえうおう
```

### Comment File Format (`<reclistname>-comment.txt`)

- Same encoding as reclist
- Lines starting with `#` are comments (skipped)
- Each line: `<entry><separator><comment>` where separator is space, tab, or colon
- Comment provides the romaji/phonetic reading for display

**Example:**
```
_あんああいあうあ	_a_n_a_a_i_a_u_a
_いんいいういえい	_i_n_i_i_u_i_e_i
```

### Guide BGM Timing File Format (`.txt`, same name as `.wav`)

OREMO-compatible format. This is the key format we must support.

- **Line 1**: Time unit — `sec` or `msec`
- **Lines starting with `#`**: Comments (skipped)
- **Data lines**: CSV with fields:
  1. Row number (1-indexed, sequential)
  2. Time position (in the declared unit)
  3. isRecordingStart (0 or 1) — triggers recording start
  4. isRecordingEnd (0 or 1) — triggers recording stop
  5. isSwitching (0 or 1) — triggers advance to next entry
  6. repeatTargetNodeIndex (1-indexed; 0 = no repeat) — which row to loop back to
  7. Comment (optional string)

**Example** (from OneNoteJazz BGM at 100 BPM, key A):
```
msec

# One Note Jazz by Chie ( http://chie.cc/ )

1, 9600.0, 0, 0, 0, 0, BGM再生
2, 10800.0, 1, 0, 0, 0, 録音開始
3, 12000.0, 0, 0, 0, 0, 　　　　発声はじめ！
4, 16800.0, 0, 0, 0, 0, 　　　　　　発声おわり！
5, 18000.0, 0, 1, 0, 0, 録音停止
6, 19200.0, 0, 0, 1, 1, 録音を保存し次へ。BGMを先頭へリピート。
```

**Typical timing flow:**
1. BGM playback starts (row 1)
2. Recording starts (row 2, isRecordingStart=1)
3. User begins singing (row 3, just a cue marker)
4. User finishes singing (row 4, just a cue marker)
5. Recording stops (row 5, isRecordingEnd=1)
6. Switch to next entry & loop BGM (row 6, isSwitching=1, repeat=1 → back to row 1)

**Note:** In our VSTi context, "recording start/stop" maps to the timing window we track for label generation. The actual DAW recording is managed by the user. We just need to know the boundaries to generate correct labels.

### BGM Organization

BGMs come in sets organized by BPM and pitch:
```
OneNoteJazz by ちえP/
├── 90/     (Jazz-90-A.wav, Jazz-90-A.txt, Jazz-90-A#.wav, Jazz-90-A#.txt, ...)
├── 100/
├── 110/
├── 120/
├── 130/
├── 140/
└── midi/
```

Each BPM folder contains WAV+TXT pairs for every semitone (C, C#, D, D#, E, F, F#, G, G#, A, A#, B).

## UI Design

### Layout (from design draft)

The plugin window has a compact, horizontal layout:

1. **Main card area** (top, rounded rectangle):
   - Small text: comment/romaji reading (e.g., `_a_n_a_a_i_a_u_a`)
   - Large text: current entry in Japanese (e.g., `＿あんああいあうあ`)
   - Right side: circular timing indicator (pie chart showing BGM cycle progress)

2. **Next entry bar** (middle):
   - `>>` icon + next entry text (smaller)
   - Progress counter: `% 14 / 115` (current / total)

3. **Bottom bar** (dark blue/navy):
   - Brand logo + "KiraNastro VSTi" text
   - Hamburger menu icon (right side)

### Design System
- **Style**: Material Design 3 (loosely)
- **Primary Color**: Blue (~#1A3FC7 for text/accents, navy for bottom bar)
- **Background**: Light blue/white gradient
- **Cards**: Rounded corners, subtle shadows
- **Typography**: Sarasa UI JP (body text, CJK), Lexend (brand name)

### Fonts
- **UI Font**: Sarasa UI JP (`SarasaUiJ-Regular.ttf`, `SarasaUiJ-Bold.ttf`, etc.) at `/Users/shine_5402/Downloads/Sarasa-TTF-Unhinted-1.0.36/`
- **Brand Font**: Lexend (variable weight) at `/Users/shine_5402/Downloads/Lexend/`
- Fonts will be embedded as binary data in the plugin

## Project Structure (Planned)

```
KiraNastroVST/
├── CLAUDE.md                       # This file
├── CMakeLists.txt                  # Top-level CMake
├── JUCE/                           # JUCE as git submodule
├── design/                         # Design assets
│   ├── main_draft.pdf
│   ├── branding.svg
│   └── branding.pdf
├── src/
│   ├── PluginProcessor.h/cpp       # AudioProcessor — BGM playback, timing
│   ├── PluginEditor.h/cpp          # AudioProcessorEditor — UI
│   ├── data/
│   │   ├── ReclistParser.h/cpp     # Reclist + comment file parsing
│   │   ├── GuideBGMParser.h/cpp    # Guide BGM timing file parsing
│   │   └── LabelExporter.h/cpp     # Export timing labels for KiraWavTar
│   ├── audio/
│   │   └── BGMPlayer.h/cpp         # In-memory BGM playback with looping
│   ├── ui/
│   │   ├── MainComponent.h/cpp     # Main plugin UI layout
│   │   ├── LookAndFeel.h/cpp       # MD3-inspired custom styling
│   │   └── TimingIndicator.h/cpp   # Pie chart timing display
│   └── utils/
│       ├── Fonts.h                 # Embedded font management
│       └── TextEncoding.h/cpp      # Shift-JIS / UTF-8 detection
└── resources/
    └── fonts/                      # Embedded font files (copied at build time)
```

## Key Technical Decisions

### BGM Playback Strategy
- Pre-load entire BGM WAV into `juce::AudioBuffer<float>` (files are ~20 seconds)
- Track playback position with a sample counter in `processBlock`
- When the timing scheduler says "loop", reset the playback position to the repeat target node's time
- Output BGM audio mixed into the plugin's audio output buffer

### DAW Integration
- The plugin is a VSTi (instrument), so it sits on an instrument track
- BGM audio goes through the plugin output — the user hears it on that track
- The user records on a separate audio track (standard DAW workflow)
- No MIDI is actually needed for the core workflow, but we accept MIDI input as required by VSTi spec
- Could optionally use MIDI note-on to trigger next entry advance

### Label Generation
- Track the timing of each entry (start time relative to DAW transport or relative to recording start)
- Export a label/description file compatible with KiraWavTar's `.kirawavtar-desc.json` format
- Alternative: export simple text-based labels (entry name + start/end time) that can be used with other splitting tools

### Text Encoding
- Must handle Shift-JIS encoded files (very common in the UTAU community, especially Japanese reclists and BGM timing files)
- Auto-detect encoding or provide manual override
- JUCE's string handling is UTF-8 internally, so we need conversion utilities

## Development Phases

### Phase 1: Core Infrastructure
- [ ] Set up JUCE CMake project structure
- [ ] Implement PluginProcessor skeleton (VSTi, accepts MIDI, audio output)
- [ ] Implement PluginEditor skeleton with basic UI
- [ ] Set up font embedding (Sarasa UI JP, Lexend)

### Phase 2: Data Loading
- [ ] Reclist parser (UTF-8 + Shift-JIS)
- [ ] Comment file parser
- [ ] Guide BGM timing file parser
- [ ] BGM WAV file loading into memory

### Phase 3: BGM Playback
- [ ] BGM audio playback in processBlock
- [ ] Timing node tracking and looping
- [ ] Entry advancement logic

### Phase 4: UI
- [ ] Main display (current entry, comment, timing indicator)
- [ ] Next entry preview and progress counter
- [ ] Bottom bar with branding
- [ ] Menu/settings dialog (load reclist, load BGM, etc.)
- [ ] Custom LookAndFeel (MD3-inspired)

### Phase 5: Label Export
- [ ] Track entry timing during recording
- [ ] Export KiraWavTar-compatible label file
- [ ] Export simple text labels as alternative

### Phase 6: Polish
- [ ] State save/restore (DAW session recall)
- [ ] Cross-platform testing (macOS, Windows, Linux)
- [ ] Error handling and edge cases
- [ ] Skip already-recorded entries option

## Reference Codebases

- **RecStar** (`/Users/shine_5402/Project/recstar`): Kotlin Multiplatform app, modern reference for the same workflow. Key files: `model/GuideAudio.kt` (BGM timing), `model/Reclist.kt` (reclist parsing), `audio/RecordingScheduler.kt` (timing logic).
- **OREMO** (`/Users/shine_5402/Downloads/oremo-3.0-b190106/source`): Tcl/Tk app, the original tool. Key files: `proc.tcl` (recording logic, BGM timing parsing), `guideBGM/korede.tcl` (timing file creation tool).
- **KiraWavTar** (`/Users/shine_5402/Project/KiraWavTar`): Qt 6 app for combining/splitting audio files. Uses `.kirawavtar-desc.json` for describing how to split audio.

## Example Data Locations

- **Reclist**: `/Users/shine_5402/Downloads/VCVlist_romaji(beta)/reclist/8mora.txt` (186 entries)
- **Comment file**: `/Users/shine_5402/Downloads/VCVlist_romaji(beta)/reclist/8mora-comment.txt`
- **Guide BGM**: `/Users/shine_5402/Downloads/OneNoteJazz by ちえP/100/Jazz-100-A.wav` (with `.txt` timing file)
- **Sarasa UI JP font**: `/Users/shine_5402/Downloads/Sarasa-TTF-Unhinted-1.0.36/SarasaUiJ-*.ttf`
- **Lexend font**: `/Users/shine_5402/Downloads/Lexend/Lexend-VariableFont_wght.ttf` (or static variants in `static/`)
