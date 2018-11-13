#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>

#define SVR_PORT                        80
#define BUF_SIZE                        1024
char messenge[]="HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html; chaeset=UTF-8\r\n"
				"\r\n"
				"<!DOCTYPE html>\r\n"
				"<html><head><title>405410102 Homework_1</title>\r\n"
				"</head>\r\n"
				"<body><center>Hey my name is Tsung Chuang-Tzu ,my student ID is 405410102.\r\n"
				"<img src=\"MyICPC.jpg\"\r\n"
				"</body></html>\r\n";
			
int main (int argc, char **argv){
	

	fd_set ob_fd_set;
	int sock_fd;
	int max_fd;
	int	img_fd;
	int flag = 1;
	char buff[BUF_SIZE];

	// create a socket
	if((sock_fd=socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("Fail to create socket.\n");
		return -1;
	}
	else{
		printf("sock_fd={%d}\n",sock_fd);
	}

	//Set socket option 
	if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int))<0){
		perror("Fail to set sockoption.\n");
		return -1;
	}

	// Bind
	struct sockaddr_in server_addr; //server info
	socklen_t len;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(SVR_PORT);
	len = sizeof(struct sockaddr_in);


	if(bind(sock_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0) {
		perror("Fail to bind.\n");
		return -1;
	}
	else{
		printf("Bind {%s:%u} success.\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port));
	}

	// Listen 
	if(listen(sock_fd,128)==-1) {
		perror("Fail to listen.\n");
		return -1;
	}

	FD_ZERO(&ob_fd_set);
	FD_SET(sock_fd, &ob_fd_set);
	max_fd=sock_fd;

	while(1){
		int ret;
		struct timeval tv;
		fd_set read_fds;

		//Set the timeout 
		tv.tv_sec=3;
		tv.tv_usec=0;

		//Copy fd set to new set 
		read_fds=ob_fd_set;
		ret=select(max_fd+1,&read_fds,NULL,NULL,&tv);
		printf("==%d\n",ret);
		if(ret==-1){
			perror("Fail to select.\n");
			return -1;
		}
		else if(ret==0){
			printf("select timeout\n");
			continue;
		}
		else{
			int i;

			// service all
			for(i=0;i<FD_SETSIZE;i++){
				if(FD_ISSET(i,&read_fds)){
					if(i==sock_fd){
						//Connection request
						struct sockaddr_in client_addr;
						int new_fd;

						//accept
						new_fd=accept(sock_fd,(struct sockaddr*)&client_addr,&len);
						if(new_fd==-1){
							perror("Fail to accept.\n");
							return -1;
						}
						else{
							memset(buff,0,1024);
							read(new_fd,buff,1023);

							printf("Accpet client come from {%s:%u} by fd {%d}\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),new_fd);

							if(!strncmp(buff,"GET /MyICPC.jpg",15)){
								img_fd=open("MyICPC.jpg",O_RDONLY);
								sendfile(new_fd,img_fd,NULL,160000);
								close(img_fd);
							}
							else{
								write(new_fd,messenge,sizeof(messenge)-1);
							}
							//Add to fd set
							FD_SET(new_fd,&ob_fd_set);
							if(new_fd>max_fd)
								max_fd=new_fd;
						}
					}
					else{
						// Data already connect
						int recv_len;

						//
						memset(buff,0,sizeof(buff));
						recv_len=recv(i,buff,sizeof(buff),0);
						if (recv_len==-1) {
							perror("Fail to recv.\n");
							return -1;
						}
						else if(recv_len==0) {
							printf("Client disconnect\n");
						}
						else{
							printf("Receive: len={%d} msg={%s}\n",recv_len,buff);

							send(i,buff,recv_len,0);
						}

						//Close all socket
						close(i);
						FD_CLR(i,&ob_fd_set);
					}

				} 
			}
		} 
	} 

	return 0;
}
