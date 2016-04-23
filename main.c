/*
*  主函数
*/
#include "sock.c"
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define METHOD_GET     0
#define METHOD_POST    1
#define METHOD_OPTIONS 2
#define METHOD_TRACE   3
#define PROGRAM_VERSION "0.1.0"
#define REQUEST_SIZE   2048

int success=0;
int failed=0;
int bytes=0;

int method=0; //存储方法
int force=0;  //等待响应
int force_reload=0; //重启标志
int msgPipe[2]; //管道
char request[REQUEST_SIZE]; //请求
int http10=0; //http版本设置
int clients=0;//客户端数量
char host[1024];
int port=0;
int benchtime=0; //测试的时间长度
int timeout=0; //时间是否过期

void make_request(const char *url);
size_t readn(int sock,void *vptr,size_t n);
static int startTest(void);
void test(const char *host,const int port,const char *request);
static void alarm_handler(int signal);

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
     {"port",required_argument,NULL,'p'},
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
	"  -p|--port <server:port>  set server port for request.\n"
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
                      benchtime = atoi(optarg);
                      break;
            case 'h':
            case '?':
                      usage();return 2;break;
            case 'c': clients =atoi(optarg);break;
            case 'p': port=atoi(optarg);break;
     	}
     }
     
     if(optind == argc)
     {
     	fprintf(stderr,"missing URL\n");
     	usage();
     	return 2;
     }
     if(clients==0) clients=1;
     if(benchtime==0) benchtime=3;

     //Copyright
     fprintf(stderr,"Damocles - Easy tool for web server test"PROGRAM_VERSION"\n"
     	            "Copyright (c) Hunter Zhao Open Source Software.\r\n");
     make_request(argv[optind]);
     
     startTest();
  //    int sock=socket_connect(host,port);
  //    if(sock==-1) {
  //    	printf("bad sock\n");
  //    	return 0;
  //    }
  //    printf("%s\n",request);

  //    int rc=sizeof(request);
  //    if(rc!=write(sock,request,rc)){
  //    	close(sock);
  //    }
  //    else  printf("done\n");

  //    size_t r=0;
  //    char buf[1024];
  //    memset(buf,0,sizeof(buf));
  //    r=readn(sock,buf,351);
  //    //r=recv(sock,buf,1024,0);
  //    printf("%s\n%d",buf,r);

	 // close(sock);
     return 1;
}

static int startTest(void){  //外部文件无法调用
     //socket 
     int i,j,k;
     pid_t pid=0;
     FILE *f;
     
     //返回套接字
     i=socket_connect(host,port);
     if(i==-1){
        fprintf(stderr,"\nconnect server faild,abort\n");
        return;
     }
     close(i);
     
     if(pipe(msgPipe))
     {
        perror("pipe failed.\n");
        return 3;
     }

     //生成子进程
     for(i=0;i<clients;i++){
         pid=fork();
         if(pid<=(pid_t)0){
            sleep(1); //让子进程更快？？？
            //如果是子进程，直接执行后面的代码，不能让子进程继续fork
            break;
         }
     }

     if(pid<(pid_t)0)
     {
        fprintf(stderr,"error occur forking.%d\n",i);
        return 3;
     }
     
     if(pid==(pid_t)0)
     {
        success=0;failed=0;bytes=0;
        test(host,port,request);
        //printf("%d %d %d\n",success,failed,bytes);
        f=fdopen(msgPipe[1],"w"); //用于由创建管道和网络通信通道函数获得的描述符,该类特殊文件不能使用fopen
        if(f==NULL)
        {
            perror("open pip failed.\n");
            return 3;
        }

        fprintf(f,"%d %d %d\n",success,failed,bytes);
        fclose(f);

        return 0;
     }else{
        f=fdopen(msgPipe[0],"r");
        if(f==NULL)
        {
            perror("open pip failed.\n");
            return 3;
        }

        setvbuf(f,NULL,_IONBF,0);//直接从流中读入数据或直接向流中写入数据，而没有缓冲区。
        success=0; //成功请求数
        failed=0; //失败请求数
        bytes=0;  //传输字节数
        int su=0,fa=0,by=0;
        while(1)
        {
            int rc=fscanf(f,"%d %d %d",&su,&fa,&by);//返回读入参数个数，失败返回-1
            printf("%d %d %d\n",su,fa,by);
            if(rc<2)
            {
                perror("some of children died.\n");
                break;
            }
            success+=su;
            failed+=fa;
            bytes+=by;
            if(--clients==0) break;
        }
        fclose(f);
        //绘制一个进度表***
        //printf("%d %d %d %d\n",success,failed,bytes,benchtime);
        printf("\nspeed=%d pages/min,%d bytes/sec.\nRequests: %d success,%d failed\n",
            (int)((success+failed)/(benchtime/60.0f)),(int)(bytes/(float)benchtime),success,failed);
     }
     return i;
}

static void alarm_handler(int signal){
    timeout=1;
}
void test(const char *host,const int port,const char *request){
     int rlen;
     char buf[1024];
     int s,i;
     struct sigaction sa;

     //安装信号
     sa.sa_handler=alarm_handler;
     sa.sa_flags=0;
     if(sigaction(SIGALRM,&sa,NULL)) //设置要捕捉的信号，信号来了如何执行什么操作
        exit(3);
    alarm(benchtime); //当定时器指定的时间到时，它向进程发送SIGALRM信号。可以设置忽略或者不捕获此信号，如果采用默认方式其动作是终止调用该alarm函数的进程

    rlen=strlen(request);

    while(1){
        //timeout=1 说明时间已经过期
        if (timeout)
        {
            printf("time is out\n");
            if(failed>0)
            {
                //是超时导致的failed 不算一次failed
                failed--;
            }
            return;
        }

        s=socket_connect(host,port);
        if(s<0)
        {
            failed++;
            continue;
        }

        if(rlen!=write(s,request,rlen))
        {
            failed++;
            close(s);
            continue;
        }
        
        if(force ==0) //force==0 读取服务器回复
        {
            i=readn(s,buf,sizeof(buf));
            //printf(buf);
            //printf("%d %d\n",strlen(buf),i);
            if(i<0)
            {
                failed++;
                // close(s);
            }
            else
                if(i==0) break;
                else bytes+=i;
        }

        if(close(s))
        {
            failed++;
            continue;
        }
        success++;
    }
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

size_t readn(int sock,void *vptr,size_t n){
    size_t nleft;
    size_t nread;
    char   *ptr;

    ptr=vptr;
    nleft=n;
    while(nleft>0){
          if((nread=recv(sock,ptr,nleft,0))<0 ){
             return -1;
          }
          if(nread==0)
             break;
          nleft -= nread;
          ptr   += nread;
    }
    return n-nleft;
}