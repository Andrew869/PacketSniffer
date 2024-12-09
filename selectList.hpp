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
            generatedBlocks.push_back(block);
        }

        return generatedBlocks; // Devolver referencia al vector almacenado
    }

    void CreateSecLists() {
        // vector<DataBlocks> blocks;
        // rawList = DerList<DataBlocks>(win3->subw, splitIntoBlocks(), draw_rawData);
        // rawList = std::make_unique<DerList<DataBlocks>>(win3->subw, splitIntoBlocks(), draw_rawData);
        rawList = new DerList<DataBlocks>(win3->subw, splitIntoBlocks(), draw_rawData);
        rawList->subw.erase();
        rawList->draw_list();
        // win2->erase();
        // mvwprintw(win2->win, 0, 0, "%d", rawList->list.size());
        // win2->refresh();
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

    mvwprintw(subw.win, y, 0, "%5d %11f  %-15s %-15s  %4s  %5d", option_index + 1, list[option_index].elapsed_seconds.count(), source_ip, dest_ip, protText.c_str(), list[option_index].length);
}

void draw_rawData(SubWindow& subw, vector<DataBlocks>& list, int option_index, int y){
    mvwprintw(subw.win, y, 0, "%-48s  %-16s", list[option_index].hexData.c_str(), list[option_index].rawData.c_str());
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
// // std::unique_ptr<DerList<DataBlocks>> rawList;

// void CreateSecLists(vector<u_char>& data, bpf_u_int32 length) {
//     vector<DataBlocks> blocks;
//     splitIntoBlocks(blocks, data, length);
//     rawList = std::make_unique<DerList<DataBlocks>>(win3->subw, blocks, draw_rawData);
//     rawList->subw.erase();
//     rawList->draw_list();
//     // win2->erase();
//     // mvwprintw(win2->win, 0, 0, "%d", rawList->list.size());
//     // win2->refresh();
// }