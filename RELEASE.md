# Building OPL PC Tools

Instructions for building release versions of OPL PC Tools for Linux
and Windows. Both scripts build the project, package it together with
the Qt libraries, and automatically create an archive.

## Common preparation

1. Clone the repository and cd into its directory.
2. Initialize the lz4 submodule:

```bash
git submodule update --init --recursive
```

## Building for Linux

### Linux system requirements

- Linux x86_64
- **Docker** or **Podman**
- Internet connection

The build is fully performed in a container. Inside the container Qt is
built from source, so the first run takes a long time.

### Building

```bash
./make-release-linux.sh -v <version>
./make-release-linux.sh -v <version> -t tarball
./make-release-linux.sh -v <version> -t appimage
./make-release-linux.sh -v <version> -y
```

| Option         | Description                                    |
|----------------|------------------------------------------------|
| `-v <version>` | OPL PC Tools version (required)                |
| `-t <target>`  | Target: `all` (default), `tarball`, `appimage` |

The build is performed in several stages (`Dockerfile`, multi-stage
build):

1. **qt-builder** — builds Qt from source and installs it to `/opt/qt6`.
2. **app-builder** — builds OPL PC Tools with CMake/Ninja, removes the
   rpath (`patchelf`), and runs `strip`.
3. **tarball** — assembles a standalone archive with the binary, Qt
   libraries, plugins, and translations.
4. **appimage** — assembles an AppImage using `linuxdeploy` with the Qt
   plugin.

The images are removed after the build; only the final artifacts remain
in `./release/`.

### Artifacts

After the build finishes, the `./release/` directory will contain:

- `oplpctools_linux_<version>_amd64.tar.gz` — tarball;
- `oplpctools_<version>_amd64.AppImage` — AppImage.

## Building for Windows

### Windows system requirements

- Windows 10/11 (64-bit)
- ~5 GB of free disk space
- Internet connection for downloading packages

### Installing MSYS2

1. Download MSYS2 from <https://www.msys2.org/>
2. Run the installer and install to `C:\msys64`
3. After installation, open the **MSYS2 MinGW x86_64** terminal

Run a full update:

```bash
pacman -Syu
```

You may need to close and reopen the terminal. Repeat the command until
it completes successfully:

```bash
pacman -Syu
```

### Installing the toolchain

```bash
pacman -S mingw-w64-x86_64-toolchain
```

This installs GCC, binutils, and other tools for x86_64 (64-bit).

Additional packages:

```bash
pacman -S mingw-w64-x86_64-cmake
pacman -S git
```

> **Note:** CMake can also be installed from the official website
> <https://cmake.org/download/> (choose the Windows x64 Installer). In
> that case it will be available from cmd/PowerShell, but you will need
> to add it to PATH.

### Installing Qt

Qt 6.10.0 must be installed at:

```text
C:\Qt6\6.10.0\mingw_64
```

This is the standard installation path used by the Qt online installer.
This is exactly the path expected by the `make-release-windows.ps1` script.

### Building

Run the script from **PowerShell 7**, specifying the version with the `-v` parameter:

```powershell
./make-release-windows.ps1 -v 4.0
```

The script performs three steps:

1. **Build** — configures CMake (MinGW Makefiles generator, Release) and
   builds the project in `build\windows-amd64\`.
2. **Packaging** — copies `oplpctools.exe` to the temporary folder
   `release\oplpctools\`, runs `windeployqt --release --no-translations`
   (Qt translations are deployed for the Russian locale only and placed
   next to the exe), copies the translations `oplpctools_ru.qm`,
   `qt_ru.qm`, `qtbase_ru.qm`, along with the MinGW runtime DLLs.
3. **Archive** — creates the ZIP archive
   `release\oplpctools_windows_<version>_amd64.zip` and removes the
   temporary folder.

### Artifacts

After the build finishes, the `release\` directory will contain:

- `oplpctools_windows_<version>_amd64.zip` — ZIP archive with the
  executable, Qt libraries, and translations.

## Localization

The UI language is chosen from the system locale at runtime. For a
locale `xx` the application looks for `oplpctools_xx.qm` (project's own
translations) and `qtbase_xx.qm` (Qt's built-in dialogs) next to the
executable, falling back to the application data directory
(`QStandardPaths::AppDataLocation`). If the files are not found, the
application runs untranslated (English).

The translations are compiled at build time:

- Project translations are stored as `.ts` sources in
  `src/OplPcTools/Translations/` and compiled by CMake.
- Qt's own translations (`qt_<locale>.qm`, `qtbase_<locale>.qm`) are
  copied from the Qt installation during packaging.

The list of locales is hardcoded in both scripts and does not depend on
the build machine's system locale:

- `make-release-windows.ps1`: `$Locales = @('ru')`
- `make-release-linux.sh`: `LOCALES="ru"`

The project's own `.qm` files are always bundled (the scripts copy all
`oplpctools_*.qm` from the build directory); the locale list only
controls which of Qt's translations (`qt_<locale>.qm`,
`qtbase_<locale>.qm`) are included.

### Adding a new locale

1. Create `src/OplPcTools/Translations/oplpctools_<locale>.ts` — e.g.
   copy `oplpctools_ru.ts` and translate the strings in Qt Linguist, or
   run the `translation_update` CMake target to generate it from the
   sources.
2. Add the locale to the list in both scripts so that Qt's translations
   are bundled as well:
   - `make-release-windows.ps1`: `$Locales = @('ru', '<locale>')`
   - `make-release-linux.sh`: `LOCALES="ru <locale>"`
3. Rebuild. The compiled `.qm` files are picked up automatically, and at
   runtime the application loads the new locale from the system
   language. No further changes are required.
