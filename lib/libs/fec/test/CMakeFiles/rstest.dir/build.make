# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.20.0/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.20.0/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec

# Include any dependencies generated for this target.
include test/CMakeFiles/rstest.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/rstest.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/rstest.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/rstest.dir/flags.make

test/CMakeFiles/rstest.dir/rstest.o: test/CMakeFiles/rstest.dir/flags.make
test/CMakeFiles/rstest.dir/rstest.o: test/rstest.c
test/CMakeFiles/rstest.dir/rstest.o: test/CMakeFiles/rstest.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object test/CMakeFiles/rstest.dir/rstest.o"
	cd /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT test/CMakeFiles/rstest.dir/rstest.o -MF CMakeFiles/rstest.dir/rstest.o.d -o CMakeFiles/rstest.dir/rstest.o -c /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test/rstest.c

test/CMakeFiles/rstest.dir/rstest.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/rstest.dir/rstest.i"
	cd /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test/rstest.c > CMakeFiles/rstest.dir/rstest.i

test/CMakeFiles/rstest.dir/rstest.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/rstest.dir/rstest.s"
	cd /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test && /Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test/rstest.c -o CMakeFiles/rstest.dir/rstest.s

# Object files for target rstest
rstest_OBJECTS = \
"CMakeFiles/rstest.dir/rstest.o"

# External object files for target rstest
rstest_EXTERNAL_OBJECTS =

test/rstest: test/CMakeFiles/rstest.dir/rstest.o
test/rstest: test/CMakeFiles/rstest.dir/build.make
test/rstest: libfec.a
test/rstest: /Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk/usr/lib/libm.tbd
test/rstest: test/CMakeFiles/rstest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable rstest"
	cd /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rstest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/rstest.dir/build: test/rstest
.PHONY : test/CMakeFiles/rstest.dir/build

test/CMakeFiles/rstest.dir/clean:
	cd /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test && $(CMAKE_COMMAND) -P CMakeFiles/rstest.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/rstest.dir/clean

test/CMakeFiles/rstest.dir/depend:
	cd /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test /Users/sven/arbeit/programmiersprachen/CPP/webdabplus/src/libs/fec/test/CMakeFiles/rstest.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/rstest.dir/depend

