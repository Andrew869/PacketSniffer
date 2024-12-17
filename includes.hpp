#include <arpa/inet.h>          // Manipulación de direcciones IP
#include <netinet/ip.h>         // Estructuras de cabeceras IP
#include <pcap/pcap.h>          // Captura de paquetes de red
#include <stdio.h>              // Entrada/salida estándar
#include <stdlib.h>             // Funciones estándar de C
#include <string.h>             // Funciones de manipulación de cadenas
#include <netinet/in.h>         // Estructuras de direcciones de red
#include <netinet/ip_icmp.h>    // Definiciones de ICMP
#include <netinet/tcp.h>        // Definiciones de TCP
#include <netinet/udp.h>        // Definiciones de UDP
#include <vector>               // Contenedor de datos dinámico
#include <pthread.h>            // Hilos de ejecución en C

#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>

#include <ncurses.h>            // Librería de interfaz de usuario en terminal
#include <sys/ioctl.h>          // Entradas/salidas del sistema
#include <signal.h>             // Manipulación de señales
#include <unistd.h>             // Funciones de sistema estándar POSIX
#include <cstdio>               // Funciones de entrada/salida de C++
#include <iostream>             // Flujo de entrada/salida estándar de C++
#include <ctime>                // Manipulación de fechas y tiempos
#include <cstdlib>              // Funciones de la biblioteca estándar de C
#include <chrono>               // Utilidades de tiempo en C++
#include <sstream>              // Para std::stringstream
#include <iomanip>              // Para std::setw
#include <memory>
#include <any>
#include <map>
#include <cstdarg>
#include<fstream>
#include <xlsxwriter.h>

using namespace std;
using namespace chrono;