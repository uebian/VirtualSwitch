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

void processPacketFromTAP(int fd, struct sockaddr *dst, unsigned char *buffer,
		int nread, int port_num) {
	/*unsigned char smac[6], dmac[6];
	 //解析源MAC和目的MAC
	 memcpy(dmac, &buffer[0], 6);
	 memcpy(smac, &buffer[6], 6);
	 unsigned char buf[10000];*/
	socklen_t len;
	len = sizeof(*dst);
	unsigned char buf[10000];
	memcpy(&buf[1], buffer, nread);
	buf[0] = port_num;
	sendto(fd, buf, nread + 1, 0, dst, len);
	//printf("sent %d bytes.\n",nread + 1);


}

void processPacketFromUDP(struct sockaddr *dst, int socks_fd,int tun_fd) {
	std::cout<<"UDP received process thread started."<<std::endl;
	unsigned char buf[10000];
	struct sockaddr_in src;
	socklen_t len;
	len = sizeof(*dst);
	int recv_num;
	while (1) {
		recv_num = recvfrom(socks_fd, buf, sizeof(buf), 0, (struct sockaddr*) &src,
				&len); //接收来自server的信息
		if (recv_num < 0) {
			perror("recvfrom error:");
		} else {
			write(tun_fd, buf, recv_num);
			/*char* contents_sent=phex(buffer, nread);
			 printf("Write %d bytes to tun/tap device, that's %s\n", nread,contents_sent);
			 free(contents_sent);*/
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
	// 一旦设备开启成功，系统会给设备分配一个名称，对于tun设备，一般为tunX，X为从0开始的编号；
	// 对于tap设备，一般为tapX
	strcpy(dev, ifr.ifr_name);
	return fd;
}

int main(int argc, char **argv) {
	std::cout << "enter server ip and port and port_num:";
	std::string ip;
	int port, port_num;
	std::cin >> ip >> port >> port_num;
	int client_fd;
	struct sockaddr_in* ser_addr=new struct sockaddr_in;

	client_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_fd < 0) {
		printf("create socket fail!\n");
		return -1;
	}

	memset(ser_addr, 0, sizeof(*ser_addr));
	ser_addr->sin_family = AF_INET;
	ser_addr->sin_addr.s_addr = inet_addr(ip.c_str());
	//ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //注意网络序转换
	ser_addr->sin_port = htons(port);  //注意网络序转换
	int tun_fd, nread;
	unsigned char buffer[10000];
	char tun_name[IFNAMSIZ];

	tun_name[0] = '\0';
	tun_fd = tap_alloc(tun_name, IFF_TAP | IFF_NO_PI);

	if (tun_fd < 0) {
		perror("Allocating interface");
		exit(1);
	}

	printf("Tap device: %s created\n", tun_name);
	std::thread udp_process(processPacketFromUDP,(sockaddr*)ser_addr,client_fd ,tun_fd).detach();
	std::cout<<"start listening from interface..."<<std::endl;
	while (1) {
		nread = read(tun_fd, buffer, sizeof(buffer));//收包
		if (nread < 0) {
			perror("Reading from interface");
			close(tun_fd);
			exit(1);
		}
		std::thread(processPacketFromTAP, client_fd, (sockaddr*)ser_addr, buffer, nread,port_num).detach();
		//char *contents_received = phex(buffer, nread);

		/*char *smac_str = phex(smac, 6);
		 char *dmac_str = phex(dmac, 6);
		 printf("Read %d bytes from tun/tap device, SMAC:%s, DMAC:%s\n", nread,
		 smac_str, dmac_str);
		 free(contents_received);
		 free(smac_str);
		 free(dmac_str);*/
		//buffer[20] = 0;
		//*((unsigned short*) &buffer[22]) += 8;
		// 发包
		/*nread = write(tun_fd, buffer, nread);
		 char* contents_sent=phex(buffer, nread);
		 printf("Write %d bytes to tun/tap device, that's %s\n", nread,contents_sent);
		 free(contents_sent);*/
	}
	return 0;
}
