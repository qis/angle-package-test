# Simple OpenGL ES 3.0 Test Project
This project is designed to test my [angle-package][angle-package] fork of [vcpkg][vcpkg].

## Setup
The following steps can be used to setup [vcpkg][vcpkg] after installing Visual Studio 2017 and Git.

```cmd
mkdir C:\Libraries
cd C:\Libraries && git clone https://github.com/qis/angle-package vcpkg
cd vcpkg && bootstrap-vcpkg.bat && vcpkg integrate install && exit
```

Add `C:\Libraries\vcpkg` to the `PATH` environment variable.<br>
Set the `VCPKG` environment variable to `C:/Libraries/vcpkg/scripts/buildsystems/vcpkg.cmake`.<br>
Set the `VCPKG_DEFAULT_TRIPLET` environment variable to `x64-windows`.<br>

```cmd
vcpkg install angle
```

## Build
Verify the contents of `solution.cmd`, execute it and build the application.

[angle-package]: https://github.com/qis/angle-package
[vcpkg]: https://github.com/Microsoft/vcpkg
