#include "includes.hpp"
#include "display.hpp"

#define cmvprintw(c, y, x, format, ...) \
    do { \
        attron(COLOR_PAIR(c)); \
        mvprintw(y, x, format, ##__VA_ARGS__); \
        attroff(COLOR_PAIR(c)); \
    } while (0)

using namespace std;

class packetData {
public:
    duration<double> elapsed_seconds;
    vector<u_char> data;
    bpf_u_int32 length;

    packetData(duration<double> elapsed_seconds, const u_char* packet_ptr, bpf_u_int32 length) : length(length)  {
        this->elapsed_seconds = elapsed_seconds;
        data.assign(packet_ptr, packet_ptr + length);
    }
private:
};

class SelectList {
public:
    int index;
    int length;

};

int link_hdr_length = 0;
window *win1;
vector<packetData> packets;
int max_y, max_x;
int packetCount = 0;
steady_clock::time_point start_time, end_time;
string selected_protocol="ALL";


int current_selection = 0;
int scroll_start = 0;

int isAutoScroll = true;


void InitDisplay(int &height, int &width);
void EndDisplay();
void *threadpcap(void *arg);
void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr);
void draw_list();
void move_selection(int direction);
void select_protocol();

int main(int argc, char const *argv[]) {
    pcap_if_t *alldevsp , *device;
    char errbuf[100] , *devname , devs[100][100];
    int count = 0;

    // int packets_count = -1;
    // char filters[100] = "port 80";InitDisplay
    // char filters[] = "tcp port 22";
    char filters[] = "";

    InitDisplay(max_y, max_x);
    refresh();

    if(pcap_findalldevs(&alldevsp ,errbuf)) {
		// printf("Error finding devices : %s" , errbuf);
		exit(1);
	}

    for(device = alldevsp ; device != NULL ; device = device->next) {
		if(device->name != NULL) {
			strcpy(devs[count] , device->name);
		}
		count++;
	}

    int keyPressed = ' ';
    int index = 0;

    while(keyPressed != '\n'){
        keyPressed = getch();
        switch (keyPressed) {
            case 'W':
            case 'w':
            case KEY_UP:
                index--;
                break;
            case 'S':
            case 's':
            case KEY_DOWN:
                index++;
                break;
            case '\n':
                // mvprintw(max_y - 2, max_x - 1, "Selected!");
                devname = devs[index];
                break;
        }

        if(index >= count)
            index = 0;
        else if(index < 0)
            index = count - 1;

        for (size_t i = 0; i < count; i++) {
            if(i == index)
                cmvprintw(1 ,i, 0, "%-20s", devs[i]);
            else
                mvprintw(i, 0, "%-20s", devs[i]);
        }
        mvprintw(max_y - 1, max_x - 1, "%d", index);
    }
    select_protocol();
    char error_buffer[PCAP_ERRBUF_SIZE];
    // devname = devs[0];
    pcap_t *capdev = pcap_open_live(devname, 65535, 1, -1, error_buffer);

    int link_hdr_type = pcap_datalink(capdev);

    if (capdev == NULL) {
        // printf("ERR: pcap_open_live() %s\n", error_buffer);
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
    clear();
    refresh();
    win1 = new window(max_y / 2, 66, 0, 0);
    win1->init();

    start_time = steady_clock::now();
    // if (pcap_loop(capdev, packets_count, packetManager, (u_char *)NULL)) {
    //     printf("ERR: pcap_loop() failed!\n");
    //     exit(1);
    // }
    pthread_t captureThread;
    int threadError = pthread_create(&captureThread, NULL, threadpcap, (void *)capdev);
    if (threadError) {
        fprintf(stderr, "Error al crear el hilo: %d\n", threadError);
        return 1;
    }

    int key;
    while ((key = getch()) != 'q') {
        switch (key) {
            case KEY_UP:
                isAutoScroll = false;
                move_selection(-1);
                break;
            case KEY_DOWN:
                isAutoScroll = false;
                move_selection(1);
                break;
            case KEY_HOME:
                isAutoScroll = false;
                current_selection = 0;
                move_selection(-1);
                break;
            case KEY_END:
                isAutoScroll = true;
                current_selection = packets.size() - 1;
                move_selection(1);
                break;
            case KEY_BACKSPACE:
                win1->init();
                win1->refresh();
                break;
            case 'f':
                select_protocol();
                current_selection=0;
                scroll_start=0;
                draw_list();
                break;
        }
        // mvprintw(max_y - 2, max_x - 10, "%6d", current_selection);
        // mvprintw(max_y - 1, max_x - 10, "%6d", scroll_start);
    }

    // Espera a que el hilo de captura termine (opcional)
    pthread_join(captureThread, NULL);

    // Cerrar la captura
    pcap_close(capdev);

    EndDisplay();

    return 0;
}

void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr) {
    end_time = steady_clock::now();
    duration<double> elapsed_seconds = duration_cast<duration<double>>(end_time - start_time);
    packets.push_back({elapsed_seconds, packetd_ptr, pkthdr->len});

    if(isAutoScroll){
        move_selection(1);
    }
}


