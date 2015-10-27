Windows - Visual Studio 2013
============================
#### Prerequisites ####
* Microsoft Visual C++ 2013 Update 1 (the free Express edition will work)
* If you have multiple MSVS installation use MSVS Developer console from target version.
* This build is for 64bit binaries.

#### Set up the directory structure####
* Create a base directory for all projects.  I'm putting everything in
  `D:\Graphene`, you can use whatever you like.  In several of the batch files
  and makefiles, this directory will be referred to as `GRA_ROOT`:

  ```
mkdir D:\Graphene
```

* Clone the Graphene repository

  ```
cd D:\Graphene
git clone https://github.com/cryptonomex/graphene.git
cd Graphene
git submodule update --init --recursive
```

* Dowload CMake

  Download the latest *Win32 Zip* build CMake from
  http://cmake.org/cmake/resources/software.html (version 2.8.12.2 as of this
  writing).  Unzip it to your base directory, which will create a directory that
  looks something like `D:\Graphene\cmake-2.8.12.2-win32-x86`.  Rename this
  directory to `D:\Graphene\CMake`.

  If you already have CMake installed elsewhere on your system you can use it,
  but Graphene has a few batch files that expect it to be in the base
  directory's `CMake` subdirectory, so those scripts would need tweaking.

* Download library dependencies:
   You can get prebuilt packages containing some of the libraries described below built
   by using currently used compiler. Download the appropriate package from one of the links below.
 
   64bit version:    http://get.syncad.com/invictus/Bitshare_toolkit_prebuilt_libs-vs2013-x64.7z

   and should be unpacked into Bitshare-root directory (ie: D:\Graphene).
   Now the package contains: BerkeleyDB, OpenSSL 1.0.1g.

 * BerkeleyDB

   Graphene depends on BerkeleyDB 12c Release 1 (12.1.6.0.20).  You can build
   this from source or download our pre-built binaries to speed things up.

 * Boost

   Graphene depends on the Boost libraries version 1.57.  You can build them from
   source.
   * download the latest boost source from http://www.boost.org/users/download/
   * unzip it to the base directory `D:\Graphene`.
   * This will create a directory like `D:\Graphene\boost_1_57_0`.

 * OpenSSL

   Graphene depends on OpenSSL, and you must build this from source.
    * download the latest OpenSSL source from http://www.openssl.org/source/
    * Untar it to the base directory `D:\Graphene`
    * this will create a directory like `D:\Graphene\openssl-1.0.1g`.

At the end of this, your base directory should look like this (directory names will
be slightly different for the 64bit versions):
```
D:\Graphene
+- BerkeleyDB
+- Graphene
+- boost_1_57_0
+- CMake
+- OpenSSL
```

#### Build the library dependencies - Skip downloaded prebuilt binary package(s) ####

* Set up environment for building:
cd D:\Graphene

  ```
notepad setenv_x64.bat

@echo off
set GRA_ROOT=d:\graphene\graphene
set OPENSSL_ROOT=%GRA_ROOT%\OpenSSL
set OPENSSL_ROOT_DIR=%OPENSSL_ROOT%
set OPENSSL_INCLUDE_DIR=%OPENSSL_ROOT%\include
set DBROOTDIR=%GRA_ROOT%\BerkeleyDB
set ICUROOT=%GRA_ROOT%\ICU
set BOOST_ROOT=%GRA_ROOT%\boost_1_57_0
set TCL_ROOT=%GRA_ROOT%\tcl
set NPM_INSTALL_PREFIX=%GRA_ROOT%\npm

set PATH=%NPM_INSTALL_PREFIX%;"%APPDATA%"\npm;%QTDIR%\bin;%ICUROOT%\bin;%ICUROOT%\lib;%GRA_ROOT%\bin;%TCL_ROOT%\bin;%GRA_ROOT%\Cmake\bin;%BOOST_ROOT%\lib;%PATH%

echo Setting up VS2013 environment...
call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64


setenv_x64.bat
```


* Build OpenSSL DLLs
  ```
cd D:\Graphene\openssl-1.0.1g
perl Configure --openssldir=D:\Graphene\OpenSSL VC-WIN64A
ms\do_ms.bat
nmake -f ms\ntdll.mak
nmake -f ms\ntdll.mak install
```

  This will create the directory `D:\Graphene\OpenSSL` with the libraries, DLLs,
  and header files.

#### Build project files for Graphene ####

* Run CMake:
cd D:\Graphene\Graphene

  ```
notepad run_cmake_x64.bat

setlocal
call "d:\graphene\setenv_x64.bat"
cd %GRA_ROOT%
cmake-gui -G "Visual Studio 12"

run_cmake_x64.bat
```

 This pops up the cmake gui, but if you've used CMake before it will probably be
 showing the wrong data, so fix that:
 * Where is the source code: `D:\Graphene\Graphene`
 * Where to build the binaries: `D:\Graphene\x64` 

 Then hit **Configure**.  It may ask you to specify a generator for this
 project; if it does, choose **Visual Studio 12 2013 Win64** for 64 bit builds and select **Use default
 native compilers**.  Look through the output and fix any errors.  Then
 hit **Generate**.


* Launch *Visual Studio* and load `D:\Graphene\Graphene.sln` 
* Set Active Configuration to RelWithDebInfo, ensure Active Solution platform is x64 for 64 bit builds

* Click on each of the VS projects and add additional compiling option '/bigobj'

* *Build Solution*

Or you can build the `INSTALL` target in Visual Studio which will
copy all of the necessary files into your `D:\Graphene\install`
directory, then copy all of those files to the `bin` directory.
