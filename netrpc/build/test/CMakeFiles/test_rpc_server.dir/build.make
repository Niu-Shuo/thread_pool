# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/niushuo/cpp/thread_pool/netrpc

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/niushuo/cpp/thread_pool/netrpc/build

# Include any dependencies generated for this target.
include test/CMakeFiles/test_rpc_server.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/test_rpc_server.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test_rpc_server.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test_rpc_server.dir/flags.make

test/CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o: test/CMakeFiles/test_rpc_server.dir/flags.make
test/CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o: ../test/test_rpc_server.cc
test/CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o: test/CMakeFiles/test_rpc_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/niushuo/cpp/thread_pool/netrpc/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o"
	cd /home/niushuo/cpp/thread_pool/netrpc/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o -MF CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o.d -o CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o -c /home/niushuo/cpp/thread_pool/netrpc/test/test_rpc_server.cc

test/CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.i"
	cd /home/niushuo/cpp/thread_pool/netrpc/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/niushuo/cpp/thread_pool/netrpc/test/test_rpc_server.cc > CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.i

test/CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.s"
	cd /home/niushuo/cpp/thread_pool/netrpc/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/niushuo/cpp/thread_pool/netrpc/test/test_rpc_server.cc -o CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.s

test/CMakeFiles/test_rpc_server.dir/order.pb.cc.o: test/CMakeFiles/test_rpc_server.dir/flags.make
test/CMakeFiles/test_rpc_server.dir/order.pb.cc.o: ../test/order.pb.cc
test/CMakeFiles/test_rpc_server.dir/order.pb.cc.o: test/CMakeFiles/test_rpc_server.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/niushuo/cpp/thread_pool/netrpc/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object test/CMakeFiles/test_rpc_server.dir/order.pb.cc.o"
	cd /home/niushuo/cpp/thread_pool/netrpc/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/test_rpc_server.dir/order.pb.cc.o -MF CMakeFiles/test_rpc_server.dir/order.pb.cc.o.d -o CMakeFiles/test_rpc_server.dir/order.pb.cc.o -c /home/niushuo/cpp/thread_pool/netrpc/test/order.pb.cc

test/CMakeFiles/test_rpc_server.dir/order.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_rpc_server.dir/order.pb.cc.i"
	cd /home/niushuo/cpp/thread_pool/netrpc/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/niushuo/cpp/thread_pool/netrpc/test/order.pb.cc > CMakeFiles/test_rpc_server.dir/order.pb.cc.i

test/CMakeFiles/test_rpc_server.dir/order.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_rpc_server.dir/order.pb.cc.s"
	cd /home/niushuo/cpp/thread_pool/netrpc/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/niushuo/cpp/thread_pool/netrpc/test/order.pb.cc -o CMakeFiles/test_rpc_server.dir/order.pb.cc.s

# Object files for target test_rpc_server
test_rpc_server_OBJECTS = \
"CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o" \
"CMakeFiles/test_rpc_server.dir/order.pb.cc.o"

# External object files for target test_rpc_server
test_rpc_server_EXTERNAL_OBJECTS =

../bin/test_rpc_server: test/CMakeFiles/test_rpc_server.dir/test_rpc_server.cc.o
../bin/test_rpc_server: test/CMakeFiles/test_rpc_server.dir/order.pb.cc.o
../bin/test_rpc_server: test/CMakeFiles/test_rpc_server.dir/build.make
../bin/test_rpc_server: ../lib/libnetrpc.a
../bin/test_rpc_server: test/CMakeFiles/test_rpc_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/niushuo/cpp/thread_pool/netrpc/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../bin/test_rpc_server"
	cd /home/niushuo/cpp/thread_pool/netrpc/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_rpc_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test_rpc_server.dir/build: ../bin/test_rpc_server
.PHONY : test/CMakeFiles/test_rpc_server.dir/build

test/CMakeFiles/test_rpc_server.dir/clean:
	cd /home/niushuo/cpp/thread_pool/netrpc/build/test && $(CMAKE_COMMAND) -P CMakeFiles/test_rpc_server.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test_rpc_server.dir/clean

test/CMakeFiles/test_rpc_server.dir/depend:
	cd /home/niushuo/cpp/thread_pool/netrpc/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/niushuo/cpp/thread_pool/netrpc /home/niushuo/cpp/thread_pool/netrpc/test /home/niushuo/cpp/thread_pool/netrpc/build /home/niushuo/cpp/thread_pool/netrpc/build/test /home/niushuo/cpp/thread_pool/netrpc/build/test/CMakeFiles/test_rpc_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test_rpc_server.dir/depend

