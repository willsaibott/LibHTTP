#!/bin/sh

doxygen docs/config

coverage/generate_report.sh

rm -rf binaries build

qmake -qt=qt5 libhttp.pro -spec linux-clang CONFIG+=qml_debug CONFIG+=qtquickcompiler CONFIG+=force_debug_info CONFIG+=separate_debug_info CONFIG+=debug && /usr/bin/make qmake_all

make clean -j8

make -j8

qmake -qt=qt5 libhttp.pro -spec linux-clang CONFIG+=qtquickcompiler && /usr/bin/make qmake_all

make clean -j8

make -j8

valgrind --child-silent-after-fork=yes --smc-check=stack --tool=memcheck --gen-suppressions=all --track-origins=yes --leak-check=full --num-callers=25 --error-exitcode=1 'binaries/linux/clang/x64/debug/Test'

valgrind --child-silent-after-fork=yes --smc-check=stack --tool=memcheck --gen-suppressions=all --track-origins=yes --leak-check=full --num-callers=25 --error-exitcode=1 'binaries/linux/clang/x64/release/Test'
