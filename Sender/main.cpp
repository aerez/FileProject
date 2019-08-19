#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

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
string fname;


int SendFile(int sock){
    cout<<fname<<endl;

    send(sock,fname.c_str(),fname.size()+1,0);
    //string path="/home/aerez/CLionProjects/FileProject/Sender/";
    //path.append(fname);
    FILE *fp= fopen(fname.c_str(),"rb");
    if(fp==NULL)
    {
        printf("error opening file");
        return 1;
    }

    while(1){
        char buff[256]={0};
        int nread= fread(buff,1,256,fp);

        if(nread>0){
            send(sock,buff,nread,0);

        }
        if(nread<256){
            if(feof(fp)){
                cout<<"end of file"<<endl;
                cout<<"File transfer complete!"<<endl;
            }
            if(ferror(fp))
                cout<<"Error reading"<<endl;

            break;
        }
    }

}

int main() {
    system("clear");
    int sockfd=0;


    struct addrinfo hints, *ai, *p;
    int rv, yes = 1;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;//ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM;// tcp socket

    if ((rv = getaddrinfo(LOOPBACK,PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "select server: %s\n", gai_strerror(rv));
        exit(1);
    }


    for (p = ai; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        //avoid "address already in use msg" error
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, " server: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai);



    if(listen(sockfd,10)==-1){
        perror("listen");
        exit(3);
    }
    cout << "listener is READY!" << endl;
    int newfd;
    addrlen= sizeof(remoteaddr);
    cout<<"waiting for connection"<<endl;
    if((newfd=accept(sockfd,(struct sockaddr*)&remoteaddr,&addrlen))<0){
        printf("accept error");
    }
    char remoteIP[INET6_ADDRSTRLEN];
    cout<<"server: new connection from "<<
        inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),
                  remoteIP,INET6_ADDRSTRLEN)<<"on socket "<<sockfd<<endl;


    cout<<"Enter file name to send: ";
    getline(cin,fname);

    SendFile(newfd);

    return 0;
}