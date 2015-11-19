# **Linux编译安装GCC-5.1**
***

## **1. 下载GCC-5.1源码**
```
$ wget ftp://mirrors.kernel.org/gnu/gcc/gcc-5.1.0/gcc-5.1.0.tar.gz
```

## **2. 解压**
```
$ tar -xzvf gcc-5.1.0.tar.gz
```

## **3. 下载编译依赖项**
```
$ cd gcc-5.1.0
$ ./contrib/download_prerequires
$ cd ..
```

## **4. 建立编译输出目录**
```
$ mkdir gcc-build-5.1.0
```

## **5. 生成Makefile文件**
```
$ cd gcc-build-5.1.0
$ ../gcc-5.1.0/configure --enable-checking=release --enable-languages=c,c++ --disable-multilib
```

## **6. 编译**
```
$ make -j4
```

## **7. 安装**
  请切换到**root**用户
```
# make install
```

## **8. 检查当前GCC版本**
```
$ gcc -v
```

## **9. 切换到root用户, 替换新的GCC依赖库(libstdc++.so.6)**
  请首先进入gcc-build-5.1.0目录
```
# cp x86_64-unknown-linux-gnu/libstdc++-v3/src/.libs/libstdc++.so.6.0.21 /usr/lib/x86_64-linux-gnu/
# cd /usr/lib/x86_64-linux-gnu/
# rm libstdc++.so.6
# ln -s libstdc++.so.6.0.21 libstdc++.so.6
```
