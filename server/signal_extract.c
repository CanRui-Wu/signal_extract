#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX 80
#define SERVER_PORT 8000
#define MAX_CONN 10

void *handle_client_conn(void *arg) {
	int buf[MAX]; //定义缓冲区
	int *p_conn_fd = (int *)arg;
	int conn_fd = *p_conn_fd;
	while(1) {
		int n = read(conn_fd,buf,MAX); //从已连接套接字中读取数据到buf中
		if (n == 0) {
			printf("Close the connection\n");
			close(conn_fd);
			pthread_exit(NULL);
		}
		int i;
		printf("AP%d --- Receive the station of ",ntohl(buf[7]));
		for(i = 0;i < 6;i++) {
			printf("%02x",ntohl(buf[i]));
			if(i != 5)
				printf(":");
		}
		printf(",its signal is %d dBM\n",ntohl(buf[6]));
	}
	close(conn_fd);
	return NULL;
}

int main() {
	struct sockaddr_in server_addr,client_addr;  //网络套接字地址结构体
	int listen_fd = socket(AF_INET,SOCK_STREAM,0); //打开一个网络通讯端口，用来初始化监听套接口的描述字
	bzero(&server_addr,sizeof(server_addr)); //清空服务端套接字(bzero将内存前n个字节置为0)
	server_addr.sin_family = AF_INET;     //地址采用IPv4地址，htonl和htons将地址和端口主机字节序转为网络字节序
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY相当于所有地址，inet_addr("0.0.0.0")
	server_addr.sin_port = htons(SERVER_PORT);
	bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)); //将地址和端口绑到监听描述符上，要先将sockaddr_in转为sockaddr
	listen(listen_fd,20); //将套接口从CLOSED状态转为LISTEN状态，后面的20表示最大同时连接的客户端
	printf("等待客户端连接中...\n");
	pthread_t id[MAX_CONN];
	int conn_id = 0;
	while(1) {
		socklen_t client_addr_len = (socklen_t)sizeof(client_addr);

		//accept函数返回新的连接描述字，client_addr被置为客户端套接字，后面的是套接字长度
		int conn_fd = accept(listen_fd,(struct sockaddr*)&client_addr,&client_addr_len); 
		
		//注意conn_fd前的void＊不能去掉，如果采用地址传递，线程执行时，conn_fd在while循环生命周期已经结束了
		int result = pthread_create(id+conn_id,NULL,(void *)handle_client_conn,(void *)(&conn_fd)); 
		if(result != 0) {
			printf("Unable to create thread,Error Code:%d",result);
			exit(1);
		}
		pthread_detach(*(id+conn_id));
		conn_id++;
	}
	close(listen_fd);
	return 0;
}