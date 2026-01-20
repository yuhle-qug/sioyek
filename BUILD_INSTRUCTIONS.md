# Build Instructions for Sioyek Enhanced with Vocabulary Plugin

## Quick Start

### Prerequisites

1. **Qt5 or Qt6 Development Environment**
   - Download from: https://www.qt.io/download-qt-installer
   - Minimum: Qt 5.12 LTS
   - Choose: Desktop development kit for your compiler

2. **C++ Compiler**
   - **Windows (Recommended)**: MSVC 2019 or later (via Visual Studio)
   - **Alternative**: MinGW 64-bit
   - **Alternative**: Clang

3. **Build Tools**
   - **Windows + MSVC**: Visual Studio 2019+ (Community Edition free)
   - **Alternative**: CMake + Ninja (cross-platform)

4. **Git** (to clone repository)

## Windows Build (MSVC)

### Step 1: Install Qt

```bash
# Download Qt Online Installer
Invoke-WebRequest -Uri "https://download.qt.io/official_releases/online_installers/qt-online-installer-windows-x64-online.exe" `
  -OutFile "$env:TEMP\qt-installer.exe"

# Run installer
& "$env:TEMP\qt-installer.exe"
```

**In the installer:**
1. Create account or skip login
2. Select "Custom installation"
3. Choose:
   - **Qt → Qt 5.15 LTS** (or Qt 6.5 LTS)
   - **Tools → MSVC 2019 64-bit** (or matching your Visual Studio version)
   - **Tools → CMake**
4. Install to default location
5. After installation, add to PATH or use full paths

### Step 2: Configure Environment

```powershell
# Option A: Temporary (current PowerShell session only)
$QtRoot = "C:\Qt\5.15.2\msvc2019_64"  # Adjust path to your Qt version
$env:QMAKESPEC = "$QtRoot\mkspecs\win32-msvc"
$env:Path = "$QtRoot\bin;$env:Path"
$env:GEMINI_API_KEY = "your-api-key-here"  # Optional

# Option B: Permanent (Windows environment variables)
# 1. Right-click "This PC" → Properties
# 2. Click "Advanced system settings"
# 3. Click "Environment Variables"
# 4. Add new User variables:
#    - QT_PATH = C:\Qt\5.15.2\msvc2019_64
#    - GEMINI_API_KEY = your-api-key-here
# 5. Edit PATH and add: %QT_PATH%\bin
# 6. Restart PowerShell
```

### Step 3: Generate Build Files

```powershell
cd c:\Users\huy37\Projects\sioyek-enhanced

# Option A: Using qmake (Qt build system)
qmake pdf_viewer_build_config.pro -o Makefile

# Option B: Using CMake (if available)
cmake -B build -DCMAKE_PREFIX_PATH=$env:QMAKESPEC
```

### Step 4: Build

```powershell
# Option A: Using nmake (from MSVC)
nmake  # or nmake release

# Option B: Using MinGW make
mingw32-make

# Option C: Using Visual Studio directly (if qmake generated .sln)
msbuild sioyek.sln /property:Configuration=Release

# Option D: Using CMake
cmake --build build --config Release
```

### Step 5: Run

```powershell
# Find executable (usually in release/ or build/Release/)
ls -r -Filter "*.exe" | where { $_.Name -like "sioyek*" }

# Run it
.\Release\sioyek.exe
```

## Linux Build (GCC/Clang)

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install -y \
  qt5-default \
  libqt5opengl5-dev \
  libqt5sql5-sqlite \
  libqt5svg5-dev \
  libmupdf-dev \
  build-essential \
  cmake

# Fedora/RHEL
sudo dnf install -y \
  qt5-qtbase-devel \
  qt5-qtsvg-devel \
  mupdf-devel \
  gcc-c++ \
  make \
  cmake

# Arch
sudo pacman -S qt5-base qt5-svg mupdf gcc make cmake
```

### Build

```bash
cd ~/Projects/sioyek-enhanced

# Method 1: qmake
qmake pdf_viewer_build_config.pro
make

# Method 2: CMake (if CMakeLists.txt exists)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install
sudo make install  # for qmake
sudo cmake --install build  # for CMake
```

## macOS Build

### Prerequisites

```bash
# Using Homebrew
brew install qt5 mupdf cmake

# Or download from qt.io (recommended for MVVM frameworks)
```

### Build

```bash
cd ~/Projects/sioyek-enhanced

# qmake
qmake pdf_viewer_build_config.pro
make

# Run
open sioyek.app
```

## Troubleshooting

### Error: "qmake: command not found"

