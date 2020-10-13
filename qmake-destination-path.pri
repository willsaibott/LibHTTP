platform_path  = unknown-platform
compiler_path  = unknown-compiler
processor_path = unknown-processor
build_path     = unknown-build

PLATFORM_WIN {
  platform_path = windows
}

PLATFORM_LINUX {
  platform_path = linux
}

COMPILER_CLANG {
  compiler_path = clang
}

COMPILER_GCC {
  compiler_path = gcc
}

COMPILER_MSVC2019 {
  compiler_path = vs19
}

PROCESSOR_x64 {
  processor_path = x64
}

PROCESSOR_x86 {
  processor_path = x86
}

BUILD_DEBUG {
  build_path = debug
}
else {
  build_path = release
}

DESTINATION_PATH = $$platform_path/$$compiler_path/$$processor_path/$$build_path
!build_pass:message(Dest path: $${DESTINATION_PATH})
