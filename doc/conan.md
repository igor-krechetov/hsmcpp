# Conan packaging and publishing guide

This repository now includes a `conanfile.py` recipe for `hsmcpp`.

## 1) Prerequisites
- Conan 2.x installed
- A Conan remote configured (ConanCenter or your own Artifactory-compatible remote)
- Compiler toolchain + CMake 3.16+

```bash
python3 -m pip install --upgrade conan
conan --version
```

## 2) Create the package locally
From the repository root:

```bash
conan profile detect --force
conan create . --build=missing
```

This command builds `hsmcpp`, runs install rules, and creates a versioned package in your local Conan cache.

## 3) Test consumption locally
You can install the package in another project using:

```bash
conan install --requires=hsmcpp/1.0.2 --build=missing
```

And with CMake, consume generated files from Conan (`CMakeDeps` / `CMakeToolchain`) as usual.

## 4) Export and upload to a remote repository

### Add remote (example)
```bash
conan remote add myremote https://<your-conan-server>/artifactory/api/conan/conan-local
conan remote list
```

### Authenticate
```bash
conan remote login myremote <username>
```

### Upload recipe + binaries
```bash
conan upload hsmcpp/1.0.2 -r=myremote --confirm
```

Use `--only-recipe` first if you want CI to build binaries later:

```bash
conan upload hsmcpp/1.0.2 -r=myremote --only-recipe --confirm
```

## 5) Suggested CI pipeline (recommended)
1. Trigger on tags (for example `v1.0.3`).
2. Run `conan create` for supported compiler/arch matrix.
3. Upload recipe and all binaries to your remote.
4. Optionally promote from staging to production repository.

## 6) Notes for ConanCenter submission
For ConanCenter Index (CCI), you typically:
- Prepare a CCI-style recipe in the `conan-center-index` format.
- Open a PR to CCI repository.
- Follow CCI review guidelines for options, components, and compatibility.

This in-repo recipe is ideal for private remotes and internal/public Artifactory channels, and can serve as the baseline for a CCI recipe.

## 7) Cross-compilation (Windows/Linux hosts, Linux/QNX/Windows targets)

Conan 2 uses two profiles:
- **build profile** (`-pr:b`): machine running Conan (host machine for the build tools).
- **host profile** (`-pr:h`): target machine where the produced binaries will run.

This recipe maps Conan host OS to CMake `HSMBUILD_PLATFORM` like this:
- `Windows` -> `windows`
- `Linux`/`FreeBSD`/`Neutrino (QNX)` -> `posix`

### Supported host machines for packaging
- Linux
- Windows

### Supported target OS
- Linux
- QNX (Conan `os=Neutrino`)
- Windows

### Example profile snippets

`profiles/linux-gcc`:
```ini
[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=13
compiler.libcxx=libstdc++11
build_type=Release
```

`profiles/windows-msvc`:
```ini
[settings]
os=Windows
arch=x86_64
compiler=msvc
compiler.version=194
compiler.cppstd=17
build_type=Release
```

`profiles/qnx-aarch64` (example values, adapt to your SDK/toolchain):
```ini
[settings]
os=Neutrino
os.version=7.1
arch=armv8
compiler=qcc
compiler.version=8.3
compiler.libcxx=libstdc++11
build_type=Release
```

### Example commands

Build Linux target on Linux host:
```bash
conan create . -pr:b=default -pr:h=profiles/linux-gcc --build=missing
```

Build Windows target on Windows host:
```bash
conan create . -pr:b=default -pr:h=profiles/windows-msvc --build=missing
```

Cross-build QNX target on Ubuntu host:
```bash
conan create . -pr:b=default -pr:h=profiles/qnx-aarch64 --build=missing
```

For QNX cross builds, your QNX SDK environment/toolchain must be available in the build environment so CMake can locate compilers and sysroot.
