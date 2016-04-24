/*
* 建立与指定服务器的连接，返回套接字
* author zhao
*/

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

int socket_connect(const char* ip,int port){
     int sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
     if(sock<0){
       printf("sock init failed\n");
        return sock;
     }
     struct sockaddr_in seraddr;
     struct hostent *hp;
     memset(&seraddr,0,sizeof(seraddr));

     // if(inet_pton(AF_INET,ip,&seraddr.sin_addr)<=0){  //新的ip字符串转换函数，取代inet_addr()
     // 	 printf("inet_pton error\n");
     // 	// return -1;
     //     hp=gethostbyname(ip);
     //     if(hp==NULL)
     //      	 return -1;
     //     char str[32];
     //     printf("%s",inet_ntop(hp->h_addrtype, hp->h_addr, str, sizeof(str)));
     //     memcpy(&seraddr.sin_addr, hp->h_addr, hp->h_length);
     // }

     hp = gethostbyname(ip);
     if (hp == NULL)
            return -1;
     memcpy(&seraddr.sin_addr, hp->h_addr, hp->h_length);

     seraddr.sin_family=AF_INET;
     seraddr.sin_port  =htons(port);

     if(connect(sock,(struct sockaddr *)&seraddr,sizeof(seraddr))<0)
     	  return -1;

     /*
     char buf[255];
     write(sock,buf,sizeof(buf));
     while((n=read(sock,buf,sizeof(buf)))>0){
        buf[n]=0;
        if(fputs(buf,stdout)==EOF)
        	printf("error");
     }
     */

     return sock;
}
