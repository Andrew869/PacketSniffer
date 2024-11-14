#include <arpa/inet.h>
#include <netinet/ip.h>
#include <pcap/pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

int link_hdr_length = 0;

void call_me(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packetd_ptr) {
    packetd_ptr += link_hdr_length;
    struct ip *ip_hdr = (struct ip *)packetd_ptr;
 
    char packet_srcip[INET_ADDRSTRLEN];         // source ip address
    char packet_dstip[INET_ADDRSTRLEN];         // destination ip address
    strcpy(packet_srcip, inet_ntoa(ip_hdr->ip_src));
    strcpy(packet_dstip, inet_ntoa(ip_hdr->ip_dst));
    int packet_id = ntohs(ip_hdr->ip_id),       // identification
    packet_ttl = ip_hdr->ip_ttl,                // Time To Live
    packet_tos = ip_hdr->ip_tos,                // Type Of Service
    packet_len = ntohs(ip_hdr->ip_len),         // header length + data length
    packet_hlen = ip_hdr->ip_hl;                // header length

    printf("**************************************************************************\n");
    printf("ID: %d | SRC: %s | DST: %s | TOS: 0x%x | TTL: %d\n", packet_id, packet_srcip, packet_dstip, packet_tos, packet_ttl);

    packetd_ptr += (4 * packet_hlen);
    int protocol_type = ip_hdr->ip_p;

    struct tcphdr *tcp_header;
    struct udphdr *udp_header;
    struct icmp *icmp_header;
    int src_port, dst_port;

    switch (protocol_type) {
      case IPPROTO_TCP:
        tcp_header = (struct tcphdr *)packetd_ptr;
        src_port = tcp_header->th_sport;
        dst_port = tcp_header->th_dport;
        printf("PROTO: TCP | FLAGS: %c/%c/%c | SPORT: %d | DPORT: %d |\n",
               (tcp_header->th_flags & TH_SYN ? 'S' : '-'),
               (tcp_header->th_flags & TH_ACK ? 'A' : '-'),
               (tcp_header->th_flags & TH_URG ? 'U' : '-'), src_port, dst_port);
        break;
      case IPPROTO_UDP:
        udp_header = (struct udphdr *)packetd_ptr;
        src_port = udp_header->uh_sport;
        dst_port = udp_header->uh_dport;
        printf("PROTO: UDP | SPORT: %d | DPORT: %d |\n", src_port, dst_port);
        break;
      case IPPROTO_ICMP:
        icmp_header = (struct icmp *)packetd_ptr;
        int icmp_type = icmp_header->icmp_type;
        int icmp_type_code = icmp_header->icmp_code;
        printf("PROTO: ICMP | TYPE: %d | CODE: %d |\n", icmp_type, icmp_type_code);
        break;
    }
}

int main(int argc, char const *argv[]) {
    char device[50] = "";
    int packets_count = 10;
    char filters[100] = "port 80";

    if (argc >= 2) {
        strcpy(device, argv[1]);
    } else {
        printf("Falta el nombre de la interfaz como argumento\n");
        exit(0);
    }

    char error_buffer[PCAP_ERRBUF_SIZE];

    pcap_t *capdev = pcap_open_live(device, BUFSIZ, 0, -1, error_buffer);

    int link_hdr_type = pcap_datalink(capdev);

    if (capdev == NULL) {
        printf("ERR: pcap_open_live() %s\n", error_buffer);
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

    if (pcap_loop(capdev, packets_count, call_me, (u_char *)NULL)) {
        printf("ERR: pcap_loop() failed!\n");
        exit(1);
    }

    return 0;
}