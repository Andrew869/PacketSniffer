void packetManager(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr);

void* ThreadPCAP(void* arg) {
    pcap_t *handle = (pcap_t *)arg;
    int packets_count = -1;

    int result = pcap_loop(handle, packets_count, packetManager, nullptr);
    if (result == -1) {
        fprintf(stderr, "ERR: pcap_loop() failed: %s\n", pcap_geterr(handle));
        exit(1);
    } 
    // else if (result == -2) {
    //     printf("pcap_loop() finalizado por pcap_breakloop().\n");
    // }

    return NULL;
}

class PacketCapture {
public:
    PacketCapture() {}
    PacketCapture(string devName) : currDevName(devName) {}

    void open() {
        handle = pcap_open_live(currDevName.c_str(), 65535, 1, 1000, errbuf);
        if (handle == NULL) {
            conWin->PrintM("ERR: pcap_open_live() failed: %s\n", errbuf);
            // printf("ERR: pcap_open_live() failed: %s\n", errbuf);
            // exit(1);
        }
    }

    // int GetNetMask() {
    //     return pcap_lookupnet(currDevName.c_str(), &net, &mask, errbuf) == PCAP_ERROR;
    // }

    int SetFilters(string newFilters) {
        // strcpy(filters, NewFilters);
        bool isCapturingPrev = isCapturing;
        filters = newFilters;
        StopCapture();
        pcap_lookupnet(currDevName.c_str(), &net, &mask, errbuf);
        if (pcap_compile(handle, &bpf, filters.c_str(), 0, net) == PCAP_ERROR) {
            conWin->PrintM("ERROR: Invalid Filter Expressions", pcap_geterr(handle));
            // conWin->PrintM("Puerto establecido: %s", filters.c_str());
            return 0;
        }
        if (pcap_setfilter(handle, &bpf)) {
            // conWin->PrintM("ERR: pcap_setfilter() failed: %s\n", pcap_geterr(handle));
            pcap_freecode(&bpf);
            return 0;
        }
        pcap_freecode(&bpf);
        if(isCapturingPrev) StartCapture();
        return 1;
        // conWin->PrintM("Puerto establecido: %s", filters.c_str());
    }

    void StartCapture(bool resetTimer = false) {
        if(!isCapturing){
            pthread_create(&captureThread, NULL, ThreadPCAP, (void*)handle);
            if(resetTimer) start_time = steady_clock::now();
            isCapturing = true;
            conWin->PrintM("Packet capture started");
        }
    }

    void StopCapture() {
        if(isCapturing){
            pcap_breakloop(handle);  // Finalizar el pcap_loop
            pthread_join(captureThread, NULL);
            isCapturing = false;
            conWin->PrintM("Packet capture stopped");
        }
    }

    void close() {
        if(isCapturing){
            pcap_breakloop(handle);  // Finalizar el pcap_loop
            pthread_join(captureThread, NULL);
            pcap_close(handle);
            isCapturing = false;
        }
    }

    duration<double> GetElapsedTime() {
        end_time = steady_clock::now();
        return duration_cast<duration<double>>(end_time - start_time);
    }

    bool isCapturing = false;
    pcap_t *handle;
    pthread_t captureThread;
    string currDevName;
    // char filters[100];
    string filters;
    steady_clock::time_point start_time, end_time;
    char errbuf[PCAP_ERRBUF_SIZE];
    // int packets_count = -1;
    struct bpf_program bpf;
    bpf_u_int32 net;  // Dirección de red
    bpf_u_int32 mask;  // Máscara de red
};

PacketCapture* packetCapture;