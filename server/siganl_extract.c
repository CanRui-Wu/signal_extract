#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 80
#define SERVER_PORT 8000

int main() {
	struct sockaddr_in server_addr,client_addr;  //网络套接字地址结构体
	int listen_fd = socket(AF_INET,SOCK_STREAM,0); //打开一个网络通讯端口，用来初始化监听套接口的描述字
	int conn_fd; //定义连接套接口的描述字
	int buf[MAX]; //定义缓冲区

	bzero(&server_addr,sizeof(server_addr)); //清空服务端套接字(bzero将内存前n个字节置为0)

	server_addr.sin_family = AF_INET;     //地址采用IPv4地址，htonl和htons将地址和端口主机字节序转为网络字节序
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY相当于所有地址，inet_addr("0.0.0.0")
	server_addr.sin_port = htons(SERVER_PORT);
	bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)); //将地址和端口绑到监听描述符上，要先将sockaddr_in转为sockaddr

	listen(listen_fd,20); //将套接口从CLOSED状态转为LISTEN状态，后面的20表示最大同时连接的客户端
	printf("等待客户端连接中...\n");

	while(1) {
		int client_addr_len = sizeof(client_addr);
		//accept函数返回新的连接描述字，client_addr被置为客户端套接字，后面的是套接字长度
		conn_fd = accept(listen_fd,(struct sockaddr*)&client_addr,&client_addr_len); 
		int n = read(conn_fd,buf,MAX); //从已连接套接字中读取数据到buf中
		int i;
		printf("AP%d --- Receive the station of ",ntohl(buf[7]));
		for(i = 0;i < 6;i++) {
			printf("%02x",ntohl(buf[i]));
			if(i != 5)
				printf(":");
		}
		printf(",its signal is %d dBM\n",ntohl(buf[6]));
		close(conn_fd);
	}
	
}