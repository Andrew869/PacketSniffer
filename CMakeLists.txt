cmake_minimum_required(VERSION 3.15)
project(sniffer)

# Agrega todos los archivos .cpp en la carpeta src
file(GLOB SOURCES "*.cpp")

# Busca las bibliotecas ncurses, libpcap y libxlsxwriter
find_package(Curses REQUIRED)
find_library(PCAP_LIBRARY NAMES pcap)
find_package(ZLIB REQUIRED)
find_library(XLSXWRITER_LIBRARY NAMES xlsxwriter)

# Verifica que se hayan encontrado las bibliotecas necesarias
if (NOT CURSES_FOUND)
    message(FATAL_ERROR "ncurses library not found")
endif()

if (NOT PCAP_LIBRARY)
    message(FATAL_ERROR "libpcap library not found")
endif()

if (NOT XLSXWRITER_LIBRARY)
    message(FATAL_ERROR "libxlsxwriter library not found")
endif()

# Incluye los encabezados de ncurses y libxlsxwriter
include_directories(${CURSES_INCLUDE_DIRS} /usr/local/include/xlsxwriter)

# Crea el ejecutable usando los archivos fuente
add_executable(sniffer ${SOURCES})

# Vincula las bibliotecas necesarias
target_link_libraries(sniffer PRIVATE ${CURSES_LIBRARIES} ${PCAP_LIBRARY} ${XLSXWRITER_LIBRARY} /usr/lib/x86_64-linux-gnu/libz.so pthread)