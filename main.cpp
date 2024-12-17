#include "includes.hpp"
#include "selectList.hpp"
#include "display.hpp"
<<<<<<< HEAD
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

=======
#include "packetCapture.hpp"
#include "state.hpp"

char filters[100];

int link_hdr_length = 0;

void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr);
// void *threadpcap(void *arg);
void MainLoop();
void GetDevices(vector<string>* devs);
void GetstructuredData(vector<string>* list, const PacketData &packetData);
void GetstructuredData(vector<string>* list);
void splitIntoBlocks(vector<DataBlocks>* list, const PacketData &packetData);
void splitIntoBlocks(vector<DataBlocks>* list);
// void SetupPCAP(const char* devname);
template <typename T> ParentWin<T>* GetParentWin(BaseParentWin* baseParentWin);

int max_y, max_x;
pcap_t *capdev;
pthread_t captureThread;

// int packetCount = 0;
steady_clock::time_point start_time, end_time;

// template<typename T>
// struct WinsAndList {
//     ParentWin windows;
//     List<T> list;
// };



// int state::CurrentID = 0;

// vector<State*> states;
// short currentState, prevState;
std::vector<State*> State::states;
short State::currentState = 0;
short State::prevState = 0;

int main(int argc, char const *argv[]) {
>>>>>>> origin/testing
    InitDisplay(max_y, max_x);
    refresh();
    // win5= new ParentWin(max_y,max_x,4,3);
    // win5->init();
    // printAyuda();


    menuWin = new BasicWin(1, max_x, 0, 0);
    mvwprintw(menuWin->win, 0, 0, "Hello world!");
    menuWin->refresh();
    mainWin = new BasicWin(max_y - 2, max_x, 1, 0);
    conWin = new BasicWin(1, max_x, max_y - 1, 0);
    mvwprintw(conWin->win, 0, 0, "Ready to capture!");
    conWin->refresh();

    packetCapture = new PacketCapture();

    // mainList = new MainList(win1->subw, packets, main_packet_data);

    // clear();
    // refresh();
    // win1 = new ParentWin(max_y / 2, max_x / 2, 0, 0);
    // win2 = new ParentWin(max_y / 2, max_x / 2, max_y / 2, 0);
    // win3 = new ParentWin(max_y, max_x / 2, 0, max_x / 2);
    // win4 = new ParentWin(max_y / 2, max_x / 2, 0, 0);
    short mHeight = mainWin->height;
    short mWidth = mainWin->width;
    short half_height = (mHeight / 2) - 2;
    short half_width = mWidth / 2;


    State::states.push_back(new StateM({
        new ParentWin(
            half_height, half_width, 0, 0, 
            new vector<Title>({
                Title{2, "[No.]"},
                Title{9, "[Time]"},
                Title{20, "[Source]"},
                Title{36, "[Destination]"},
                Title{52, "[Prot]"},
                Title{60, "[Len]"}
            }),
            main_packet_data
        ),
        new ParentWin(
            mHeight - half_height, half_width, half_height, 0,
            new vector<Title>({
                Title{2, "[Structured Data]"}
            }),
            print_packet_info
        ),
        new ParentWin(
            mHeight, mWidth - half_width, 0, half_width,
            new vector<Title>({
                Title{2, "[Raw Data]"}
            }),
            DrawRawData
        )
    }));

    State::states.emplace_back(new StateI({
        new ParentWin(
            half_height, 30, 0, 0,
            new vector<Title>({
                Title{2, "[Select Interface]"}
            }),
            DrawString
        )
    }));

    State::states.emplace_back(
        new StateF({new ParentWin<string>(
            3, mWidth, 0, 0,
            new vector<Title>({
                Title{2, "[Type new filter rules]"}
            })
        )
    }));

    auto sIw0 = GetParentWin<string>(State::states[STATE_I]->windows[0]);

    sIw0->SetListGenerator(GetDevices);
    sIw0->UpdateList();

    // auto parentWin = dynamic_cast<ParentWin<string>*>(states[STATE_I]->windows[0]);
    // if (parentWin) {
    //     parentWin->SetListGenerator(GetDevices);
    //     parentWin->UpdateList();
    //     // vector<string>* list = parentWin->GetList();
    //     // GetDevices(list);
    // } else {
    //     std::cerr << "Error: El objeto no es de tipo ParentWin<string>" << std::endl;
    // }

    auto sMw0 = GetParentWin<PacketData>(State::states[STATE_M]->windows[0]);
    
    packets = sMw0->GetList();
    packetIndex = sMw0->GetCurrIndex();

    State::states[STATE_M]->SetWinLink(0,1);
    auto sMw1 = GetParentWin<string>(State::states[STATE_M]->windows[1]);
    sMw1->SetListGenerator(GetstructuredData);

    State::states[STATE_M]->SetWinLink(0,2);
    auto sMw2 = GetParentWin<DataBlocks>(State::states[STATE_M]->windows[2]);
    sMw2->SetListGenerator(splitIntoBlocks);

    // sMw1->UpdateList();
    
    // states[STATE_M].SetWinLink(0,2);

    State::states[STATE_I]->DrawState();

    State::currentState = STATE_I;

    MainLoop();

    packetCapture->close();
    EndDisplay();

    return 0;
}

