# **CMake**
***

## **CMake构建指定x86或x64**
**1. Windows**

以VS2017 x64编译器为例，cmake的命令如下：
```shell
  # for x64
  \> cmake -G "Visual Studio 15 Win64" ..

  # for x86
  \> cmake -G "Visual Studio 15" ..
```
或者直接使用命令参数-A来制定架构（x64为64bit，不指定默认为x86）
```shell
  \> make -A x64 ..
```
使用nmake指令的情况需要使用vcvarsall.bat来指定x86或x64
```shell
  # for x86
  \> vcvarsall.bat x86
  \> cmake -G "NMake Makefiles" ..
  \> nmake

  # for x64
  \> vcvarsall.bat x64
  \> cmake -G "NMake Makefiles" ..
  \> nmake
```

**2. Linux**
```shell
  # 设置CFLAGS为-m32或-m64即可

  $ export CFLAGS=-m32
  ...
  $ export CFLAGS=-m64
```

**3. macOS**
```shell
  # for x86
  $ cmake -DCMAKE_OSX_ARCHITECTURES=i386 ..

  # for x64
  $ cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 ..
```
