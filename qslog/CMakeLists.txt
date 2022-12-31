# CMake build support courtesy of A.Gembe
cmake_minimum_required(VERSION 3.1)

project(QsLog)

# Add CMake modules
set(CMAKE_MODULE_PATH
    "${QsLog_SOURCE_DIR}/cmake"
    "${CMAKE_MODULE_PATH}"
)

include(QsLogConfigTargets)

# Add a _d to debug binaries
set(CMAKE_DEBUG_POSTFIX "_d")

# Qt5
find_package(Qt5Core REQUIRED)
set(CMAKE_AUTOMOC ON)

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${QsLog_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QsLog_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QsLog_BINARY_DIR}/bin)

set(HEADER_FILES
	QsLogDest.h
	QsLog.h
	QsLogDestConsole.h
	QsLogLevel.h
	QsLogDestFile.h
	QsLogDisableForThisFile.h
	QsLogDestFunctor.h
    )

set(SOURCE_FILES
	QsLogDest.cpp
	QsLog.cpp
	QsLogDestConsole.cpp
	QsLogDestFile.cpp
	QsLogDestFunctor.cpp
    )

option(QS_LOG_LINE_NUMBERS "Automatically writes the file and line for each log message" OFF)
if(QS_LOG_LINE_NUMBERS)
	add_definitions(-DQS_LOG_LINE_NUMBERS)
endif()

option(QS_LOG_DISABLE "Logging code is replaced with a no-op" OFF)
if(QS_LOG_DISABLE)
	add_definitions(-DQS_LOG_DISABLE)
endif()

option(QS_LOG_SEPARATE_THREAD "Messages are queued and written from a separate thread" OFF)
if(QS_LOG_SEPARATE_THREAD)
	add_definitions(-DQS_LOG_SEPARATE_THREAD)
endif()

option(QS_LOG_WIN_PRINTF_CONSOLE "Use fprintf instead of OutputDebugString on Windows" OFF)
if(QS_LOG_WIN_PRINTF_CONSOLE)
	add_definitions(-DQS_LOG_WIN_PRINTF_CONSOLE)
endif()

option(QS_LOG_IS_SHARED_LIBRARY "Build shared library" ON)
if(QS_LOG_IS_SHARED_LIBRARY)
	add_definitions(-DQSLOG_IS_SHARED_LIBRARY)
endif()

if(QS_LOG_IS_SHARED_LIBRARY)
	add_library(QsLog SHARED ${HEADER_FILES} ${SOURCE_FILES})
else()
	add_library(QsLog STATIC ${HEADER_FILES} ${SOURCE_FILES})
endif()

target_link_libraries(QsLog Qt5::Core)

install(FILES ${HEADER_FILES} DESTINATION include/QsLog)
QsLog_install_target(QsLog "")