// Cada que un paquete es capturado se llama a esta funcion, la cual guarda el contenido del packete en una lista
void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr) {
    duration<double> elapsed_seconds = packetCapture->GetElapsedTime();

    ParentWin<PacketData>* parentWin = dynamic_cast<ParentWin<PacketData>*>(State::states[STATE_M]->windows[0]);
    vector<PacketData>* list;
    if (parentWin) {
        list = parentWin->GetList();
        list->push_back({elapsed_seconds, packetd_ptr, pkthdr->len});
    } else {
        std::cerr << "Error: El objeto no es de tipo ParentWin<PacketData>" << std::endl;
    }

    if(State::currentState != STATE_M) return;

    int height = State::states[STATE_M]->windows[0]->GetSubHeight();


    if(autoScroll){
        State::states[STATE_M]->windows[0]->moveSelection(1);
        State::states[STATE_M]->windows[0]->DrawSubWindow();
    }
    else if(!autoScroll && list->size() <= height) {
        State::states[STATE_M]->windows[0]->DrawSubWindow();
    }
}

// Bucle principal, donde se maneja la entrada de teclas del usuario
void MainLoop(){
    int keyPressed = 0;
    while ((keyPressed = getch()) != 'q') {
        // if(keyPressed > 0) mvprintw(0, 0, "%d", keyPressed);
        State::states[State::currentState]->HandleKeyPress(State::currentState, keyPressed);
    }
    // if (capdev) {
    //     pcap_breakloop(capdev);  // Finalizar el pcap_loop
    //     pthread_join(captureThread, NULL);
    // }
}

// Optiene el nombre de las interfaces de red
void GetDevices(vector<string>* devs){
    devs->clear();
    char errbuf[100];
    pcap_if_t *alldevsp , *device;
    if(pcap_findalldevs(&alldevsp ,errbuf)) {
		// printf("Error finding devices : %s" , errbuf);
		exit(1);
	}

    for(device = alldevsp ; device != NULL ; device = device->next) {
		if(device->name != NULL) {
			// strcpy(devs[count] , device->name);
            devs->push_back(device->name);
		}
	}
<<<<<<< HEAD

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

    win5= new ParentWin(max_y,max_x,4,3);
    win5->init();
    printAyuda();
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
                save_to_csv("output.csv", *mainList);
                save_to_excel("output.xlsx", *mainList);
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
=======
>>>>>>> origin/testing
}

