#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include<iostream>
using namespace std;

int main(int argc, char **argv) {
    int port = 5501;
    int sockfd;

    // sockaddr_in is a part of sockaddr, its just for IPV4 sockets specifically
    struct sockaddr_in myaddr, remoteAddr;
    char buffer[1024];
    socklen_t addr_size;

    // telling what type of socket I'm creating in this case 
    // IPV4 -- AF_INET , UDP packet -- SOCK_DGRAM , 0 means the default port for the specific protocol
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // blocking memoery
    memset(&myaddr, '\0', sizeof(myaddr));

    // defining your address 
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(port);
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // binding your socket, 
    // socket type, your address and size of your address

    cout << "Socket binded" << endl;
    bind(sockfd, (struct sockaddr*)&myaddr, sizeof(myaddr));
    addr_size = sizeof(remoteAddr);

    // recieving from the socket 

    cout << "Listening on the socket " << endl;
    cout << "type quit to exit the session" << endl;

    while (true) {

    recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&remoteAddr, &addr_size);

    if (buffer == "quit\n") {
        cout << "Thanks for trying , bye bye " << endl;
        break;
    }
    cout << "data recieved from the buffer is " << buffer << endl;
    cout << "Remote IP address " << inet_ntoa(remoteAddr.sin_addr) << endl;
    cout << "Remote Port " << ntohs(remoteAddr.sin_port) << endl;
    cout << "Address family " << remoteAddr.sin_family << endl;

    }

    return 0;


}