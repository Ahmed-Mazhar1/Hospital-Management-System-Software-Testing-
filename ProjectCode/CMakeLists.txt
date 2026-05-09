cmake_minimum_required(VERSION 3.10)
project(HospitalManagementSystem CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hospital_management_system
    src/main.cpp
    src/HospitalRepository.cpp
    src/HospitalService.cpp
    src/HospitalConsole.cpp
)

target_include_directories(hospital_management_system PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
