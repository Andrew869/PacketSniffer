#include "includes.hpp"
#include "display.hpp"
#include "selectList.hpp"

#define cmvprintw(c, y, x, format, ...) \
    do { \
        attron(A_REVERSE); \
        mvprintw(y, x, format, ##__VA_ARGS__); \
        attroff(A_REVERSE); \
    } while (0)

int link_hdr_length = 0;

void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr);
void *threadpcap(void *arg);

int max_y, max_x;

// int packetCount = 0;
steady_clock::time_point start_time, end_time;

int main(int argc, char const *argv[]) {
    pcap_if_t *alldevsp , *device;
    char errbuf[100] , *devname , devs[100][100];
    int count = 0;

    // int packets_count = -1;
    // char filters[100] = "port 80";
    // char filters[] = "tcp port 22";z
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
    win1 = new ParentWin((max_y / 2)-4, max_x / 2, 3, 0);
    win1->init();
    // win1->subw.win = subwin(win1->win, win1->height - 2, win1->width - 2, 1, 1);
    // box(win1->subw.win, 0 ,0);
    // mvwprintw(win1->subw.win, 1, 1, "hello");
    // mvprintw(5, 0, "%d",win1->subw.win);
    // win1->subw.refresh();
    
    win2 = new ParentWin(max_y / 2, max_x / 2, max_y / 2, 0);
    win2->init();

    // box(win2->subw.win, 0 ,0);
     //mvwprintw(win2->subw.win, 1, 1, "hello");
    // mvprintw(6, 0, "%d",win2->subw.win);
     win2->subw.refresh();

    win3 = new ParentWin(max_y-4, max_x / 2, 3, max_x / 2);
    win3->init();
    PrintTitles();

    win4=new ParentWin(3,max_x,0,0);
    win4->init();
    printMenu();

    // mvprintw(7, 0, "%d",win3->subw.win);
    // win3->subw.refresh();
    mainList = new MainList(win1->subw, packets, main_packet_data);

    // lists.emplace_back(win1->subw, packets, main_packet_data);
    // lists.emplace_back(win3->subw, packets, main_packet_data);
    // lists.emplace_back(win1->subw, packets, main_packet_data);

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
                autoScroll = false;
                mainList->move_selection(-1);
                break;
            case KEY_DOWN:
                autoScroll = false;
                mainList->move_selection(1);
                break;
            case 'w':
                // autoScroll = false;
                mainList->rawList->move_selection(-1);
                break;
            case 's':
                // autoScroll = false;
                mainList->rawList->move_selection(1);
                break;
            case KEY_HOME:
                autoScroll = false;
                // current_selection = 0;
                mainList->current_selection = 0;
                mainList->move_selection(-1);
                break;
            case KEY_END:
                autoScroll = true;
                mainList->current_selection = mainList->list.size() - 1;
                // current_selection = packets.size() - 1;
                mainList->move_selection(1);
                break;
            case KEY_BACKSPACE:
                win1->erase();
                win2->erase();
                win3->erase();
                win1->init();
                win2->init();
                win3->init();
                PrintTitles();
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

    if(autoScroll){
        mainList->move_selection(1);
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