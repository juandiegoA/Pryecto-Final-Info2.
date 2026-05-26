cmake_minimum_required(VERSION 3.21)

project(UltimateEnTRON VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(tron_logic
    src/logic/AgenteInteligente.cpp
    src/logic/BarreraEstatica.cpp
    src/logic/BarreraTemporizada.cpp
    src/logic/Checkpoint.cpp
    src/logic/Disco.cpp
    src/logic/DiscoEnemigo.cpp
    src/logic/Dron.cpp
    src/logic/Juego.cpp
    src/logic/Jugador.cpp
    src/logic/Nivel.cpp
    src/logic/NivelDefensaNucleo.cpp
    src/logic/NivelRutaTransmision.cpp
    src/logic/NodoCentralEnergia.cpp
    src/logic/Obstaculo.cpp
    src/logic/Posicion.cpp
    src/logic/Temporizador.cpp
)

target_include_directories(tron_logic PUBLIC include)

add_executable(ultimate_en_tron
    src/main.cpp
    src/gui/MainWindow.cpp
)

target_include_directories(ultimate_en_tron PRIVATE include)
target_link_libraries(ultimate_en_tron PRIVATE tron_logic Qt6::Widgets)

if(WIN32)
    set_target_properties(ultimate_en_tron PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

