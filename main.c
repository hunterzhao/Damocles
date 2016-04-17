/*
*  主函数
*/
#include "sock.c"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define METHOD_GET     0
#define METHOD_POST    1
#define METHOD_OPTIONS 2
#define METHOD_TRACE   3
#define PROGRAM_VERSION "0.1.0"
#define REQUEST_SIZE   2048
int method=0; //存储方法
int force=0;  //等待响应
int force_reload=0; //重启标志
int msgPipe[2]; //管道
char request[REQUEST_SIZE]; //请求
int http10=0; //http版本设置
int damotime; //模拟请求时间
int clients=0;//客户端数量
char host[1024];

void make_request(const char *url);

static const struct option longopts[]=  //全局静态变量在声明他的文件之外是不可见的
{
     {"force",no_argument,&force,1},
     {"reload",no_argument,&force_reload,1},
     {"time",required_argument,NULL,'t'},
     {"help",no_argument,NULL,'?'},
     {"http09",no_argument,NULL,'9'},
     {"http10",no_argument,NULL,'1'},
     {"http11",no_argument,NULL,'2'},
     {"get",no_argument,&method,METHOD_GET},
     {"post",no_argument,&method,METHOD_POST},
     {"options",no_argument,&method,METHOD_OPTIONS},
     {"trace",no_argument,&method,METHOD_TRACE},
     {"version",no_argument,NULL,'V'},
     {"proxy",required_argument,NULL,'p'},
     {"clients",required_argument,NULL,'c'},
     {"NULL",0,NULL,0}
};

static void usage() //指对函数的作用域仅局限于本文件
{
	fprintf(stderr,
	"damo [option]... URL\n"
	"  -f|--force               Don't wait for reply from server.\n"
	"  -r|--reload              Send reload request - Pragma: no-cache.\n"
	"  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"
	"  -p|--proxy <server:port> Use proxy server for request.\n"
	"  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"
	"  -9|--http09              Use HTTP/0.9 style requests.\n"
	"  -1|--http10              Use HTTP/1.0 protocol.\n"
	"  -2|--http11              Use HTTP/1.1 protocol.\n"
	"  --get                    Use GET request method.\n"
	"  --head                   Use HEAD request method.\n"
	"  --options                Use OPTIONS request method.\n"
	"  --trace                  Use TRACE request method.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"
	);
}
int main(int argc,char* argv[]){
     int options_index=0;
     int opt=0;

     if(argc ==1)
     {
     	usage();
     	return 2;
     }
     //                                                         当前参数的在longopt中的索引
     while((opt=getopt_long(argc,argv,"912Vfrt:p:c:?h",longopts,&options_index))!= EOF)
     {
     	switch(opt)
     	{
     		case 0  :break;
     		case 'f': force=1; printf("i choose f\n"); break;
     		case 'r': force_reload = 1;break;
            case '9': http10 = 0;break;
            case '1': http10 = 1;break;
            case '2': http10 = 2;break;
            case 'V':
                      printf(PROGRAM_VERSION"\n");
                      exit(0);
            case 't':
                      damotime = atoi(optarg);
                      break;
            case 'h':
            case '?':
                      usage();return 2;break;
            case 'c': clients =atoi(optarg);break;
     	}
     }
     
     if(optind == argc)
     {
     	fprintf(stderr,"missing URL\n");
     	usage();
     	return 2;
     }
     if(clients==0) clients=1;
     if(damotime==0) damotime=10;

     //Copyright
     fprintf(stderr,"Damocles - Easy tool for web server test"PROGRAM_VERSION"\n"
     	            "Copyright (c) Hunter Zhao Open Source Software.\n");
     make_request(argv[optind]);

     int sock=socket_connect(host,80);
     if(sock==-1) {
     	printf("bad sock\n");
     	return 0;
     }
     printf("%s\n",request);

     int rc=sizeof(request);
     if(rc!=write(sock,request,rc)){
     	close(sock);
     }
     else  printf("done\n");

     int r=0;
     int len=0;
     char buf[1024];
     memset(buf,0,sizeof(buf));
     while((r=read(sock,buf,1024))>0){
     	 buf[r]='\0';
     	 len+=r;
     }
     printf("%d",len);
	 close(sock);
     return 1;
}

void make_request(const char *url){
     //http 1.0 request
	char tmp[1024];
	switch(method){
		case METHOD_GET:  sprintf(request,"GET ",4); break;
		case METHOD_POST: sprintf(request,"POST ",5); break;
		case METHOD_TRACE:sprintf(request,"TRACE ",6); break;
		case METHOD_OPTIONS:sprintf(request,"OPTIONS ",8); break;
	}
    
    char *p=strchr(url,'/');

	strcat(request,p);
	strcat(request," HTTP/1.1");
    strcat(request,"\r\n");
    strcat(request,"Accept: text/*\r\n");
    strcat(request,"User-Agent: Damocles "PROGRAM_VERSION"\r\n");
    strcat(request,"Host: ");
    strncpy(tmp,url,p-url);
    tmp[p-url]='\0';
    strcat(request,tmp);
    strcat(host,tmp); //赋值给host
    strcat(request,"\r\n");
    strcat(request,"\r\n");
   // printf("%s",request);
}