// Procesa un packete para poder mostrarlo en modo "RAW"
void splitIntoBlocks(vector<DataBlocks>* list, const PacketData &packetdata) {
    list->clear(); // Limpiar cualquier dato anterior

<<<<<<< HEAD
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
=======
    // u_char *data_ptr = (*packets)[(*packetIndex)].data.data();
    // bpf_u_int32 len = (*packets)[(*packetIndex)].length;

    const u_char *data_ptr = packetdata.data.data();
    bpf_u_int32 len = packetdata.length;


    for (size_t i = 0; i < len; i += 16) {
        DataBlocks block;
        std::stringstream hexStream, asciiData;

        for (size_t j = i; j < i + 16 && j < len; ++j) {
            hexStream << std::hex << std::setw(2) << std::setfill('0') 
                    << static_cast<int>(data_ptr[j]) << " ";

            if (data_ptr[j] >= 32 && data_ptr[j] <= 126)
                asciiData << static_cast<unsigned char>(data_ptr[j]);
            else 
                asciiData << '.';
        }

        block.hexData = hexStream.str();
        block.asciiData = asciiData.str();
        list->push_back(block);
    }
}

void splitIntoBlocks(vector<DataBlocks>* list){
    PacketData packet = (*packets)[(*packetIndex)];
    splitIntoBlocks(list, packet);
}

//Procesa un paquete para mostrarlo estructuradamente
void GetstructuredData(vector<string>* list, const PacketData &packetdata) {
    list->clear();
    
    // u_char *data_ptr = (*packets)[(*packetIndex)].data.data();
    // bpf_u_int32 len = (*packets)[(*packetIndex)].length;

    const u_char *data_ptr = packetdata.data.data();
    bpf_u_int32 len = packetdata.length;

    // Cabecera Ethernet
    if (len >= sizeof(struct ethhdr)) {
        struct ethhdr* eth = (struct ethhdr*)data_ptr;
        list->push_back( "Ethernet Header:");
        ostringstream oss;

        // Formatear y agregar la dirección de destino
        oss << "   |-Destination Address : ";
        for (int i = 0; i < 6; ++i) {
            oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>( eth->h_dest[i]);
            if (i < 5) oss << "-";
        }
        list->push_back(oss.str());
        oss.str(""); // Limpiar el contenido del stringstream
        oss.clear(); // Restablecer flags

        // Formatear y agregar la dirección de origen
        oss << "   |-Source Address      : ";
        for (int i = 0; i < 6; ++i) {
            oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(eth->h_source[i]);
            if (i < 5) oss << "-";
        }
        list->push_back(oss.str());


        // list->push_back( "   |-Destination Address : "+to_string( eth->h_dest[0])+to_string(eth->h_dest[1]) + to_string(eth->h_dest[2])+to_string(eth->h_dest[3])+to_string(eth->h_dest[4]) +to_string(eth->h_dest[5]));
        // list->push_back( "   |-Source Address      : "+to_string( eth->h_source[0])+to_string(eth->h_source[1]) + to_string(eth->h_source[2])+to_string(eth->h_source[3])+to_string(eth->h_source[4]) +to_string(eth->h_source[5]));
        list->push_back( "   |-Protocol            : "+to_string((unsigned short)eth->h_proto));
    } else {
        list->push_back( "Incomplete Ethernet Header");
    }

    // Cabecera IP
    if (len > sizeof(struct ethhdr)) {
        struct iphdr* iph = (struct iphdr*)(data_ptr + sizeof(struct ethhdr));
        struct sockaddr_in source, dest;
        source.sin_addr.s_addr = iph->saddr;
        dest.sin_addr.s_addr = iph->daddr;

        list->push_back( "IP Header:");
        list->push_back( "   |-IP Version        : "+ to_string((unsigned int)iph->version));
        list->push_back( "   |-Header Length     : "+ to_string (((unsigned int)(iph->ihl)) * 4));
        list->push_back( "   |-Source IP         : "+ string( inet_ntoa(source.sin_addr)));
        list->push_back( "   |-Destination IP    : "+ string( inet_ntoa(dest.sin_addr)));
        list->push_back( "   |-Protocol          : "+ to_string( (unsigned int)iph->protocol));

        // Cabecera TCP o UDP
        if (iph->protocol == IPPROTO_TCP && len > sizeof(struct ethhdr) + iph->ihl * 4) {
            struct tcphdr* tcph = (struct tcphdr*)(data_ptr + sizeof(struct ethhdr) + iph->ihl * 4);
            list->push_back( "TCP Header:");
            list->push_back( "   |-Source Port      : "+ to_string( ntohs(tcph->source)));
            list->push_back( "   |-Destination Port : "+ to_string( ntohs(tcph->dest)));
            list->push_back(  "   |-Sequence Number    : "+to_string(ntohl(tcph->seq)));
            list->push_back(  "   |-Acknowledge Number : "+to_string(ntohl(tcph->ack_seq)));
            list->push_back(  "   |-Header Length      : " +to_string((unsigned int)tcph->doff)+"DWORDS"+to_string((unsigned int)tcph->doff*4)+"BYTES");
            list->push_back(  "   |-Urgent Flag          :"+to_string((unsigned int)tcph->urg));
            list->push_back(  "   |-Acknowledgement Flag :"+to_string((unsigned int)tcph->ack));
            list->push_back(  "   |-Push Flag            : "+to_string((unsigned int)tcph->psh));
            list->push_back(  "   |-Reset Flag           : "+to_string((unsigned int)tcph->rst));
            list->push_back(  "   |-Synchronise Flag     : "+to_string((unsigned int)tcph->syn));
            list->push_back(  "   |-Finish Flag          : "+to_string((unsigned int)tcph->fin));
            list->push_back(  "   |-Window         : "+to_string(ntohs(tcph->window)));
            list->push_back(  "   |-Checksum       : "+to_string(ntohs(tcph->check)));
            list->push_back(  "   |-Urgent Pointer : "+to_string(tcph->urg_ptr));
        } else if (iph->protocol == IPPROTO_UDP && len > sizeof(struct ethhdr) + iph->ihl * 4) {
            struct udphdr* udph = (struct udphdr*)(data_ptr + sizeof(struct ethhdr) + iph->ihl * 4);
            list->push_back( "UDP Header:");
            list->push_back( "   |-Source Port      : "+ to_string( ntohs(udph->source)));
            list->push_back( "   |-Destination Port : "+ to_string( ntohs(udph->dest)));
            list->push_back( "   |-UDP Length       : " + to_string( ntohs(udph->len)));
            list->push_back( "   |-UDP Checksum     : " + to_string( ntohs(udph->check)));
        }
    } else {
        list->push_back( "Incomplete IP Header");
    }
}

