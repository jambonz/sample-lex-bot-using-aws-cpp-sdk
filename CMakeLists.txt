cmake_minimum_required(VERSION 3.10)
project(lex_app)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)

add_executable(lex_app main.cc)

# Add these two lines
find_package(ZLIB REQUIRED)
if (NOT ZLIB_FOUND)
    message(FATAL_ERROR "ZLIB not found!")
endif()

find_package(AWSSDK REQUIRED COMPONENTS lexv2-runtime transcribestreaming)
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Use the PRIVATE keyword for all libraries
target_link_libraries(lex_app PRIVATE ${AWSSDK_LINK_LIBRARIES})
target_link_libraries(lex_app PRIVATE Boost::system Boost::filesystem)

# Copy caller.r8 to the build directory
configure_file(I-would-like-to-book-a-flight.r8 I-would-like-to-book-a-flight.r8 COPYONLY)