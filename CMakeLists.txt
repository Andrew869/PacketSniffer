cmake_minimum_required(VERSION 3.15)
project(packetsniffer)

# Agrega todos los archivos .cpp en la carpeta src
file(GLOB SOURCES "*.cpp")

# Busca las bibliotecas ncurses, libpcap y libxlsxwriter
find_package(Curses REQUIRED)
find_library(PCAP_LIBRARY NAMES pcap)
find_library(XLSXWRITER_LIBRARY NAMES xlsxwriter)

# Verifica que se hayan encontrado las bibliotecas necesarias
if (NOT CURSES_FOUND)
    message(FATAL_ERROR "ncurses library not found")
endif ()

if (NOT PCAP_LIBRARY)
    message(FATAL_ERROR "libpcap library not found")
endif ()

if (NOT XLSXWRITER_LIBRARY)
    message(FATAL_ERROR "libxlsxwriter library not found")
endif ()

# Incluye los encabezados de ncurses
include_directories(${CURSES_INCLUDE_DIRS})

# Crea el ejecutable usando los archivos fuente
add_executable(packetsniffer ${SOURCES})

# Vincula las bibliotecas necesarias
target_link_libraries(packetsniffer ${CURSES_LIBRARIES} ${PCAP_LIBRARY} ${XLSXWRITER_LIBRARY})

# Configuración de instalación del ejecutable
install(TARGETS packetsniffer RUNTIME DESTINATION /usr/local/bin)

# Configuración para instalar el archivo .desktop
install(FILES resources/packetsniffer.desktop DESTINATION /usr/share/applications)

# Instala un ícono si está disponible (opcional)
# install(FILES resources/packetsniffer.png DESTINATION /usr/share/icons/hicolor/48x48/apps)

# Mensaje de éxito al finalizar la instalación
message(STATUS "CMake configuration is complete. To install, run: sudo make install")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
