#include "includes.hpp"
#include "selectList.hpp"
#include "display.hpp"
#include "packetCapture.hpp"
#include "state.hpp"

char filters[100];

int link_hdr_length = 0;

void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr);
// void *threadpcap(void *arg);
void MainLoop();
void GetDevices(vector<string>* devs);
void GetstructuredData(vector<string>* list);
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
    InitDisplay(max_y, max_x);
    refresh();

    menuWin = new BasicWin(1, max_x, 0, 0);
    mvwprintw(menuWin->win, 0, 0, "Hello world!");
    menuWin->refresh();
    mainWin = new BasicWin(max_y - 2, max_x, 1, 0);
    conWin = new BasicWin(1, max_x, max_y - 1, 0);
    mvwprintw(conWin->win, 0, 0, "Hello Console!");
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
        new ParentWin(half_height, half_width, 0, 0, main_packet_data),
        new ParentWin(mHeight - half_height, half_width, half_height, 0, print_packet_info),
        new ParentWin(mHeight, mWidth - half_width, 0, half_width, DrawRawData)
    }));

    State::states.emplace_back(new StateI({new ParentWin(half_height, half_width, 0, 0, DrawString)}));
    State::states.emplace_back(new StateF({new ParentWin<string>(half_height, half_width, 0, 0)}));

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

void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr) {
    // packetCapture->end_time = steady_clock::now();
    // duration<double> elapsed_seconds = duration_cast<duration<double>>(end_time - start_time);
    duration<double> elapsed_seconds = packetCapture->GetElapsedTime();

    ParentWin<PacketData>* parentWin = dynamic_cast<ParentWin<PacketData>*>(State::states[STATE_M]->windows[0]);
    vector<PacketData>* list;
    if (parentWin) {
        list = parentWin->GetList();
        list->push_back({elapsed_seconds, packetd_ptr, pkthdr->len});
    } else {
        std::cerr << "Error: El objeto no es de tipo ParentWin<PacketData>" << std::endl;
    }
    // vector<PacketData>* packets = dynamic_cast<ParentWin<PacketData>*>(states[currentState]->windows[0])->GetList();

    // packets->push_back({elapsed_seconds, packetd_ptr, pkthdr->len});

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

// void *threadpcap(void *arg) {
//     pcap_t *capdev = (pcap_t *)arg;
//     int packets_count = -1;

//     int result = pcap_loop(capdev, packets_count, packetManager, nullptr);
//     if (result == -1) {
//         fprintf(stderr, "ERR: pcap_loop() failed: %s\n", pcap_geterr(capdev));
//         exit(1);
//     } 
//     // else if (result == -2) {
//     //     printf("pcap_loop() finalizado por pcap_breakloop().\n");
//     // }

//     return NULL;
// }

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
}

void splitIntoBlocks(vector<DataBlocks>* list) {
    list->clear(); // Limpiar cualquier dato anterior

    u_char *data_ptr = (*packets)[(*packetIndex)].data.data();
    bpf_u_int32 len = (*packets)[(*packetIndex)].length;

    for (size_t i = 0; i < len; i += 16) {
        DataBlocks block;
        std::stringstream hexStream, rawStream;

        for (size_t j = i; j < i + 16 && j < len; ++j) {
            hexStream << std::hex << std::setw(2) << std::setfill('0') 
                    << static_cast<int>(data_ptr[j]) << " ";

            if (data_ptr[j] >= 32 && data_ptr[j] <= 126)
                rawStream << static_cast<unsigned char>(data_ptr[j]);
            else 
                rawStream << '.';
        }

        block.hexData = hexStream.str();
        block.rawData = rawStream.str();
        list->push_back(block);
    }
}

void GetstructuredData(vector<string>* list) {
    list->clear();
    
    u_char *data_ptr = (*packets)[(*packetIndex)].data.data();
    bpf_u_int32 len = (*packets)[(*packetIndex)].length;

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

void SetupPCAP(const char* devname) {
    char errbuf[PCAP_ERRBUF_SIZE];
    int packets_count = -1;
    struct bpf_program bpf;
    bpf_u_int32 net;  // Dirección de red
    bpf_u_int32 mask;  // Máscara de red

    // Abrir el dispositivo
    capdev = pcap_open_live(devname, 65535, 1, 1000, errbuf);
    if (capdev == NULL) {
        printf("ERR: pcap_open_live() failed: %s\n", errbuf);
        exit(1);
    }

    // Obtener tipo de enlace
    int link_hdr_type = pcap_datalink(capdev);

    // Configurar filtro BPF
    if (pcap_lookupnet(devname, &net, &mask, errbuf) == -1) {
        fprintf(stderr, "No se pudo obtener la dirección de red: %s\n", errbuf);
        exit(1);
    }

    // bpf_u_int32 netmask = 0xFFFFFF; // Netmask predeterminado
    if (pcap_compile(capdev, &bpf, filters, 0, net) == PCAP_ERROR) {
        printf("ERR: pcap_compile() failed: %s\n", pcap_geterr(capdev));
        pcap_close(capdev);
        exit(1);
    }

    if (pcap_setfilter(capdev, &bpf)) {
        printf("ERR: pcap_setfilter() failed: %s\n", pcap_geterr(capdev));
        pcap_freecode(&bpf);
        pcap_close(capdev);
        exit(1);
    }

    pcap_freecode(&bpf); // Liberar el filtro compilado

    // Crear el hilo de captura
    // int threadError = pthread_create(&captureThread, NULL, threadpcap, (void*)capdev);
    // if (threadError) {
    //     fprintf(stderr, "Error al crear el hilo: %d\n", threadError);
    //     pcap_close(capdev);
    //     exit(1);
    // }

    start_time = steady_clock::now();
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
}