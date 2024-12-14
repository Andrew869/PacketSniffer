cmake_minimum_required(VERSION 3.15)
project(sniffer)

# Agrega todos los archivos .cpp en la carpeta src
file(GLOB SOURCES "*.cpp")

# Busca las bibliotecas ncurses y libpcap
find_package(Curses REQUIRED)
find_library(PCAP_LIBRARY NAMES pcap)

# Verifica que se hayan encontrado las bibliotecas
if (CURSES_FOUND)
    include_directories(${CURSES_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "ncurses library not found")
endif ()

if (PCAP_LIBRARY)
    # Crea el ejecutable usando los archivos fuente
    add_executable(sniffer ${SOURCES})
    target_link_libraries(sniffer ${CURSES_LIBRARIES} ${PCAP_LIBRARY})
else ()
    message(FATAL_ERROR "libpcap library not found")
endif ()