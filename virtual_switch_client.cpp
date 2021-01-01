#include <iostream>
#include <cstring>
#include <cassert>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <thread>

#include "utils.h"

//using namespace std;

void processPacketFromUDP(struct sockaddr *dst, int socks_fd, int tun_fd) {
	std::cout << "UDP received process thread started." << std::endl;
	unsigned char buf[10000];
	struct sockaddr_in src;
	socklen_t len;
	len = sizeof(*dst);
	int recv_num;
	while (1) {
		recv_num = recvfrom(socks_fd, buf, sizeof(buf), 0,
				(struct sockaddr*) &src, &len); //接收来自server的信息
		if (recv_num < 0) {
			perror("recvfrom error:");
		} else {
			write(tun_fd, buf, recv_num);
			/*char* contents_sent=phex(buffer, nread);*/
			//printf("Write %d bytes to tun/tap device.\n", recv_num);
		}
	}
	printf("server:%s\n", buf);
}

int tap_alloc(char *dev, int flags) {
	assert(dev != NULL);

	struct ifreq ifr;
	int fd, err;
	char *clonedev = (char*) "/dev/net/tun";
	if ((fd = open(clonedev, O_RDWR)) < 0) {
		return fd;
	}
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = flags;
	if (*dev != '\0') {
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}
	if ((err = ioctl(fd, TUNSETIFF, (void*) &ifr)) < 0) {
		close(fd);
		return err;
	}
	strcpy(dev, ifr.ifr_name);
	return fd;
}

int main(int argc, char **argv) {
	std::cout << "enter server ip and port and port_num:";
	std::string ip;
	int port, port_num;
	std::cin >> ip >> port >> port_num;
	int client_fd;
	struct sockaddr_in *ser_addr = new struct sockaddr_in;

	client_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_fd < 0) {
		printf("Create socket fail!\n");
		return -1;
	}
	memset(ser_addr, 0, sizeof(*ser_addr));
	ser_addr->sin_family = AF_INET;
	ser_addr->sin_addr.s_addr = inet_addr(ip.c_str());
	//ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //注意网络序转换
	ser_addr->sin_port = htons(port);  //注意网络序转换
	int tun_fd, nread;
	char tun_name[IFNAMSIZ];

	tun_name[0] = '\0';
	tun_fd = tap_alloc(tun_name, IFF_TAP | IFF_NO_PI);

	if (tun_fd < 0) {
		perror("Allocating interface");
		exit(1);
	}

	printf("Tap device: %s created\n", tun_name);
	std::thread udp_process(processPacketFromUDP, (sockaddr*) ser_addr,
			client_fd, tun_fd);
	udp_process.detach();
	std::cout << "Start listening from interface..." << std::endl;
	unsigned char buffer[10001];
	buffer[0] = port_num;
	while (1) {
		nread = read(tun_fd, &buffer[1], sizeof(buffer) - 1);  //收包
		if (nread < 0) {
			perror("Reading from interface");
			close(tun_fd);
			exit(1);
		}
		sendto(client_fd, buffer, nread + 1, 0, (sockaddr*) ser_addr,
				sizeof(*ser_addr));
		//char *contents_received = phex(buffer, nread);
	}
	return 0;
}