void GetstructuredData(vector<string>* list) {
    PacketData packet = (*packets)[(*packetIndex)];
    GetstructuredData(list, packet);
}

void save_to_csv() {
    string timestamp = getCurrentTimeString();

    string filename = "log_" + timestamp + ".csv";
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para escritura\n";
        return;
    }

    // Encabezados
    file << "Packet No,Time,Source IP,Destination IP,Protocol,Length,Hex Data,Ascii Data,Info\n";

    int index = 1;
    for (const auto& packet : (*packets)) {

        // Generar bloques asciiData e información
        vector<DataBlocks>* rawBlocks = new vector<DataBlocks>;
        splitIntoBlocks(rawBlocks, packet);

        vector<string>* info = new vector<string>;
        GetstructuredData(info, packet);

        // Extraer información básica del paquete
        const struct ip* ip_header = (struct ip*)(packet.data.data() + 14);
        char source_ip[INET_ADDRSTRLEN];
        char dest_ip[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(ip_header->ip_src), source_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ip_header->ip_dst), dest_ip, INET_ADDRSTRLEN);

        int protocol = ip_header->ip_p; // Campo ip_p contiene el número de protocolo (TCP, UDP, etc.)    

        // Escribir los encabezados generales del paquete
        file << index++ << ",";
        file << packet.elapsed_seconds.count() << ",";
        file << source_ip << ",";
        file << dest_ip << ",";
        file << GetProtocolText(protocol) << ",";
        file << packet.length << ",";

        // Escribir el asciiData y hexData (concatenados en una sola línea)
        string hexDataCombined, asciiDataCombined;
        for (const auto& block : (*rawBlocks)) {
            hexDataCombined += block.hexData + " ";
            asciiDataCombined += block.asciiData + " ";
        }

        file << "\"" << hexDataCombined << "\",";
        file << "\"" << asciiDataCombined << "\",";

        // Escribir la información (getInfo) en múltiples líneas
        string infoCombined;
        for (const auto& line : (*info)) {
            infoCombined += line + " | ";
        }
        file << "\"" << infoCombined << "\"\n";
    }

    file.close();
}

