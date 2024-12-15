// void CreateSecLists(vector<u_char>& data, bpf_u_int32 length);

struct DataBlocks {
    string hexData; // Lista de cadenas en formato hexadecimal
    string rawData; // Lista de cadenas en formato "raw"
};

class PacketData {
public:
    duration<double> elapsed_seconds;
    vector<u_char> data;
    bpf_u_int32 length;

    PacketData(duration<double> elapsed_seconds, const u_char* packet_ptr, bpf_u_int32 length) : length(length)  {
        this->elapsed_seconds = elapsed_seconds;
        data.assign(packet_ptr, packet_ptr + length);
    }
private:
};

void draw_rawData(SubWindow& subw, vector<DataBlocks>& list, int option_index, int y);
void print_packet_info(SubWindow& subw, vector<string>& list, int option_index, int y) ;


template<typename T>
class List {
public:
    // Constructor que inicializa el vector
    List(SubWindow& subw, vector<T>& list, void (*ListManager)(SubWindow&, vector<T>&, int, int))
        : subw(subw), list(list), ListManager(ListManager), current_selection(0), scroll_start(0) {}
    
    int current_selection;
    int scroll_start;
    SubWindow& subw;
    vector<T>& list;
    void (*ListManager)(SubWindow&, vector<T>&, int, int);

    void draw_list() {
        int available_rows = subw.height;

        for (int i = 0; i < available_rows; i++) {
            int option_index = scroll_start + i;

            if (option_index < static_cast<int>(list.size())) {
                if (option_index == current_selection) {
                    wattron(subw.win, A_REVERSE);
                }

                ListManager(subw, list, option_index, i);

                wattroff(subw.win, A_REVERSE);
            }
        }
        subw.refresh();
    }

    void move_selection(int direction) {
        int new_selection = current_selection + direction;

        if (new_selection >= 0 && new_selection < static_cast<int>(list.size())) {
            current_selection = new_selection;
        }

        if (current_selection < scroll_start) {
            scroll_start = current_selection;
        } else if (current_selection >= scroll_start + subw.height) {
            scroll_start = current_selection - subw.height + 1;
        }
        draw_list();
    }
};

template<typename T>
class DerList : public List<T> {
public:
    DerList(SubWindow& subw, vector<T>& list, void (*ListManager)(SubWindow&, vector<T>&, int, int))
    : List<T>(subw, list, ListManager) {
    }

    void move_selection(int dir){
        this->List<T>::move_selection(dir);
        win2->erase();
        mvwprintw(win2->win, 0, 0, "%d", this->current_selection);
        mvwprintw(win2->win, 1, 0, "%d", this->scroll_start);
        mvwprintw(win2->win, 2, 0, "%d", this->list.size());
        win2->refresh();
    }

};

class MainList : public List<PacketData> {
public:
    MainList(SubWindow& subw, vector<PacketData>& list, void (*ListManager)(SubWindow&, vector<PacketData>&, int, int))
        : List<PacketData>(subw, list, ListManager){}

    // void (*MoveManager)(vector<u_char>&, bpf_u_int32);

    DerList<DataBlocks>* rawList;
    vector<DataBlocks> generatedBlocks;

    DerList<string>* infoList;
    vector<string> output;

    // DerList<DataBlocks>* rawList;
    // std::unique_ptr<DerList<DataBlocks>> rawList;

    void draw_list() {
        int available_rows = this->subw.height;

        for (int i = 0; i < available_rows; i++) {
            int option_index = this->scroll_start + i;

            if (option_index < static_cast<int>(this->list.size())) {
                if (option_index == this->current_selection) {
                    wattron(this->subw.win, A_REVERSE);
                }

                this->ListManager(this->subw, this->list, option_index, i);

                wattroff(this->subw.win, A_REVERSE);
            }
        }
        this->subw.refresh();
    }

    void move_selection(int direction) {
        this->List<PacketData>::move_selection(direction);
        // PacketData* packet = reinterpret_cast<PacketData*>(&this->list[this->current_selection]);
        CreateSecLists();
    }

    vector<DataBlocks>& splitIntoBlocks() {
        generatedBlocks.clear(); // Limpiar cualquier dato anterior

        u_char *data_ptr = this->list[this->current_selection].data.data();
        bpf_u_int32 len = this->list[this->current_selection].length;

        for (size_t i = 0; i < len; i += 16) {
            DataBlocks block;
            ostringstream hexStream, rawStream;

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
            generatedBlocks.push_back(block);
        }

        return generatedBlocks; // Devolver referencia al vector almacenado
    }

