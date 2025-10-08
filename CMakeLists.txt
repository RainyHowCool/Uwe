# CMakeList.txt: Uwe 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# 针对 MSVC 的特定设置
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Uwe")

# 针对 Linux + GCC 13 的设置
if (UNIX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13)
    # 启用 C++20 模块支持（GCC 13 及以上）
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")
    # 可选：开启更严格的警告
    target_compile_options(Uwe PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endif()

# 将源代码添加到此项目的可执行文件。
add_executable (Uwe)
target_sources(Uwe PRIVATE src/main.cpp "src/ir.cpp" )
target_sources(
    Uwe
    PRIVATE FILE_SET CXX_MODULES FILES "modules/lexer.cppm" "modules/parser.cppm" "modules/vm.cppm" "modules/logger.cppm")
set_property(TARGET Uwe PROPERTY CXX_STANDARD 20)

# TODO: 如有需要，请添加测试并安装目标。