void save_to_excel() {
    string timestamp = getCurrentTimeString();

    string filename = "log_" + timestamp + ".xlsx";

    lxw_workbook *workbook = workbook_new(filename.c_str());
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    // Definir encabezados
    worksheet_write_string(worksheet, 0, 0, "Packet No", NULL);
    worksheet_write_string(worksheet, 0, 1, "Time", NULL);
    worksheet_write_string(worksheet, 0, 2, "Source IP", NULL);
    worksheet_write_string(worksheet, 0, 3, "Destination IP", NULL);
    worksheet_write_string(worksheet, 0, 4, "Protocol", NULL);
    worksheet_write_string(worksheet, 0, 5, "Length", NULL);
    worksheet_write_string(worksheet, 0, 6, "Hex Data", NULL);
    worksheet_write_string(worksheet, 0, 7, "Raw Data", NULL);
    worksheet_write_string(worksheet, 0, 8, "Info", NULL);

    int index = 0;
    for (const auto& packet : (*packets)) {
        index++;
        // Generar bloques asciiData e información
        vector<DataBlocks>* rawBlocks = new vector<DataBlocks>;
        splitIntoBlocks(rawBlocks, packet);

        vector<string>* info = new vector<string>;
        GetstructuredData(info, packet);


        // Extraer IPs
        const struct ip* ip_header = (struct ip*)(packet.data.data() + 14);
        char source_ip[INET_ADDRSTRLEN];
        char dest_ip[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(ip_header->ip_src), source_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ip_header->ip_dst), dest_ip, INET_ADDRSTRLEN);

        // Protocolo
       int protocol = ip_header->ip_p; // Campo ip_p contiene el número de protocolo (TCP, UDP, etc.)

        // Combinar rawData y hexData
        string hexDataCombined, rawDataCombined;
        for (const auto& block : (*rawBlocks)) {
            hexDataCombined += block.hexData + " ";
            rawDataCombined += block.asciiData + " ";
        }

        // Combinar la información
        string infoCombined;
        for (const auto& line : (*info)) {
            infoCombined += line + " | ";
        }

        // Escribir datos en la hoja
        worksheet_write_number(worksheet, index, 0, index, NULL);
        worksheet_write_number(worksheet, index, 1, packet.elapsed_seconds.count(), NULL);
        worksheet_write_string(worksheet, index, 2, source_ip, NULL);
        worksheet_write_string(worksheet, index, 3, dest_ip, NULL);
        worksheet_write_string(worksheet, index, 4, GetProtocolText(protocol).c_str(), NULL);
        worksheet_write_number(worksheet, index, 5, packet.length, NULL);
        worksheet_write_string(worksheet, index, 6, hexDataCombined.c_str(), NULL);
        worksheet_write_string(worksheet, index, 7, rawDataCombined.c_str(), NULL);
        worksheet_write_string(worksheet, index, 8, infoCombined.c_str(), NULL);
    }

    // Guardar y cerrar el libro de Excel
    workbook_close(workbook);
}

template <typename T>
ParentWin<T>* GetParentWin(BaseParentWin* baseParentWin) {
    // Realiza el dynamic_cast
    auto parentWin = dynamic_cast<ParentWin<T>*>(baseParentWin);

    // Retorna el puntero convertido o nullptr si falla el cast
    if (parentWin) {
        return parentWin;
    } else {
        std::cerr << "Error: El objeto no es de tipo ParentWin<" << typeid(T).name() << ">" << std::endl;
        return nullptr;
    }
>>>>>>> origin/testing
}