    vector<string>& getInfo(){
        output.clear();
        PacketData& packet = list[this->current_selection];
        const u_char* data = packet.data.data();
        bpf_u_int32 length = packet.length;

    
        // Cabecera Ethernet
        if (length >= sizeof(struct ethhdr)) {
            struct ethhdr* eth = (struct ethhdr*)data;
            output.push_back( "Ethernet Header:");
            ostringstream oss;

            // Formatear y agregar la dirección de destino
            oss << "   |-Destination Address : ";
            for (int i = 0; i < 6; ++i) {
                oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>( eth->h_dest[0]);
                if (i < 5) oss << "-";
            }
            output.push_back(oss.str());
            oss.str(""); // Limpiar el contenido del stringstream
            oss.clear(); // Restablecer flags

            // Formatear y agregar la dirección de origen
            oss << "   |-Source Address      : ";
            for (int i = 0; i < 6; ++i) {
                oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(eth->h_source[0]);
                if (i < 5) oss << "-";
            }
            output.push_back(oss.str());


            // output.push_back( "   |-Destination Address : "+to_string( eth->h_dest[0])+to_string(eth->h_dest[1]) + to_string(eth->h_dest[2])+to_string(eth->h_dest[3])+to_string(eth->h_dest[4]) +to_string(eth->h_dest[5]));
            // output.push_back( "   |-Source Address      : "+to_string( eth->h_source[0])+to_string(eth->h_source[1]) + to_string(eth->h_source[2])+to_string(eth->h_source[3])+to_string(eth->h_source[4]) +to_string(eth->h_source[5]));
            output.push_back( "   |-Protocol            : "+to_string(ntohs(eth->h_proto)) );
        } else {
            output.push_back( "Incomplete Ethernet Header");
        }

        // Cabecera IP
        if (length > sizeof(struct ethhdr)) {
            struct iphdr* iph = (struct iphdr*)(data + sizeof(struct ethhdr));
            struct sockaddr_in source, dest;
            source.sin_addr.s_addr = iph->saddr;
            dest.sin_addr.s_addr = iph->daddr;

            output.push_back( "IP Header:");
            output.push_back( "   |-IP Version        : "+ to_string((unsigned int)iph->version));
            output.push_back( "   |-Header Length     : "+ to_string (((unsigned int)(iph->ihl)) * 4));
            output.push_back( "   |-Source IP         : "+ string( inet_ntoa(source.sin_addr)));
            output.push_back( "   |-Destination IP    : "+ string( inet_ntoa(dest.sin_addr)));
            output.push_back( "   |-Protocol          : "+ to_string( (unsigned int)iph->protocol));

            // Cabecera TCP o UDP
            if (iph->protocol == IPPROTO_TCP && length > sizeof(struct ethhdr) + iph->ihl * 4) {
                struct tcphdr* tcph = (struct tcphdr*)(data + sizeof(struct ethhdr) + iph->ihl * 4);
                output.push_back( "TCP Header:");
                output.push_back( "   |-Source Port      : "+ to_string( ntohs(tcph->source)));
                output.push_back( "   |-Destination Port : "+ to_string( ntohs(tcph->dest)));
                output.push_back(  "   |-Sequence Number    : "+to_string(ntohl(tcph->seq)));
	            output.push_back(  "   |-Acknowledge Number : "+to_string(ntohl(tcph->ack_seq)));
	            output.push_back(  "   |-Header Length      : " +to_string((unsigned int)tcph->doff)+"DWORDS"+to_string((unsigned int)tcph->doff*4)+"BYTES");
	            output.push_back(  "   |-Urgent Flag          :"+to_string((unsigned int)tcph->urg));
	            output.push_back(  "   |-Acknowledgement Flag :"+to_string((unsigned int)tcph->ack));
	            output.push_back(  "   |-Push Flag            : "+to_string((unsigned int)tcph->psh));
	            output.push_back(  "   |-Reset Flag           : "+to_string((unsigned int)tcph->rst));
	            output.push_back(  "   |-Synchronise Flag     : "+to_string((unsigned int)tcph->syn));
	            output.push_back(  "   |-Finish Flag          : "+to_string((unsigned int)tcph->fin));
	            output.push_back(  "   |-Window         : "+to_string(ntohs(tcph->window)));
	            output.push_back(  "   |-Checksum       : "+to_string(ntohs(tcph->check)));
	            output.push_back(  "   |-Urgent Pointer : "+to_string(tcph->urg_ptr));

	         
		
	          
	       
						


            } else if (iph->protocol == IPPROTO_UDP && length > sizeof(struct ethhdr) + iph->ihl * 4) {
                struct udphdr* udph = (struct udphdr*)(data + sizeof(struct ethhdr) + iph->ihl * 4);
                output.push_back( "UDP Header:");
                output.push_back( "   |-Source Port      : "+ to_string( ntohs(udph->source)));
                output.push_back( "   |-Destination Port : "+ to_string( ntohs(udph->dest)));
                output.push_back( "   |-UDP Length       : " + to_string( ntohs(udph->len)));
	            output.push_back( "   |-UDP Checksum     : " + to_string( ntohs(udph->check)));
            }
        } else {
            output.push_back( "Incomplete IP Header");
        }

        // Refresca la ventana para mostrar los datos
        return output;
    }

