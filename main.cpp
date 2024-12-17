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
void GetHelp(vector<string>* list);
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
    InitDisplay(max_y, max_x);
    refresh();

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

    State::states.push_back(new StateI({
        new ParentWin(
            half_height, 30, 0, 0,
            new vector<Title>({
                Title{2, "[Select Interface]"}
            }),
            DrawString
        )
    }));

    State::states.push_back(new StateF({
        new ParentWin<string>(
            3, mWidth, 0, 0,
            new vector<Title>({
                Title{2, "[Type new filter rules]"}
            })
        )
    }));
    
    State::states.push_back(new StateH({
        new ParentWin<string>(
            mHeight, mWidth, 0, 0,
            new vector<Title>({
                Title{2, "[Help]"}
            }),
            DrawString
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

    auto sHw0 = GetParentWin<string>(State::states[STATE_H]->windows[0]);
    sHw0->SetListGenerator(GetHelp);
    sHw0->UpdateList();
    
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
}

// Procesa un packete para poder mostrarlo en modo "RAW"
void splitIntoBlocks(vector<DataBlocks>* list, const PacketData &packetdata) {
    list->clear(); // Limpiar cualquier dato anterior

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

void GetHelp(vector<string>* list){
    list->clear();

    list->push_back("Network Interface Selection");
    list->push_back("    In this section, you must select the network interface you want to work with.");
    list->push_back("");
    list->push_back("Packet Capture");
    list->push_back("    Once the network interface is selected, the main screen will appear, containing 3 sections.");
    list->push_back("    Section 1 - List of captured packets");
    list->push_back("        In this section, you can see a list that grows as packets are captured.");
    list->push_back("    Section 2 - Structured data");
    list->push_back("        In this part, you can see the packet information in a structured format.");
    list->push_back("    Section 3 - Raw data");
    list->push_back("        This section shows the packet data in \"raw\" format.");
    list->push_back("    Each of these sections can be accessed by pressing the <tab> key.");
    list->push_back("");
    list->push_back("Log Saving");
    list->push_back("    The captured packet information can be exported by pressing the key combination");
    list->push_back("    <ctrl + s>, which will generate two files: one in CSV format and another in XLSX format.");
    list->push_back("");
    list->push_back("Common Filter Expressions");
    list->push_back("    Capture by protocol:");
    list->push_back("        \"ip\": Captures only IP packets.");
    list->push_back("        \"icmp\": Captures only ICMP packets (ping).");
    list->push_back("        \"tcp\": Captures only TCP packets.");
    list->push_back("        \"udp\": Captures only UDP packets.");
    list->push_back("    Capture by IP address:");
    list->push_back("        \"host 192.168.1.1\": Captures packets with the source or destination IP of 192.168.1.1.");
    list->push_back("        \"net 192.168.1.0/24\": Captures packets from the 192.168.1.0/24 network.");
    list->push_back("    Capture by port:");
    list->push_back("        \"port 80\": Captures packets with port 80 (HTTP).");
    list->push_back("        \"src port 80\": Captures packets with source port 80.");
    list->push_back("        \"dst port 80\": Captures packets with destination port 80.");
    list->push_back("    Capture combinations:");
    list->push_back("        \"tcp and port 80\": Captures only TCP packets on port 80.");
    list->push_back("        \"udp and src host 192.168.1.1\": Captures only UDP packets with source IP 192.168.1.1.");
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
}