#include "includes.hpp"
#include "display.hpp"

#define cmvprintw(c, y, x, format, ...) \
    do { \
        attron(A_REVERSE); \
        mvprintw(y, x, format, ##__VA_ARGS__); \
        attroff(A_REVERSE); \
    } while (0)

using namespace std;

struct DataBlocks {
    string hexData; // Lista de cadenas en formato hexadecimal
    string rawData; // Lista de cadenas en formato "raw"
};

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
parentwin *win1, *win2, *win3;
vector<packetData> packets;
int max_y, max_x;
int packetCount = 0;
steady_clock::time_point start_time, end_time;

int current_selection = 0;
int scroll_start = 0;
int current_selection3 = 0;
int scroll_start3 = 0;

int isAutoScroll = true;


void InitDisplay(int &height, int &width);
void EndDisplay();
void *threadpcap(void *arg);
void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr);
void draw_list();
void move_selection(int direction);
vector<DataBlocks> splitIntoBlocks();

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
    win1 = new parentwin(max_y / 2, max_x / 2, 0, 0);
    win1->init();
    // win1->subw.win = subwin(win1->win, win1->height - 2, win1->width - 2, 1, 1);
    // box(win1->subw.win, 0 ,0);
    // mvwprintw(win1->subw.win, 1, 1, "hello");
    // mvprintw(5, 0, "%d",win1->subw.win);
    // win1->subw.refresh();
    
    win2 = new parentwin(max_y / 2, max_x / 2, max_y / 2, 0);
    win2->init();
    // box(win2->subw.win, 0 ,0);
    // mvwprintw(win2->subw.win, 1, 1, "hello");
    // mvprintw(6, 0, "%d",win2->subw.win);
    // win2->subw.refresh();

    win3 = new parentwin(max_y, max_x / 2, 0, max_x / 2);
    win3->init();
    // mvprintw(7, 0, "%d",win3->subw.win);
    // win3->subw.refresh();

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
                // win1->init();
                // win1->refresh();
                // box(win2->subw.win, 0, 0);
                // win2->subw.refresh();
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

void draw_list() {
    int available_rows = win1->subw.height;

    for (int i = 0; i < available_rows; i++) {
        int option_index = scroll_start + i;

        if (option_index < packets.size()) {
            if (option_index == current_selection) {
                wattron(win1->subw.win, A_REVERSE);
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

            mvwprintw(win1->subw.win, i, 0, "%5d %11f  %-15s %-15s  %4s  %5d", option_index + 1, packets[option_index].elapsed_seconds.count(), source_ip, dest_ip, protText.c_str(), packets[option_index].length);
            wattroff(win1->subw.win, A_REVERSE);
        }
    }
    // mvwprintw(win1->subw.win, 0, 0, "hello");
    // win1->subw.refresh();
    win1->subw.refresh();
}
void draw_rawData(const vector<DataBlocks>& blocks){
    win3->subw.erase();
    int available_rows = win3->subw.height;

    // mvwprintw(win3->win, 1, 1, "%d", available_rows);

    for (int i = 0; i < available_rows; i++) {
        int option_index = scroll_start3 + i;

        if (option_index < blocks.size()) {
            // if (option_index == current_selection3) {
            //     wattron(win3->win, A_REVERSE);
            // }

            mvwprintw(win3->subw.win, i, 0, "%-48s  %-16s", blocks[i].hexData.c_str(), blocks[i].rawData.c_str());
            // wattroff(win3->win, A_REVERSE);
        }
    }
    win3->subw.refresh();
}

void move_selection(int direction) {
    int new_selection = current_selection + direction;

    if (new_selection >= 0 && new_selection < static_cast<int>(packets.size())) {
        current_selection = new_selection;
    }

    if (current_selection < scroll_start) {
        scroll_start = current_selection;
    } else if (current_selection >= scroll_start + win1->subw.height) {
        scroll_start = current_selection - win1->subw.height + 1;
    }

    // wclear(win3->win);
    // win3->init();
    // wmove(win3->win, 1, 1);
    // mvwprintw(win2->win, 0, 0, "Hello world!"); 
    // int y = max_y / 2;
    // vector<string> rawData;
    // string line = "";
    // for (int i = 0; i < packets[current_selection].length; i++) {

    //     if(packets[current_selection].data.data()[i] >= 32 && packets[current_selection].data.data()[i]<=128) 
    //     {
    //         // wprintw(win3->win, "%c", (unsigned char)packets[current_selection].data.data()[i]);
    //         line += (unsigned char)packets[current_selection].data.data()[i];
    //     }
    //     else 
    //     {
    //         line += '.';
    //         // wprintw(win3->win, ".");
    //     }

    //     if (i % 16 == 0) {
    //         // wprintw(win2->win, "\n");
    //         // y++;
    //         rawData.push_back(line);
    //         line = "";
    //     }
    //     // if(y >= max_y - 1) break;
        
        
    //     // wprintw(win2->win, "%02X ", packets[current_selection].data.data()[i]);
    // }
    // // win3->refresh();

    // vector<DataBlocks> blocks = splitIntoBlocks();

    draw_list();
    draw_rawData(splitIntoBlocks());
}


vector<DataBlocks> splitIntoBlocks() {
    vector<DataBlocks> blocks; // Estructura para almacenar los bloques

    u_char *data = packets[current_selection].data.data();

    // Iterar sobre el vector en bloques de 16 bytes
    for (size_t i = 0; i < packets[current_selection].length; i += 16) {
        DataBlocks block;
        stringstream hexStream; // stringstream para almacenar el bloque en hexadecimal
        stringstream rawStream; // stringstream para almacenar el bloque en formato "raw"

        // Obtener el bloque actual de 16 bytes
        for (size_t j = i; j < i + 16 && j < packets[current_selection].length; ++j) {
            // Formatear y añadir a la lista en hexadecimal
            hexStream << hex << setw(2) << setfill('0') << static_cast<int>(data[j]) << " ";
            // Añadir al raw stream
            
            if(data[j] >= 32 &&  data[j] <= 126)
                rawStream << (unsigned char)data[j];
            else 
                rawStream << '.';
        }
        block.hexData = hexStream.str();
        block.rawData = rawStream.str();

        // Guardar las cadenas formateadas en sus respectivas listas
        blocks.push_back(block);
        // blocks.rawData.push_back(rawStream.str());
    }

    return blocks;
}