```powershell
# Check if Qt is installed
ls "C:\Qt"

# Find qmake
Get-ChildItem -Path "C:\Qt" -Filter "qmake.exe" -Recurse

# Add to PATH manually
$QtBin = "C:\Qt\5.15.2\msvc2019_64\bin"
$env:Path = "$QtBin;$env:Path"
```

### Error: "Cannot find MuPDF headers"

MuPDF is already in the repository (`mupdf/` folder). If build fails:

```bash
# Linux
sudo apt-get install libmupdf-dev

# macOS
brew install mupdf

# Windows - already included, just ensure paths are correct
# Check: pdf_viewer_build_config.pro line with INCLUDEPATH
```

### Error: "Qt headers not found"

Verify Qt installation:

```bash
# List Qt installations
ls /opt/Qt  # macOS/Linux
ls C:\Qt    # Windows

# Check qmake output
qmake -query
# Should show paths like: QT_INSTALL_BINS, QT_INSTALL_LIBS, etc.
```

### Error: "GEMINI_API_KEY not found" at runtime

This is OK! The plugin works without it:
- Dictionary lookups will still work (Free Dictionary API)
- Vietnamese translations will still work (MyMemory)
- Gemini explanations will be skipped

To enable Gemini:
1. Get API key from https://ai.google.dev/
2. Set environment variable:
   ```powershell
   $env:GEMINI_API_KEY = "sk-abc123xyz"
   ```
3. Restart Sioyek

### Linker error: "undefined reference to sqlite3_*"

```bash
# sqlite3 should already be included in pdf_viewer/sqlite3.c
# If not, check:
# 1. sqlite3.c is in SOURCES in .pro file
# 2. sqlite3.h is in pdf_viewer/ directory
# 3. Add this to .pro if needed:
#    SOURCES += pdf_viewer/sqlite3.c
```

## Verifying the Build

### Check Plugin Compilation

```bash
# These files should compile without errors:
# ✓ plugins/dictionary_integration/english_dict_fetcher.cpp
# ✓ plugins/gemini_integration/gemini_client.cpp
# ✓ plugins/quizlet_export/quizlet_exporter.cpp
# ✓ plugins/vocabulary_manager/vocabulary_manager.cpp
# ✓ plugins/vocabulary_manager/vocab_integration.cpp

# If you see compilation errors about these, run:
grep -r "syntax error" build.log  # to find issues
```

### Test at Runtime

After successfully building:

```bash
# 1. Open a PDF
./sioyek /path/to/document.pdf

# 2. Try highlighting a word
#    - Select a word with mouse (usually in "highlight" mode)
#    - A sidebar should appear on the right with definitions

# 3. Export vocabulary
#    - Press 'q' (or configured keybinding)
#    - Choose save location
#    - File should be created as vocabulary.csv

# 4. Check console for errors
#    - Run from terminal: ./sioyek
#    - Errors will print to console
```

## Alternative: Building with CMake

If you prefer CMake over qmake:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel 4
```

This requires a `CMakeLists.txt` file. If needed, we can create one.

## Next Steps After Build

1. **Configure plugins** (see [VOCAB_PLUGIN_SETUP.md](VOCAB_PLUGIN_SETUP.md))
2. **Set up API key** for Gemini:
   ```bash
   $env:GEMINI_API_KEY = "your-key"
   ```
3. **Test vocabulary features**:
   - Highlight words → see sidebar
   - Press 'q' → export to CSV
   - Import CSV to Quizlet

## Build System Architecture

```
pdf_viewer_build_config.pro
├── Qt Modules:
│   ├── gui
│   ├── core
│   ├── opengl
│   ├── sql
│   └── network (for HTTP requests)
│
├── Core Files:
│   ├── pdf_viewer/*.cpp (PDF rendering)
│   ├── mupdf/lib/* (PDF library)
│   └── zlib/* (compression)
│
└── Plugin Files (NEW):
    ├── plugins/dictionary_integration/*.cpp
    ├── plugins/gemini_integration/*.cpp
    ├── plugins/quizlet_export/*.cpp
    └── plugins/vocabulary_manager/*.cpp
```

## Performance Notes

- Initial build: 2-5 minutes depending on system
- Rebuilds after code change: 10-30 seconds
- Runtime: ~500MB memory (PDF rendering takes most)
- Network: Minimal (only on-demand for definitions/Gemini)

---

**Questions?** Check the troubleshooting section or consult:
- Qt Documentation: https://doc.qt.io/
- Sioyek GitHub: https://github.com/ahrm/sioyek