    void CreateSecLists() {
        rawList = new DerList<DataBlocks>(win3->subw, splitIntoBlocks(), draw_rawData);
        rawList->subw.erase();
        rawList->draw_list();

        infoList = new DerList<string>(win2->subw, getInfo(), print_packet_info);
        infoList->subw.erase();
        infoList->draw_list();
    }
};

vector<PacketData> packets;
int autoScroll = true;

void main_packet_data(SubWindow& subw, vector<PacketData>& list, int option_index, int y) {
    const struct ip* ip_header;   // Estructura para el encabezado IP
    int ip_header_length;         // Longitud del encabezado IP

    // Saltar el encabezado Ethernet si es necesario (usualmente 14 bytes)

    // Obtener el encabezado IP desde los datos del paquete
    ip_header = (struct ip*)(list[option_index].data.data() + 14);  // +14 para saltar el encabezado Ethernet (si presente)
    ip_header_length = ip_header->ip_hl * 4; // Longitud del encabezado IP en bytes (ip_hl está en palabras de 4 bytes)

    // Obtener direcciones IP de origen y destino
    char source_ip[INET_ADDRSTRLEN];
    char dest_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(ip_header->ip_src), source_ip, INET_ADDRSTRLEN); // IP de origen
    inet_ntop(AF_INET, &(ip_header->ip_dst), dest_ip, INET_ADDRSTRLEN);   // IP de destino

    // Obtener protocolo
    int protocol = ip_header->ip_p; // Campo ip_p contiene el número de protocolo (TCP, UDP, etc.)
    string protText;
    switch(protocol) {
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
        case IPPROTO_IGMP:
            protText = "IGMP";
            break;
        case IPPROTO_IPIP:
            protText = "IPIP";
            break;
        case IPPROTO_EGP:
            protText = "EGP";
            break;
        case IPPROTO_PUP:
            protText = "PUP";
            break;
        case IPPROTO_IDP:
            protText = "IDP";
            break;
        case IPPROTO_TP:
            protText = "TP";
            break;
        case IPPROTO_DCCP:
            protText = "DCCP";
            break;
        case IPPROTO_IPV6:
            protText = "IPV6";
            break;
        case IPPROTO_RSVP:
            protText = "RSVP";
            break;
        case IPPROTO_GRE:
            protText = "GRE";
            break;
        case IPPROTO_ESP:
            protText = "ESP";
            break;
        case IPPROTO_AH:
            protText = "AH";
            break;
        case IPPROTO_MTP:
            protText = "MTP";
            break;
        case IPPROTO_BEETPH:
            protText = "BEETPH";
            break;
        case IPPROTO_ENCAP:
            protText = "ENCAP";
            break;
        case IPPROTO_PIM:
            protText = "PIM";
            break;
        case IPPROTO_COMP:
            protText = "COMP";
            break;
        case IPPROTO_SCTP:
            protText = "SCTP";
            break;
        case IPPROTO_UDPLITE:
            protText = "UDPLITE";
            break;
        case IPPROTO_MPLS:
            protText = "MPLS";
            break;
        case IPPROTO_RAW:
            protText = "RAW";
            break;
        default:
            protText = to_string(protocol);
            break;
        
    }

    

    // Obtener tamaño del paquete
    int packet_size = ntohs(ip_header->ip_len);

    mvwprintw(subw.win, y, 0, "%5d %11f  %-15s %-15s  %4s  %5d", option_index + 1, list[option_index].elapsed_seconds.count(), source_ip, dest_ip, protText.c_str(), list[option_index].length);
}

void draw_rawData(SubWindow& subw, vector<DataBlocks>& list, int option_index, int y){
    mvwprintw(subw.win, y, 0, "%-48s  %-16s", list[option_index].hexData.c_str(), list[option_index].rawData.c_str());
}

void print_packet_info(SubWindow& subw, vector<string>& list, int option_index, int y) {
    mvwprintw(subw.win,y,0,"%*s", -subw.width, list[option_index].c_str());
}

