cmake_minimum_required(VERSION 3.15)
project(drogon_backend CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Drogon (installed to C:/msys64/mingw64)
find_package(Drogon CONFIG REQUIRED)

# Add our executable
add_executable(${PROJECT_NAME} main.cc)

# Link Drogon to our executable
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
