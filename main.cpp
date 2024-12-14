#include "includes.hpp"
#include "selectList.hpp"
#include "display.hpp"

#define cmvprintw(c, y, x, format, ...) \
    do { \
        attron(A_REVERSE); \
        mvprintw(y, x, format, ##__VA_ARGS__); \
        attroff(A_REVERSE); \
    } while (0)

#define STATE_M 0
#define STATE_I 1
#define STATE_F 2
#define STATE_S 3


int link_hdr_length = 0;

void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr);
void *threadpcap(void *arg);
void MainLoop();
void GetDevices(vector<string>* devs);
void ChangeState(short stateID);
void SetupPCAP(const char* devname);

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

class State {
public:
    State(vector<BaseParentWin*> windows) : windows(windows) {}
    
    vector<BaseParentWin*> windows;
    short currentWin = 0;

    virtual ~State() = default;    

    void DrawState(){
        for(auto var : windows) {
            var->DrawBorder();
            var->DrawSubWindow();
        }
    }

    void EraseState() {
        for(auto parent : windows) {
            parent->erase();
            parent->EraseSubWindow();
            parent->refresh();
        }
    }
    
    virtual void HandleKeyPress(short& currentState, int keyPressed) = 0;
};

class StateM : public State{
public:
    StateM(vector<BaseParentWin*> windows) : State(windows){}
    void HandleKeyPress(short& currentState, int keyPressed) override {
        switch (keyPressed) {
        case 'W':
        case 'w':
        case KEY_UP:
            //todo
            break;
        case 'S':
        case 's':
        case KEY_DOWN:
            //todo
            break;
        case 9:
            ChangeState(STATE_I);
            // mvprintw(0,0, "StateI");
            break;
        }
    }
};

class StateI : public State{
public:
    StateI(vector<BaseParentWin*> windows) : State(windows){}
    void HandleKeyPress(short& currentState, int keyPressed) override {
        switch (keyPressed) {
        case 'W':
        case 'w':
        case KEY_UP:
            this->windows[0]->moveSelection(-1);
            this->windows[0]->DrawSubWindow();
            break;
        case 'S':
        case 's':
        case KEY_DOWN:
            this->windows[0]->moveSelection(1);
            this->windows[0]->DrawSubWindow();
            break;
        case 10:
            vector<string>* tmp = dynamic_cast<ParentWin<string>*>(this->windows[currentWin])->GetList();
            // devname = tmp->at(this->windows[currentWin]->GetCurrSelect()).c_str();
            SetupPCAP(tmp->at(this->windows[currentWin]->GetCurrSelect()).c_str());
            ChangeState(STATE_M);
            // mvprintw(0,0, "StateM");
            break;
        }
    }
};

// int state::CurrentID = 0;

vector<State*> states;
short currentState, prevState;

int main(int argc, char const *argv[]) {
    InitDisplay(max_y, max_x);
    refresh();

    // mainList = new MainList(win1->subw, packets, main_packet_data);

    // clear();
    // refresh();
    // win1 = new ParentWin(max_y / 2, max_x / 2, 0, 0);
    // win2 = new ParentWin(max_y / 2, max_x / 2, max_y / 2, 0);
    // win3 = new ParentWin(max_y, max_x / 2, 0, max_x / 2);
    // win4 = new ParentWin(max_y / 2, max_x / 2, 0, 0);

    states.push_back(new StateM({
        new ParentWin(max_y / 2, max_x / 2, 0, 0, main_packet_data),
        new ParentWin(max_y, max_x / 2, 0, max_x / 2, DrawRawData)
        // new ParentWin<DataBlocks>(max_y / 2, max_x / 2, max_y / 2, 0)
    }));
    states.emplace_back(new StateI({new ParentWin(max_y / 2, max_x / 2, 0, 0, DrawString)}));
    auto parentWin = dynamic_cast<ParentWin<string>*>(states[STATE_I]->windows[0]);
    if (parentWin) {
        vector<string>* list = parentWin->GetList();
        GetDevices(list);
    } else {
        std::cerr << "Error: El objeto no es de tipo ParentWin<string>" << std::endl;
    }

    states[STATE_I]->DrawState();

    currentState = STATE_I;

    MainLoop();

    // Cerrar la captura
    pcap_close(capdev);

    EndDisplay();

    return 0;
}

void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr) {
    end_time = steady_clock::now();
    duration<double> elapsed_seconds = duration_cast<duration<double>>(end_time - start_time);

    auto parentWin = dynamic_cast<ParentWin<PacketData>*>(states[STATE_M]->windows[0]);
    if (parentWin) {
        vector<PacketData>* list = parentWin->GetList();
        list->push_back({elapsed_seconds, packetd_ptr, pkthdr->len});
    } else {
        std::cerr << "Error: El objeto no es de tipo ParentWin<PacketData>" << std::endl;
    }
    // vector<PacketData>* packets = dynamic_cast<ParentWin<PacketData>*>(states[currentState]->windows[0])->GetList();

    // packets->push_back({elapsed_seconds, packetd_ptr, pkthdr->len});

    if(autoScroll){
        states[currentState]->windows[0]->moveSelection(1);
        states[currentState]->windows[0]->DrawSubWindow();
    }
}

void *threadpcap(void *arg) {
    pcap_t *capdev = (pcap_t *)arg;
    int packets_count = -1;

    int result = pcap_loop(capdev, packets_count, packetManager, nullptr);
    if (result == -1) {
        fprintf(stderr, "ERR: pcap_loop() failed: %s\n", pcap_geterr(capdev));
        exit(1);
    } else if (result == -2) {
        printf("pcap_loop() finalizado por pcap_breakloop().\n");
    }

    return NULL;
}

void MainLoop(){
    int keyPressed = 0;
    while ((keyPressed = getch()) != 'q') {
        // mvprintw(0, 0, "%d", keyPressed);
        states[currentState]->HandleKeyPress(currentState, keyPressed);
    }
    if (capdev) {
        pcap_breakloop(capdev);  // Finalizar el pcap_loop
        pthread_join(captureThread, NULL);
    }
}

void GetDevices(vector<string>* devs){
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

void ChangeState(short stateID) {
    prevState = currentState;
    currentState = stateID;
    states[prevState]->EraseState();
    states[currentState]->DrawState();
}

void SetupPCAP(const char* devname) {
    char error_buffer[PCAP_ERRBUF_SIZE];
    int packets_count = -1;
    char filters[] = ""; // Ajustar filtro si es necesario

    // Abrir el dispositivo
    capdev = pcap_open_live(devname, 65535, 1, -1, error_buffer);
    if (capdev == NULL) {
        printf("ERR: pcap_open_live() failed: %s\n", error_buffer);
        exit(1);
    }

    // Obtener tipo de enlace
    int link_hdr_type = pcap_datalink(capdev);

    // Configurar filtro BPF
    struct bpf_program bpf;
    bpf_u_int32 netmask = 0xFFFFFF; // Netmask predeterminado
    if (pcap_compile(capdev, &bpf, filters, 0, netmask) == PCAP_ERROR) {
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
    int threadError = pthread_create(&captureThread, NULL, threadpcap, (void*)capdev);
    if (threadError) {
        fprintf(stderr, "Error al crear el hilo: %d\n", threadError);
        pcap_close(capdev);
        exit(1);
    }

    start_time = steady_clock::now();
}
