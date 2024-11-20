#include <arpa/inet.h>
#include <netinet/ip.h>
#include <pcap/pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <vector>

#include "display.hpp"

using namespace std;

class packetData {
public:
    const u_char *buffer;
    int length;

    packetData(const u_char* buffer, int length) {
        this->buffer = buffer;
        this->length = length;
    }
private:
};

int link_hdr_length = 0;
window *win1;
vector<packetData> packets;
int max_y, max_x;

void InitDisplay(int &height, int &width);
void EndDisplay();

void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr) {
    packetData data(packetd_ptr, pkthdr->len);
    packets.push_back(data);

    int length = packets.size();
    int index = 0;
    
    int cur_y = length < win1->height - 2 ? length : win1->height - 2;
    for (size_t y = 1; y <= cur_y; y++) {
        index = length - y;

        const struct ip* ip_header;   // Estructura para el encabezado IP
        int ip_header_length;         // Longitud del encabezado IP

        // Saltar el encabezado Ethernet si es necesario (usualmente 14 bytes)

        // Obtener el encabezado IP desde los datos del paquete
        ip_header = (struct ip*)(packets[index].buffer + 14);  // +14 para saltar el encabezado Ethernet (si presente)
        ip_header_length = ip_header->ip_hl * 4; // Longitud del encabezado IP en bytes (ip_hl está en palabras de 4 bytes)

        // Obtener direcciones IP de origen y destino
        char source_ip[INET_ADDRSTRLEN];
        char dest_ip[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(ip_header->ip_src), source_ip, INET_ADDRSTRLEN); // IP de origen
        inet_ntop(AF_INET, &(ip_header->ip_dst), dest_ip, INET_ADDRSTRLEN);   // IP de destino

        // Obtener protocolo
        int protocol = ip_header->ip_p; // Campo ip_p contiene el número de protocolo (TCP, UDP, etc.)
        string protText;
        switch (protocol) {
            case IPPROTO_TCP:
                protText = "TCP";
                break;
            case IPPROTO_UDP:
                protText = "UDP";
                break;
            case IPPROTO_ICMP:
                protText = "ICMP";
                break;
            case IPPROTO_IP:
                protText = "IP";
                break;
            default:
                protText = "?";
                break;
        }

        // Obtener tamaño del paquete
        int packet_size = ntohs(ip_header->ip_len);

        mvwprintw(win1->win, y, 1, "%20s\t%20s\t%20s\t%20d", source_ip, dest_ip, protText.c_str(), packet_size);
    }
    win1->refresh();
}

int main(int argc, char const *argv[]) {
    pcap_if_t *alldevsp , *device;
    char errbuf[100] , *devname , devs[100][100];
    int count = 0;

    int packets_count = -1;
    // char filters[100] = "port 80";InitDisplay
    // char filters[100] = "dst host 192.168.0.15 and udp";
    char filters[100] = "";

    if( pcap_findalldevs( &alldevsp , errbuf) ) {
		printf("Error finding devices : %s" , errbuf);
		exit(1);
	}

    for(device = alldevsp ; device != NULL ; device = device->next) {
		if(device->name != NULL) {
			strcpy(devs[count] , device->name);
		}
		count++;
	}

    char error_buffer[PCAP_ERRBUF_SIZE];
    devname = devs[0];
    pcap_t *capdev = pcap_open_live(devname, 65535, 1, -1, error_buffer);

    int link_hdr_type = pcap_datalink(capdev);

    if (capdev == NULL) {
        printf("ERR: pcap_open_live() %s\n", error_buffer);
        exit(1);
    }

    switch (link_hdr_type) {
        case DLT_NULL:
            link_hdr_length = 4;
            break;
        case DLT_EN10MB:
            link_hdr_length = 14;
            break;
        default:
            link_hdr_length = 0;
    }

    struct bpf_program bpf;
    bpf_u_int32 netmask;

    if (pcap_compile(capdev, &bpf, filters, 0, netmask) == PCAP_ERROR) {
        printf("ERR: pcap_compile() %s", pcap_geterr(capdev));
    }

    if (pcap_setfilter(capdev, &bpf)) {
        printf("ERR: pcap_setfilter() %s", pcap_geterr(capdev));
    }

    InitDisplay(max_y, max_x);
    refresh();
    win1 = new window(max_y / 2, max_x, 0, 0);
    win1->init();
    win1->refresh();

    if (pcap_loop(capdev, packets_count, packetManager, (u_char *)NULL)) {
        // printf("ERR: pcap_loop() failed!\n");
        exit(1);
    }

    EndDisplay();

    return 0;
}