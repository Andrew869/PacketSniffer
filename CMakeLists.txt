# cmake_minimum_required(VERSION 3.15)
# project(sniffer)

# # Agrega todos los archivos .cpp en la carpeta src
# file(GLOB SOURCES "*.cpp")

# # Busca las bibliotecas ncurses y libpcap
# find_package(Curses REQUIRED)
# find_library(PCAP_LIBRARY NAMES pcap)

# # Verifica que se hayan encontrado las bibliotecas
# if (CURSES_FOUND)
#     include_directories(${CURSES_INCLUDE_DIRS})
# else ()
#     message(FATAL_ERROR "ncurses library not found")
# endif ()

# if (PCAP_LIBRARY)
#     # Crea el ejecutable usando los archivos fuente
#     add_executable(sniffer ${SOURCES})
#     target_link_libraries(sniffer ${CURSES_LIBRARIES} ${PCAP_LIBRARY})
# else ()
#     message(FATAL_ERROR "libpcap library not found")
# endif ()


cmake_minimum_required(VERSION 3.15)
project(sniffer)

# Agrega todos los archivos .cpp en la carpeta src
file(GLOB SOURCES "*.cpp")

# Busca las bibliotecas ncurses, libpcap y libxlsxwriter
find_package(Curses REQUIRED)
find_library(PCAP_LIBRARY NAMES pcap)
<<<<<<< HEAD
find_package(ZLIB REQUIRED)
=======
>>>>>>> origin/testing
find_library(XLSXWRITER_LIBRARY NAMES xlsxwriter)

# Verifica que se hayan encontrado las bibliotecas necesarias
if (NOT CURSES_FOUND)
    message(FATAL_ERROR "ncurses library not found")
endif()

if (NOT PCAP_LIBRARY)
    message(FATAL_ERROR "libpcap library not found")
<<<<<<< HEAD
endif()

if (NOT XLSXWRITER_LIBRARY)
    message(FATAL_ERROR "libxlsxwriter library not found")
endif()

# Incluye los encabezados de ncurses y libxlsxwriter
include_directories(${CURSES_INCLUDE_DIRS} /usr/local/include/xlsxwriter)
=======
endif ()

if (NOT XLSXWRITER_LIBRARY)
    message(FATAL_ERROR "libxlsxwriter library not found")
endif ()

# Incluye los encabezados de ncurses
include_directories(${CURSES_INCLUDE_DIRS})
>>>>>>> origin/testing

# Crea el ejecutable usando los archivos fuente
add_executable(sniffer ${SOURCES})

# Vincula las bibliotecas necesarias
<<<<<<< HEAD
target_link_libraries(sniffer PRIVATE ${CURSES_LIBRARIES} ${PCAP_LIBRARY} ${XLSXWRITER_LIBRARY} /usr/lib/x86_64-linux-gnu/libz.so pthread)
=======
target_link_libraries(sniffer ${CURSES_LIBRARIES} ${PCAP_LIBRARY} ${XLSXWRITER_LIBRARY})
>>>>>>> origin/testing
