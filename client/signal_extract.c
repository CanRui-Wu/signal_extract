#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "radiotap/radiotap_iter.h"

struct sockaddr_in server_addr;
int ap_index;  //Tell server which AP you are
int socket_fd; //The socket of client

static const struct radiotap_align_size align_size_000000_00[] = {
	[0] = { .align = 1, .size = 4, },
	[52] = { .align = 1, .size = 4, },
};

static const struct ieee80211_radiotap_namespace vns_array[] = {
	{
		.oui = 0x000000,
		.subns = 0,
		.n_bits = sizeof(align_size_000000_00),
		.align_size = align_size_000000_00,
	},
};

static const struct ieee80211_radiotap_vendor_namespaces vns = {
	.ns = vns_array,
	.n_ns = sizeof(vns_array)/sizeof(vns_array[0]),
};


int get_ssi_signal(struct ieee80211_radiotap_iterator *iter) {
	if (iter->this_arg_index == IEEE80211_RADIOTAP_DBM_ANTSIGNAL)
		return (int)(*iter->this_arg-256);
	return 0;
}


/**
	*@description: some preparation for tcp connection
	*@return: int, if error return -1, else return the fd of socket
	*@Called By: main
**/
int tcp_client_init() {
	int socket_fd = socket(AF_INET,SOCK_STREAM,0);
	int result = connect(socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
	if(result == -1) {
		return -1;
	}
	return socket_fd;
}


void packet_handler(u_char *user,const struct pcap_pkthdr * pkthdr,const u_char *packet) {
	struct ieee80211_radiotap_iterator iter;
	int err = ieee80211_radiotap_iterator_init(&iter, (struct ieee80211_radiotap_header *)packet,256, &vns);
	if (err) {
		printf("malformed radiotap header (init returns %d)\n", err);
		exit(1);
	}
	int ssi_signal = 0;
	while (!(err = ieee80211_radiotap_iterator_next(&iter))) {
		if (iter.is_radiotap_ns)
			ssi_signal = get_ssi_signal(&iter);
		if (ssi_signal)
			break;
	}
	int radiotap_len = packet[2];
	if(packet[radiotap_len] != 0x40) { //Ignore the frame which is not probe request
		return;
	}
	printf("success to capture a probe request\n");
	printf("Whole packet length :%u\n",pkthdr->len);
	printf("Actually capture number of bytes:%u\n",pkthdr->caplen);
	printf("Radiotap total length : %d bytes\n",radiotap_len);
	printf("SSI SIGNAL %d dBm\n", ssi_signal);
	int buf[8];
	int address_index = radiotap_len + 10;
	int i;
	for(i = 0;i < 6;i++) {
		buf[i] = htonl(packet[address_index+i]);
	}
	buf[6] = htonl(ssi_signal);
	buf[7] = htonl(ap_index);
	write(socket_fd,buf,8*sizeof(int));
}



int main(int argc,char *argv[]) {
	char *device;
	char *server_ip;
	char *server_port;
	char *ap;
	if (argc == 5) {
		device = argv[1];
		server_ip = argv[2];
		server_port = argv[3];
		ap = argv[4];
	}
	ap_index = atoi(ap);
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *fd = pcap_open_live(device,65535,0,0,errbuf);
	if (fd) {
		printf("open %s is ok\n",device);
	}
	else {
		printf("fail to open %s , %s\n",device,errbuf);
		exit(1);
	}
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	server_addr.sin_port = htons(atoi(server_port));
	socket_fd = tcp_client_init();
	if(socket_fd == -1) {
		printf("Fail to create socket");
		exit(1);
	}
	pcap_loop(fd,-1,packet_handler,NULL);
	pcap_close(fd);
	return 0;
}