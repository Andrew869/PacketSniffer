#include <pcap/pcap.h>
#include <stdio.h>
#include <stdlib.h>

void call_me(u_char *user, const struct pcap_pkthdr *pkthdr, 
            const u_char *packetd_ptr) {
  printf("You just recieved a packet!\n");
}

int main(int argc, char const *argv[]) {
  const char *device = "wlo1"; // remember to replace this with your device name
  char error_buffer[PCAP_ERRBUF_SIZE];
  int packets_count = 5;

  pcap_t *capdev = pcap_open_live(device, BUFSIZ, 0, -1, error_buffer);

  if (capdev == NULL) {
    printf("ERR: pcap_open_live() %s\n", error_buffer);
    exit(1);
  }

  if (pcap_loop(capdev, packets_count, call_me, (u_char *)NULL)) {
    printf("ERR: pcap_loop() failed!\n");
    exit(1);
  }
  
  return 0;
}