
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string>
#include<string_view>
// for fork() function 
#include<unistd.h>

#include<vector>
// to prevent zombie process 
#include<signal.h>

#include<iostream>
using namespace std;

#define port 8888

const string subdir = "static";

#include <string>
using std::string;

bool has_suffix(const string& s, const string& sfx) {
    if (sfx.size() > s.size()) return false;
    return s.compare(s.size() - sfx.size(), sfx.size(), sfx) == 0;
}

bool isBinaryFile(const string&s) {
    if (has_suffix(s, ".png") ||has_suffix(s, ".gif") ||has_suffix(s, ".jpg") || has_suffix(s, ".ico") || has_suffix(s, "jpeg") )
    return true;
    return false;
}

string get_content_type(const string& path) {
    if (has_suffix(path, ".html")) return "text/html";
    if (has_suffix(path, ".txt"))  return "text/plain";
    if (has_suffix(path, ".png"))  return "image/png";
    if (has_suffix(path, ".gif"))  return "image/gif";
    if (has_suffix(path, ".jpg") || has_suffix(path, ".jpeg")) return "image/jpeg";
    if (has_suffix(path, ".ico"))  return "image/x-icon";
    if (has_suffix(path, ".css"))  return "text/css";
    if (has_suffix(path, ".js"))   return "application/javascript";
    return "application/octet-stream";
}


int main()  {
    
    int sockfd;
    struct sockaddr_in serverAddr;

    // prevent zombie process 
    signal(SIGCHLD, SIG_IGN);
    

    // int newSocket;
    // struct sockaddr_in newAddr;

    socklen_t addr_size;
    
    char buffer[1024];

    // defining the socket type , SOCK_STREAM is TCP 
    sockfd = socket(AF_INET, SOCK_STREAM,0);

    if (sockfd < 0) {
        perror("Socket creation failed !!");
        exit(1);
    }

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
    // 5 is the backalog queue size 
    listen(sockfd, 5);

    cout << "[+] Listening ..." << endl;

    // accepting the socket connection 

    while (true) {

        struct sockaddr_in newAddr; 
        socklen_t addr_size = sizeof(newAddr);
    
        int newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size); // this is your client socket 

        if (newSocket < 0) {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();

    // sending the data to the buffer TCP server 
    // strcpy(buffer, "Hello");
    // send(newSocket, buffer, strlen(buffer), 0);

    // recieving from the client 

    if (pid == 0) {

        // child process 

        close(sockfd);

        char buffer[1024];

        int bytes_recieved = recv(newSocket, buffer, sizeof(buffer)-1, 0);

        if (bytes_recieved > 0) {
        // creating the endline for the buffer recieved 
        buffer[bytes_recieved] = '\0';
        cout << "Data recieved from client : " << buffer << endl;

        char method[8], path[256], version[16];
        string status_code;

        
        sscanf(buffer, "%s %s %s", method, path, version);
        cout << "Method : " << method << " || Path :" <<  path << " ||Version :" << version << endl;
        string body;
        string filepath;

            // the end point is index
            
            filepath = subdir + path;

            cout << filepath << endl ;

        // handling binary files :) yes I wrote bad code, but for now lets just keep it this way 
        
        if (isBinaryFile(filepath)) {
            FILE* f = fopen(filepath.c_str(), "rb"); // binary mode :)

            if (!f) {
                // where the hell is file ????
                string error_body = "404 Not Found";
                string response = "HTTP/1.1 404 Not Found\r\n";
                response += "Content-Length: " + to_string(error_body.size()) + "\r\n";
                response += "\r\n";
                response += error_body;
                send(newSocket, response.c_str(), response.size(), 0);
            } else {
                fseek(f, 0, SEEK_END);
                long fsize = ftell(f);
                rewind(f);

                vector<char> body(fsize);
                fread(body.data(), 1, fsize, f);
                fclose(f);

                string content_type = get_content_type(filepath);
                string header = "HTTP/1.1 200 OK\r\n";
                header += "Content-Type: " + content_type + "\r\n";
                header += "Content-Length: " + to_string(fsize) + "\r\n";
                header += "\r\n";

                send(newSocket, header.c_str(), header.size(), 0);
                send(newSocket, body.data(), body.size(), 0);
            }

        } else { // normal files ( html / css/ js/ text )
            FILE* f = fopen(filepath.c_str(), "r");
            if (f) {
                char c;
                while ((c=fgetc(f)) != EOF)
                    body += c;
                fclose(f);
                status_code = "200 OK";
            } else {
                status_code = "404 Not Found";
                body = "<html><body>404 Not Found</body></html>";
            }
        string response = "HTTP/1.1 "+ status_code + "\r\n";
        response += "Content-Type: " + get_content_type(filepath) +"\r\n";
        response += "Content-Length: " + to_string(body.size()) + "\r\n";
        response += "\r\n";
        response += body;

        send(newSocket, response.c_str(), response.size(), 0);

        }
    }

    close(newSocket);
    exit(0);
    } else {
        // parent process 
        close(newSocket);
    }
    
    }
    cout << "[+] closing the connection " << endl;
    return 0;
}
