
#!/bin/sh

############################################################################################################
# You should edit the following 3 paths when necessary
############################################################################################################

# Get the path to the current folder
SCRIPT_DIR=$(dirname $0)
BUILD=${1:-"debug"}
PLATFORM=${2:-"x64"}
COMPILER=${3:-"clang"}
SYSTEM=${4:-"linux"}

rm -f $SCRIPT_DIR/../build$$DESTINATION_PATH/.obj/*gcda
$SCRIPT_DIR/../binaries/$SYSTEM/$COMPILER/$PLATFORM/$BUILD/Test

# SRC_DIR is the directory containing the .gcno files (%{buildDir} in Qt Creator)
SRC_DIR="$SCRIPT_DIR/../build/$SYSTEM/$COMPILER/$PLATFORM/$BUILD/.obj/"

# COV_DIR is the directory where the coverage results will be stored
COV_DIR="$SCRIPT_DIR/coverage"

############################################################################################################

# Path where the HTML files should be saved
HTML_RESULTS="${COV_DIR}""/html"

# Create the html folder if it does not exists
mkdir -p ${HTML_RESULTS}

# Generate our initial info
lcov --directory "${SRC_DIR}" --base-directory "${SCRIPT_DIR}" --capture -o "${COV_DIR}/coverage.info" --gcov-tool ${SCRIPT_DIR}/llvm-gcov.sh

# Remove some paths/files which we don't want to calculate the code coverage (e.g. third party libraries) and generate a new coverage file filtered (feel free to edit it when necessary)
lcov -r "${COV_DIR}/coverage.info"  "*/googletest/*"  "*Qt*.framework*"  "*/tests/*" "*Xcode.app*" "*.moc" "*moc_*.cpp" "*/test/*" "*/build*/*" -o "${COV_DIR}/coverage-filtered.info" "*/QtCore/*" "*/QtTest*" "/usr/include/*" "*/Test/*"

# Generate the HTML files
genhtml -o "${HTML_RESULTS}" "${COV_DIR}/coverage-filtered.info"

# Reset our counts
lcov -d "${COV_DIR}" -z

# Open the index.html
google-chrome "${HTML_RESULTS}/index.html"