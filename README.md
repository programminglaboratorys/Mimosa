<!-- PROJECT LOGO -->
<br />
<div align="center">
  <img src="./docs/logo.svg" alt="mimosa logo" width="80" height="80">

<h3 align="center">mimosa</h3>

<p align="center">
  <strong>Rust style mutex library where it stores the type in the mutex itself</strong>
</p>

<p align="center">
  <a href="#usage">Usage</a> •
  <a href="#building">Building</a> •
  <a href="#project-structure">Project Structure</a> •
  <a href="#license">License</a>
</p>

</div>

---

## About
cross-compiler, cross-platform mutex library that aims to mimic Rust style mutex where it stores the type in the mutex itself

## Usage

```cpp
#include <mimosa/mimosa.hpp>

auto text = mimosa::greeting();
```

## Building

Requires CMake 3.21+ and a C++17 compiler. On Windows, open the folder in Visual Studio 2022 and pick a configuration from the dropdown, or from a terminal (after loading the matching `vcvars64.bat`/`vcvars32.bat`):

```
cmake --preset x64-Debug
cmake --build --preset x64-Debug
ctest --preset x64-Debug
```

Presets: `x64`/`x86` combined with `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`, `ReleaseNoOptWithDeb` (no optimization, debug info, `NDEBUG` on) and `DebugASan` (AddressSanitizer). Output goes to `build/<preset>/`.

## Project Structure

```
include/mimosa/   public headers
src/                  implementation
examples/             small programs that use the library
tests/                test runner (registered with ctest)
docs/                 documentation and the logo
CMakePresets.json     build configurations
```

## License

The source code is licensed under the [MIT License](./LICENSE).

---

<div align="center">
  <p>Made with ❤️</p>
</div>
