/*
 * virtual_switch_server.cpp
 *
 *  Created on: Oct 3, 2020
 *      Author: zbc
 */

#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <map>
#include <thread>
#include "utils.h"

std::map<std::array<unsigned char, 6>, int> data;
struct sockaddr clients[16]; //16端口
bool connected[16];//已连接

void processPacket(struct sockaddr_in *addr_client, char *recv_buf,
		int recv_num, int sock_fd, int len) {
	int port_num = (int) recv_buf[0];
	clients[port_num] = *((struct sockaddr*) addr_client);
	//std::cout << "received " << recv_num << " bytes from port " << port_num << std::endl;
	std::array<unsigned char, 6> smac, dmac;
	for (int i = 0; i < 6; i++) {
		dmac[i] = recv_buf[1 + i];
	}
	for (int i = 0; i < 6; i++) {
		smac[i] = recv_buf[7 + i];
	}
	auto item = data.find(smac);
	if (item == data.end()) {
		char *smac_str = phex(&recv_buf[7], 6);
		std::cout << smac_str << " connected on port " << port_num << std::endl;
		connected[port_num]=true;
		free(smac_str);
		data.insert(
				std::pair<std::array<unsigned char, 6>, int>(smac, port_num));
	} else {
		if (item->second != port_num) {
			item->second = port_num;
			char *smac_str = phex(&recv_buf[7], 6);
			std::cout << smac_str << " connected on port " << port_num
					<< std::endl;
			free(smac_str);
		}
	}
	item = data.find(dmac);
	if (item == data.end()) {
		//处理广播包
		for (int i = 0; i < 16; i++) {
			if (i == port_num) {
				continue;
			}
			if (connected[i]) {
				unsigned char send_buf[10000];
				memcpy(send_buf, &recv_buf[1], recv_num - 1);
				int send_num = sendto(sock_fd, send_buf, recv_num - 1, 0,
						&clients[i], len);
				char *smac_str = phex(&recv_buf[7], 6);
				free(smac_str);
				if (send_num < 0) {
					perror("sendto error:");
				}
			}
		}
	} else {
		unsigned char send_buf[10000];
		memcpy(send_buf, &recv_buf[1], recv_num - 1);

		int send_num = sendto(sock_fd, send_buf, recv_num - 1, 0,
				&clients[item->second], len);
		if (send_num < 0) {
			perror("sendto error:");
		}
	}
	delete recv_buf;
}

int main(int argc, char **argv) {

	/* sock_fd --- socket文件描述符 创建udp套接字*/
	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		perror("socket");
		exit(1);
	}
	/* 将套接字和IP、端口绑定 */
	struct sockaddr_in addr_serv;
	int len;
	memset(&addr_serv, 0, sizeof(struct sockaddr_in));  //每个字节都用0填充
	addr_serv.sin_family = AF_INET;  //使用IPV4地址
	addr_serv.sin_port = htons(5000);  //端口
	/* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是SERV_PORT，就会被该应用程序接收到 */
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);  //自动获取IP地址
	len = sizeof(addr_serv);

	/* 绑定socket */
	if (bind(sock_fd, (struct sockaddr*) &addr_serv, sizeof(addr_serv)) < 0) {
		perror("bind error:");
		exit(1);
	}

	int recv_num;
	int send_num;
	std::cout << "server started." << std::endl;

	struct sockaddr_in *addr_client = new struct sockaddr_in;

	while (1) {
		char *recv_buf = new char[10000];
		recv_num = recvfrom(sock_fd, recv_buf, 10000, 0,
				(struct sockaddr*) addr_client, (socklen_t*) &len);
		if (recv_num < 0) {
			perror("recvfrom error:");
		} else {
			std::thread(processPacket, addr_client, recv_buf, recv_num, sock_fd,
					len).detach();
		}
	}
	close(sock_fd);
	return 0;
}

