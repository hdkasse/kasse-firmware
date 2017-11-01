
## Kasse Build Procedure

Download firmware source code from Kasse respository (https://github.com/HyundaiPay/kasse-firmware.git)
```
$ git clone git@github.com:HyundaiPay/kasse-firmware.git
```

### Build environment

Download arm embedded toolchain (https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)

Modify the makefile according to your build environment.
```
OS := $(shell uname -o)
ifeq ($(OS),Cygwin)
   BINPATH="c:/cygwin/5.4 2016q3/bin"
else
   BINPATH="/opt/toolchains/gcc-arm-none-eabi-5_4-2016q3/bin"
endif
```

### Command line build
MS Windows
```
build_win32.bat
```
Linux (Ubuntu, CentOS, etc..)
```
$ ./build.sh
```

The resultant binary will be located in ./build directory.


### License

If license is not specified in the header of a file, it can be assumed that it is licensed under GPLv3.
