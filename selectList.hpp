struct DataBlocks {
    string hexData; // Lista de cadenas en formato hexadecimal
    string asciiData; // Lista de cadenas en formato "raw"
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

vector<PacketData>* packets;
int* packetIndex;

// void draw_asciiData(SubWindow& subw, vector<DataBlocks>& list, int option_index, int y);

class BaseList {
public:
    virtual ~BaseList() = default;
    virtual void move_selection(int direction) = 0;
};

template<typename T>
class List : public BaseList{
public:
    // Constructor que inicializa el vector
    List(vector<T>& list, short listHeight)
    : list(list), listHeight(listHeight), current_selection(0), scroll_start(0) {}
    
    int current_selection;
    int scroll_start;
    vector<T>& list;
    short listHeight;

    int* GetCurrIndex() {
        return &current_selection;
    }

    void move_selection(int direction) override {
        int new_selection = current_selection + direction;

        if (new_selection >= 0 && new_selection < static_cast<int>(list.size())) {
            current_selection = new_selection;
        }

        if (current_selection < scroll_start) {
            scroll_start = current_selection;
        } else if (current_selection >= scroll_start + listHeight) {
            scroll_start = current_selection - listHeight + 1;
        }
    }
};

template<typename T>
class DerList : public List<T> {
public:
    DerList(vector<T>& list, short listHeight)
    : List<T>(list, listHeight) {
    }

    void move_selection(int dir) override {
        this->List<T>::move_selection(dir);
        // win2->erase();
        // mvwprintw(win2->win, 0, 0, "%d", this->current_selection);
        // mvwprintw(win2->win, 1, 0, "%d", this->scroll_start);
        // mvwprintw(win2->win, 2, 0, "%d", this->list.size());
        // win2->refresh();
    }

};

class MainList : public List<PacketData> {
public:
    MainList(vector<PacketData>& list, short listHeight)
        : List<PacketData>(list, listHeight){}

    DerList<DataBlocks>* rawList;
    vector<DataBlocks> generatedBlocks;

    void move_selection(int direction) {
        this->List<PacketData>::move_selection(direction);
        PacketData* packet = reinterpret_cast<PacketData*>(&this->list[this->current_selection]);
        // CreateSecLists();
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
            block.asciiData = rawStream.str();
            generatedBlocks.push_back(block);
        }

        return generatedBlocks; // Devolver referencia al vector almacenado
    }

    // void CreateSecLists() {
    //     rawList = new DerList<DataBlocks>(win3->subw, splitIntoBlocks(), draw_asciiData);
    //     rawList->subw.erase();
    //     rawList->draw_list();
    // }
};

// vector<PacketData> packets;
// vector<PacketData> packets;
int autoScroll = true;

string GetProtocolText(int protocol) {
    // Crear un mapa que asocia cada protocolo con su texto
    static const std::map<int, std::string> protocolMap = {
        {IPPROTO_TCP, "TCP"},
        {IPPROTO_UDP, "UDP"},
        {IPPROTO_ICMP, "ICMP"},
        {IPPROTO_IP, "IP"},
        {IPPROTO_IGMP, "IGMP"},
        {IPPROTO_IPIP, "IPIP"},
        {IPPROTO_EGP, "EGP"},
        {IPPROTO_PUP, "PUP"},
        {IPPROTO_IDP, "IDP"},
        {IPPROTO_TP, "TP"},
        {IPPROTO_DCCP, "DCCP"},
        {IPPROTO_IPV6, "IPV6"},
        {IPPROTO_RSVP, "RSVP"},
        {IPPROTO_GRE, "GRE"},
        {IPPROTO_ESP, "ESP"},
        {IPPROTO_AH, "AH"},
        {IPPROTO_MTP, "MTP"},
        {IPPROTO_BEETPH, "BEETPH"},
        {IPPROTO_ENCAP, "ENCAP"},
        {IPPROTO_PIM, "PIM"},
        {IPPROTO_COMP, "COMP"},
        {IPPROTO_L2TP, "L2TP"},
        {IPPROTO_SCTP, "SCTP"},
        {IPPROTO_UDPLITE, "UDPLITE"},
        {IPPROTO_MPLS, "MPLS"},
        {IPPROTO_ETHERNET, "ETHERNET"},
        {IPPROTO_RAW, "RAW"},
        {IPPROTO_MPTCP, "MPTCP"}
    };

    // Intentar encontrar el protocolo en el mapa
    auto it = protocolMap.find(protocol);
    if (it != protocolMap.end()) {
        return it->second;  // Si lo encuentra, devuelve el texto correspondiente
    }

    // Si no lo encuentra, devolver el número de protocolo como cadena
    return std::to_string(protocol);
}

void main_packet_data(WINDOW* win, vector<PacketData>& list, int option_index, int y, int width) {
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

    // Obtener tamaño del paquete
    int packet_size = ntohs(ip_header->ip_len);

    mvwprintw(win, y, 0, "%5d %11f  %-15s %-15s  %4s  %5d", option_index + 1, list[option_index].elapsed_seconds.count(), source_ip, dest_ip, GetProtocolText(protocol).c_str(), list[option_index].length);
}

void DrawRawData(WINDOW* win, vector<DataBlocks>& list, int option_index, int y, int width){
    mvwprintw(win, y, 0, "%-48s  %-16s", list[option_index].hexData.c_str(), list[option_index].asciiData.c_str());
}

void print_packet_info(WINDOW* win, vector<string>& list, int option_index, int y, int width) {
    mvwprintw(win, y, 0, "%*s", -width, list[option_index].c_str());
}

void DrawString(WINDOW* win, vector<string>& list, int option_index, int y, int width){
    mvwprintw(win, y, 0, "%*s", -width ,list[option_index].c_str());
}

string getCurrentTimeString() {
    // Obtener la fecha y hora actual
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // Convertir a formato legible
    std::tm tm_time = *std::localtime(&now_time);

    // Crear un stringstream para formatear la fecha y hora
    std::stringstream ss;
    ss << std::put_time(&tm_time, "%Y-%m-%d_%H-%M-%S");

    return ss.str();
}

MainList* mainList;