cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Nombre del proyecto y del mod
project(NeoBot VERSION 1.0.0)

# ============================================================
# Configuración de Geode
# ============================================================
# Si GEODE_SDK no está definido como variable de entorno, usar
# la ruta por defecto del SDK de Geode
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "La variable de entorno GEODE_SDK no está definida. "
            "Por favor, instala el Geode CLI y define GEODE_SDK.")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# ============================================================
# Archivos fuente del mod
# ============================================================
file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/manager/*.hpp
    src/hooks/*.hpp
    src/ui/*.hpp
)

# Crear la librería compartida (.so para Android)
add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

# ============================================================
# Configuración de compilación para Android ARM
# ============================================================
target_compile_options(${PROJECT_NAME} PRIVATE
    # Optimizaciones para ARM
    -O2
    # Suprimir warnings no críticos
    -Wno-unused-variable
    -Wno-unused-function
    # Compatibilidad con Android
    -fexceptions
    -frtti
)

# Definir que estamos en Android para los guards de plataforma
if (ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        GEODE_IS_ANDROID=1
    )
endif()

# ============================================================
# Configurar el mod con Geode
# ============================================================
setup_geode_mod(${PROJECT_NAME})
