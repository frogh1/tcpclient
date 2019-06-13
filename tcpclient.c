#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <signal.h>

#define SOCKET_CLOSE(isockfd)  \
{\
    if(-1 != isockfd) \
    {\
        close(isockfd);\
        isockfd = -1;\
    }\
}
typedef struct _tagNET_CLIENT_SOCKET
{
    unsigned int uiConnState;   /* 连接状态*/
    int isockfd;
    unsigned int remoteip;
    unsigned short remoteport;
    //void *private;
}NET_CLIENT_SOCKET;

enum CONN_STATE
{
    CONN_STATE_INIT = 0,
    CONN_STATE_SUCCESS
};

NET_CLIENT_SOCKET g_CliSock;

int tcp_client_create_socket(NET_CLIENT_SOCKET *sockcfg)
{
    /* 套接字描述符 */
    int sockfd;
    int iret = 0;
    unsigned int addr;
    
    /* 连接者的主机信息 */
    struct sockaddr_in their_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd == -1)
    {
        /* 如果socket()调用出现错误则显示错误信息并退出 */
        printf("socket error\n");
        return -1;
    }
    
    struct timeval tv_out;
    tv_out.tv_sec = 1;
    tv_out.tv_usec = 0;
    
    
  //  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv_out, sizeof(tv_out));//设置连接超时
    
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv_out, sizeof(tv_out));//设置连接超时
    
    printf("%s\n","create socket success" );
    /* 主机字节顺序 */
    their_addr.sin_family = AF_INET;
    /* 网络字节顺序，短整型 */
    their_addr.sin_port = htons(sockcfg->remoteport);
    //their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    
    addr = sockcfg->remoteip;
    memcpy(&their_addr.sin_addr,&addr, sizeof(addr));
    //inet_aton(g_szServerIP, &their_addr.sin_addr);
    /* 将结构剩下的部分清零*/
    memset(their_addr.sin_zero,0,8);
    //bzero(&(their_addr.sin_zero), 8);
    printf("%s\n","init socket success" );
    iret = connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr));
    
    if(iret == -1)
    {
        /* 如果connect()建立连接错误，则显示出错误信息，退出 */
        // Net_Cfg->pfnPrintf("connect to %s port %u error\n",inet_ntoa(their_addr.sin_addr),Net_Cfg->usPort);
        SOCKET_CLOSE(sockfd);
        return -1;
    }
    printf("connect socket success\n");    
    sockcfg->isockfd = sockfd;
    
    return 0;
}

int tcp_client_init(unsigned int ip,unsigned short port,unsigned int encode)
{
    NET_CLIENT_SOCKET *pstSock = &g_CliSock;
    
    signal(SIGPIPE,SIG_IGN);
    
    memset(pstSock,0,sizeof(NET_CLIENT_SOCKET));
    
    pstSock->remoteip = ip;
    pstSock->remoteport = port;
    if (tcp_client_create_socket(pstSock) < 0)
    {
        return -1;
    }
    pstSock->uiConnState = CONN_STATE_SUCCESS;

    
    return 0;
}

int tcp_client_sent_recv_buff(char *sbuf,int slen,char *rbuf,int *rlen)
{
    int iret =0;
    int recv_len = 0;
    NET_CLIENT_SOCKET *pstSock = &g_CliSock;
    
    if (CONN_STATE_INIT ==  pstSock->uiConnState)
    {
        if (tcp_client_create_socket(pstSock) < 0)
        {
            return -1;
        }
        pstSock->uiConnState = CONN_STATE_SUCCESS;
    }
    
    iret = send(pstSock->isockfd, (const char *)sbuf, slen, 0);
    printf("send iret %d\n", iret);
    if (iret < 0)
    {
        SOCKET_CLOSE(pstSock->isockfd);
        pstSock->uiConnState = CONN_STATE_INIT;
        
        return -1;
    }
#if 0
    else if (iret >= 0 && NULL != rbuf)
    {
        recv_len = recv(pstSock->isockfd, rbuf, rlen, 0);
        if (recv_len <= 0)
        {
        }
        else
        {
            rlen = recv_len;
        }
    }
#endif
    return 0;
}

int tcp_client_close()
{
     NET_CLIENT_SOCKET *pstSock = &g_CliSock;
    SOCKET_CLOSE(pstSock->isockfd);
    return 0;
}


int main(int argc,char *argv[])
{
    int iret =0;
    char *sbuf = "hello\n";
    unsigned int uiip = inet_addr("127.0.0.1");
    unsigned short port = 9999;
    iret = tcp_client_init(uiip,port,0);
    printf("iret = %d\n",iret);
    while(1)
    {
        tcp_client_sent_recv_buff(sbuf,strlen(sbuf),NULL,NULL);
        sleep(1);
    }
    tcp_client_close();
    return 0;
}
