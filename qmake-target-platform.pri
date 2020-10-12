win32 {
    CONFIG += PLATFORM_WIN
    !build_pass:message(PLATFORM_WIN)
    win32-g++ {
        CONFIG += COMPILER_GCC
        !build_pass:message(COMPILER_GCC)
    }
    win32-msvc2019 {
        CONFIG += COMPILER_MSVC2019
        !build_pass:message(COMPILER_MSVC2019)
        win32-msvc2019:QMAKE_TARGET.arch = x86_64
    }
}

linux {
  CONFIG += PLATFORM_LINUX
  !build_pass:message(PLATFORM_LINUX)
  # Make QMAKE_TARGET arch available for linux
  !contains(QT_ARCH, x86_64) {
    QMAKE_TARGET.arch = x86
  }
  else {
    QMAKE_TARGET.arch = x86_64
  }

  linux-clang {
    CONFIG += COMPILER_CLANG
    !build_pass:message(COMPILER_CLANG)
  }
}

contains(QMAKE_TARGET.arch, x86_64) {
  CONFIG += PROCESSOR_x64
  !build_pass:message(PROCESSOR_x64)
}
else {
  CONFIG += PROCESSOR_x86
  !build_pass:message(PROCESSOR_x86)
}

CONFIG(debug, release|debug) {
  CONFIG += BUILD_DEBUG
  !build_pass:message(BUILD_DEBUG)
}
else {
  CONFIG += BUILD_RELEASE
  !build_pass:message(BUILD_RELEASE)
}