void *threadpcap(void *arg) {
    pcap_t *capdev = (pcap_t *)arg;
    int packets_count = -1;

    if (pcap_loop(capdev, packets_count, packetManager, (u_char *)NULL)) {
        printf("ERR: pcap_loop() failed!\n");
        exit(1);
    }

    return NULL;
}

void select_protocol(){
    const char* protocols[]={"ALL","TCP","UDP","ICMP","IP"};
    int num_protocols=5;
    int index=0;
    int keyPressed=' ';

    while(keyPressed!='\n'){
        clear();
        mvprintw(0,0,"select a protocol to filter plis : (if you dont press anything you arent a deity )");
        for(int i=0; i<num_protocols;i++){
            if(i==index){
                cmvprintw(1,i+2,0,"%-10s",protocols[i]);
            }else{
                mvprintw(i+2,0,"%-10s",protocols[i]);

            }
        }
        mvprintw(num_protocols+3,0,"Press Enter for path of deities:  Up/Down to navigate");
        keyPressed=getch();
        switch(keyPressed){
            case 'W':
            case 'w':
            case KEY_UP:
                index =(index-1 + num_protocols)% num_protocols;
                break;
            case 'S':
            case 's':
            case KEY_DOWN:
                index=(index+1)% num_protocols;
                break;
        }
    }
    selected_protocol=protocols[index];
    clear();
    refresh();


}

void draw_list() {
    int available_rows = win1->max_y;
    int displayed_packets=0;
    int option_index=0;


    for(size_t i=scroll_start; i<packets.size() && displayed_packets < available_rows; ++i ){
        const struct ip* ip_header=(struct ip*)(packets[i].data.data() +14);
        int protocol=ip_header ->ip_p;
        string protText;
        switch (protocol) {
            case IPPROTO_TCP: protText = "TCP"; break;
            case IPPROTO_UDP: protText = "UDP"; break;
            case IPPROTO_ICMP: protText = "ICMP"; break;
            case IPPROTO_IP: protText = "IP"; break;
            default: protText = to_string(protocol); break;
        }
        if (selected_protocol != "ALL" && selected_protocol != protText) {
            continue; // Ignora este paquete
        }
        if (option_index < packets.size()) {
            if (option_index == current_selection) {
                wattron(win1->win, A_REVERSE);
            }
            const struct ip* ip_header;   // Estructura para el encabezado IP
            int ip_header_length;         // Longitud del encabezado IP

            // Saltar el encabezado Ethernet si es necesario (usualmente 14 bytes)

            // Obtener el encabezado IP desde los datos del paquete
            ip_header = (struct ip*)(packets[option_index].data.data() + 14);  // +14 para saltar el encabezado Ethernet (si presente)
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
                    protText = to_string(protocol);
                    break;
            }

            // Obtener tamaño del paquete
            int packet_size = ntohs(ip_header->ip_len);


            
        }
        if(displayed_packets == current_selection-scroll_start){
            wattron(win1->win,A_REVERSE);
        }

        char source_ip[INET_ADDRSTRLEN], dest_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&(ip_header->ip_src),source_ip,INET_ADDRSTRLEN);
        inet_ntop(AF_INET,&(ip_header->ip_dst), dest_ip, INET_ADDRSTRLEN);
        mvwprintw(win1->win, displayed_packets+1,1,"%5zu %11f  %-15s %-15s  %4s  %6d",i,packets[i].elapsed_seconds.count(),source_ip,dest_ip,protText.c_str(),ntohs(ip_header->ip_len));

        if(displayed_packets == current_selection -scroll_start){
            wattroff(win1->win,A_REVERSE);
        }
        ++displayed_packets;

    }

    win1->refresh();
}

void move_selection(int direction) {
    int new_selection = current_selection + direction;
    while (new_selection >=0 && new_selection<static_cast<int>(packets.size())){
        const struct ip* ip_header =(struct ip*)(packets[new_selection].data.data()+14);
        int protocol=ip_header->ip_p;

        string proText;
        switch (protocol){
            case IPPROTO_TCP: proText="TCP";break;
            case IPPROTO_UDP: proText="UDP";break;
            case IPPROTO_ICMP: proText="ICMP";break;
            case IPPROTO_IP: proText="IP"; break;
            default: proText=to_string(protocol);break;
        }

        if(selected_protocol == "ALL" || selected_protocol ==proText){
            current_selection=new_selection;
            break;
        }
        new_selection+= direction;

    }

    if(current_selection < scroll_start){
        scroll_start=current_selection;

    }else if(current_selection >= scroll_start +win1->max_y){
        scroll_start=current_selection -win1->max_y+1;
    }


    draw_list();
}
