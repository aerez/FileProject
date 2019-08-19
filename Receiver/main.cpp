#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <chrono>

#define LOOPBACK "127.0.0.1"
#define PORT "55555"

using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family ==AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void gotoxy(int x,int y){//moves cursor to column x and row y
    printf("%c[%d;%df",0x1B,y,x);
}

struct sockaddr_storage remoteaddr;//client address;
socklen_t addrlen;
char fname[256];

int main() {
    system("clear");
    int sockfd=0, bytesReceived=0;
    char recvBuff[256];
    memset(recvBuff,'0',sizeof(recvBuff));

    int rv;
    char s[INET6_ADDRSTRLEN];
    struct addrinfo hints, *servinfo,*p;

    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;

    if((rv=getaddrinfo(LOOPBACK,PORT,&hints,&servinfo))!=0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p=servinfo;p!=NULL;p=p->ai_next){
        if((sockfd =socket(p->ai_family,p->ai_socktype, p->ai_protocol))==-1){
            perror("client: socket");
            continue;
        }
        if(connect(sockfd, p->ai_addr, p->ai_addrlen)==-1){
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if(p==NULL){
        fprintf(stderr,"client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s,sizeof s);
    cout<< "client: connected to "<<s<<endl;

    freeaddrinfo(servinfo);

    FILE *fp;
    char fname[256];

    int namebytes=recv(sockfd,fname,256,0);
    cout<<namebytes<<endl;
    cout<<"File name: "<<string(fname,0,namebytes+1)<<endl;
    cout<<"Recieving file.."<<endl;

    fp=fopen(fname,"ab");
    if(fp==NULL){
        printf("error opening file");
        return 1;
    }
    long double sz=0;
    //receive data in 256 bytes pieces
    auto begin = chrono::high_resolution_clock::now();
    while((bytesReceived= recv(sockfd,recvBuff,256,0))>0){
        sz+=bytesReceived;
        gotoxy(0,4);
        printf("Received: %llf Kb\n",(sz/1000));
        fflush(stdout);
        fwrite(recvBuff,1,bytesReceived,fp);

    }
    if(bytesReceived<0){
        printf("\n error reading \n");
    }
    auto end=chrono::high_resolution_clock::now();
    auto dur=end-begin;
    auto ms= std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

    cout<< "Completed in "<<(ms)<<" milliseconds"<<endl;
    return 0;

}