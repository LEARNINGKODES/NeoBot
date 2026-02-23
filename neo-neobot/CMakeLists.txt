cmake_minimum_required(VERSION 3.10)

project(NeoBot VERSION 1.0.0)

find_package(Geode REQUIRED)

geode_add_mod(
    ${PROJECT_NAME}
    src/main.cpp
    src/MacroManager.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    src
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    Geode::full
)

# Añadir soporte para nlohmann/json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
