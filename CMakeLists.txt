cmake_minimum_required(VERSION 3.16)
project(ElectroMart LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(ElectroMart
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/Store.cpp
    src/Item.h
    src/Inventory.h
    src/Bill.h
)

target_link_libraries(ElectroMart PRIVATE Qt6::Widgets)
