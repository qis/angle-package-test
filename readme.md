# Simple OpenGL ES 3.0 Test Project
The following steps can be used to setup [vcpkg][vcpkg] after installing Visual Studio 2017 and Git.

```cmd
mkdir C:\Libraries
cd C:\Libraries && git clone https://github.com/Microsoft/vcpkg vcpkg
cd vcpkg && bootstrap-vcpkg.bat && vcpkg integrate install && exit
```

Add `C:\Libraries\vcpkg` and `C:\Libraries\vcpkg\installed\x64-windows\bin` to the `PATH` environment variable.<br>
Set the `VCPKG` environment variable to `C:/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake`.<br>
Set the `VCPKG_DEFAULT_TRIPLET` environment variable to `x64-windows`.<br>

```cmd
vcpkg install angle
```

## Build
Verify the contents of `solution.cmd`, execute it to generate a VS project and build the application.

[vcpkg]: https://github.com/Microsoft/vcpkg