void save_to_csv(const string& filename, MainList& mainList) {
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo para escritura\n";
        return;
    }

    // Encabezados
    file << "Packet No,Time,Source IP,Destination IP,Protocol,Length,Hex Data,Raw Data,Info\n";

    for (size_t i = 0; i < mainList.list.size(); i++) {
        mainList.current_selection = i; // Seleccionar el paquete actual
        
        // Generar bloques rawData e información
        vector<DataBlocks>& rawBlocks = mainList.splitIntoBlocks();
        vector<string>& info = mainList.getInfo();

        // Acceder al paquete actual
        PacketData& packet = mainList.list[i];

        // Extraer información básica del paquete
        const struct ip* ip_header = (struct ip*)(packet.data.data() + 14);
        char source_ip[INET_ADDRSTRLEN];
        char dest_ip[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(ip_header->ip_src), source_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ip_header->ip_dst), dest_ip, INET_ADDRSTRLEN);

        int protocol = ip_header->ip_p; // Campo ip_p contiene el número de protocolo (TCP, UDP, etc.)
    string protText;
    switch(protocol) {
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
        case IPPROTO_IGMP:
            protText = "IGMP";
            break;
        case IPPROTO_IPIP:
            protText = "IPIP";
            break;
        case IPPROTO_EGP:
            protText = "EGP";
            break;
        case IPPROTO_PUP:
            protText = "PUP";
            break;
        case IPPROTO_IDP:
            protText = "IDP";
            break;
        case IPPROTO_TP:
            protText = "TP";
            break;
        case IPPROTO_DCCP:
            protText = "DCCP";
            break;
        case IPPROTO_IPV6:
            protText = "IPV6";
            break;
        case IPPROTO_RSVP:
            protText = "RSVP";
            break;
        case IPPROTO_GRE:
            protText = "GRE";
            break;
        case IPPROTO_ESP:
            protText = "ESP";
            break;
        case IPPROTO_AH:
            protText = "AH";
            break;
        case IPPROTO_MTP:
            protText = "MTP";
            break;
        case IPPROTO_BEETPH:
            protText = "BEETPH";
            break;
        case IPPROTO_ENCAP:
            protText = "ENCAP";
            break;
        case IPPROTO_PIM:
            protText = "PIM";
            break;
        case IPPROTO_COMP:
            protText = "COMP";
            break;
        case IPPROTO_SCTP:
            protText = "SCTP";
            break;
        case IPPROTO_UDPLITE:
            protText = "UDPLITE";
            break;
        case IPPROTO_MPLS:
            protText = "MPLS";
            break;
        case IPPROTO_RAW:
            protText = "RAW";
            break;
        default:
            protText = to_string(protocol);
            break;
        
    }

        // Escribir los encabezados generales del paquete
        file << i + 1 << ",";
        file << packet.elapsed_seconds.count() << ",";
        file << source_ip << ",";
        file << dest_ip << ",";
        file << protText << ",";
        file << packet.length << ",";

        // Escribir el rawData y hexData (concatenados en una sola línea)
        string hexDataCombined, rawDataCombined;
        for (const auto& block : rawBlocks) {
            hexDataCombined += block.hexData + " ";
            rawDataCombined += block.rawData + " ";
        }

        file << "\"" << hexDataCombined << "\",";
        file << "\"" << rawDataCombined << "\",";

        // Escribir la información (getInfo) en múltiples líneas
        string infoCombined;
        for (const auto& line : info) {
            infoCombined += line + " | ";
        }
        file << "\"" << infoCombined << "\"\n";
    }

    file.close();
}
// void splitIntoBlocks(vector<DataBlocks>& blocks, vector<u_char>& data, bpf_u_int32 length) {
//     //vector<DataBlocks> blocks; // Estructura para almacenar los bloques

//     u_char *data_ptr = data.data();

//     // Iterar sobre el vector en bloques de 16 bytes
//     for (size_t i = 0; i < length; i += 16) {
//         DataBlocks block;
//         stringstream hexStream; // stringstream para almacenar el bloque en hexadecimal
//         stringstream rawStream; // stringstream para almacenar el bloque en formato "raw"

//         // Obtener el bloque actual de 16 bytes
//         for (size_t j = i; j < i + 16 && j < length; ++j) {
//             // Formatear y añadir a la lista en hexadecimal
//             hexStream << hex << setw(2) << setfill('0') << static_cast<int>(data_ptr[j]) << " ";
//             // Añadir al raw stream
            
//             if(data_ptr[j] >= 32 &&  data_ptr[j] <= 126)
//                 rawStream << (unsigned char)data_ptr[j];
//             else 
//                 rawStream << '.';
//         }
//         // mvwprintw(subw.win, y, 0, "%-48s  %-16s", hexStream.str(), rawStream.str());
//         // mvwprintw(subw.win, y, 0, "%d", data.size());
//         block.hexData = hexStream.str();
//         block.rawData = rawStream.str();

//         // Guardar las cadenas formateadas en sus respectivas listas
//         blocks.push_back(block);
//         // blocks.rawData.push_back(rawStream.str());
//     }
// }

MainList* mainList;
