# CMakeList.txt: Uwe 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Uwe")

# 将源代码添加到此项目的可执行文件。
add_executable (Uwe)
target_sources(Uwe PRIVATE src/main.cpp)
target_sources(
    Uwe
    PRIVATE FILE_SET CXX_MODULES FILES "modules/lexer.cppm" "modules/parser.cppm" "modules/vm.cppm" "modules/logger.cppm")
set_property(TARGET Uwe PROPERTY CXX_STANDARD 20)

# TODO: 如有需要，请添加测试并安装目标。
