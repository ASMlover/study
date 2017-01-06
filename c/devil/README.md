# **README for devil** #
***

## **What's devil?** ##
 * It's a pure C library.
 * It's the base library for C programming.
 * It's just for studing.
 * It's a free and open source program based FreeBSD, everyone can use it.

## **Building devil** ##
 * **Windows**
 ``` bat
  :: step1: Setting environment for using MSVC.
  :: step2: create build directory:
    \> cd devil
    \> mkdir cmake-build

  :: step3:
    \> cd cmake-build

    :: use Visual Studio
    \> cmake ..
    \> msbuild DevilPROJ.sln

    :: use NMake (default building type is `Debug`)
    \> cmake -G "NMake Makefiles" ..
    \> nmake

    :: build `Release` version
    \> cmake -G "NMake Makefiles" -D CMAKE_BUILD_TYPE=Release ..
    \> nmake
 ```

 * **Linux & macOS**
 ``` bash
    $ cd devil
    $ mkdir cmake-build
    $ cd cmake-build
    $ cmake ..
    $ make

    # build `Release` version
    $ cmake -D CMAKE_BUILD_TYPE=Release ..
    $ make
 ```
