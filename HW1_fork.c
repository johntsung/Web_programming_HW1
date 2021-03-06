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

char messenge[]="HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html; chaeset=UTF-8\r\n"
				"\r\n"
				"<!DOCTYPE html>\r\n"
				"<html><head><title>405410102 Homework_1</title>\r\n"
				"</head>\r\n"
				"<body><center>Hey my name is Tsung Chuang-Tzu ,my student ID is 405410102.\r\n"
				"<img src=\"MyICPC.jpg\"\r\n"
				"</body></html>\r\n";
int main(int argc,char *argv[]){
	struct sockaddr_in server_addr,client_addr;
	socklen_t sin_len=sizeof(client_addr);
	int fd_server,fd_client;
	char buf[2048];
	int img_fd,pid;
	int on=1;

	fd_server=socket(AF_INET,SOCK_STREAM,0);
	if(fd_server<0){
		perror("socket");
		exit(1);
	}

	setsockopt(fd_server,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int));

	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	server_addr.sin_port=htons(80);

	if(bind(fd_server,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1){
		perror("bind");
		close(fd_server);
		exit(1);
	}
	if(listen(fd_server,10)==-1) {
		perror("listen");
		close(fd_server);
		exit(1);
	}
	while(1){
		fd_client=accept(fd_server,(struct sockaddr*)&client_addr,&sin_len);

		if(fd_client==-1){
			perror("Connection failed....\n");
			continue;
		}

		printf("Got client connection.......\n");
		pid=fork();  //get pid
		if(!pid){
			//child process 
			close(fd_server);
			memset(buf,0,2048);
			read(fd_client,buf,2047);

			printf("%s\n", buf);

			if(!strncmp(buf,"GET /MyICPC.jpg",15)){
				img_fd=open("MyICPC.jpg",O_RDONLY);
				sendfile(fd_client,img_fd,NULL,160000);
				close(img_fd);
			}
			else write(fd_client,messenge,sizeof(messenge)-1);
			close(fd_client);
			printf("closing...\n");
			exit(0);
		}
		//parent process
		if(pid){
			wait(NULL); //wait 
			close(fd_client);
		}
	}
	return 0;
}
