#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include<iostream>
using namespace std;

#define port 8888

int main()  {
    
    int sockfd;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;
    
    char buffer[1024];

    // defining the socket type , SOCK_STREAM is TCP 
    sockfd = socket(AF_INET, SOCK_STREAM,0);

    cout << "Server socket created successfully" << endl;

    // blocking the memory for address 
    memset(&serverAddr, '\0', sizeof(serverAddr));

    // defining the sender server address type 
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    // binding the socket with the sender configuration 
    bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    cout << "[+] Bind to port number " << port << endl;
    
    // listening to the socket 
    listen(sockfd, 5);

    cout << "[+] Listening ..." << endl;

    // accepting the socket connection 
    newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);

    // sending the data to the buffer TCP server 
    strcpy(buffer, "Hello");
    send(newSocket, buffer, strlen(buffer), 0);

    cout << "[+] closing the connection " << endl;

    return 0;



    
}
