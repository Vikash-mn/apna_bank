cmake_minimum_required(VERSION 3.10)
project(BankingSystem VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Source files
set(SOURCES
    src/main.cpp
    src/banking.cpp
    src/utils.cpp
    src/security.cpp
    src/file_operations.cpp
)

# Include directories
include_directories(src)

# Create executable
add_executable(banking_system ${SOURCES})

# Create data directory
add_custom_target(create_data_dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/data
    COMMENT "Creating data directory"
)

# Installation
install(TARGETS banking_system DESTINATION bin)
install(DIRECTORY data/ DESTINATION data)