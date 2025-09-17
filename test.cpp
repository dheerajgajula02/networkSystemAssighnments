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
#include<cerrno>
#include<vector>
// to prevent zombie process 
#include<signal.h>
#include<typeinfo>
#include<sstream>
#include<iostream>
// NEW: Add header for socket options timeout
#include<sys/time.h> 

using namespace std;

const string subdir = "www";

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


int main(int argc, char* argv[])  {
    if(argc < 2) {
        cout << "Usage :" << argv[0] << "<port>" << endl;
        return 1;
    }
    char *endptr = nullptr;
    long port_l = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || port_l <= 0 || port_l > 65535) {
        cerr << "Invalid port: " << argv[1] << endl;
        return 1;
    }
    uint16_t port = static_cast<uint16_t>(port_l);
    int sockfd;
    struct sockaddr_in serverAddr;
    signal(SIGCHLD, SIG_IGN);
    
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (sockfd < 0) {
        perror("Socket creation failed !!");
        exit(1);
    }
    cout << "Server socket created successfully" << endl;
    
    // NEW: Allow the port to be reused immediately after the server stops.
    // This prevents the "Address already in use" error during testing.
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    cout << "[+] Bind to port number " << port << endl;
    
    listen(sockfd, 10);
    cout << "[+] Listening ..." << endl;
    
    while (true) {
        struct sockaddr_in newAddr; 
        socklen_t addr_size = sizeof(newAddr);
        int newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size); 
        if (newSocket < 0) {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            // child process 
            close(sockfd);

            // NEW: Set a 10-second timeout on the client socket. If no new data
            // arrives in 10 seconds, recv() will stop waiting.
            struct timeval tv;
            tv.tv_sec = 10;
            tv.tv_usec = 0;
            setsockopt(newSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            
            // NEW: This loop will handle multiple requests on the same connection.
            while (true) {
                char buffer[4096] = {0}; 
                int bytes_recieved = recv(newSocket, buffer, sizeof(buffer)-1, 0);

                // NEW: If recv returns 0 or -1, the connection is over.
                // This happens on timeout, if the client disconnects, or on an error.
                if (bytes_recieved <= 0) {
                    cout << "[INFO] Client disconnected or connection timed out." << endl;
                    break; // Exit the loop to close the socket.
                }
                
                buffer[bytes_recieved] = '\0';
                cout << "--- Request Received ---\n" << buffer << "------------------------\n";

                // NEW: Check if the client sent "Connection: Keep-alive".
                bool keep_alive_requested = false;
                // Use strstr to search for the header. Check for lower and upper case 'K'.
                if (strstr(buffer, "Connection: Keep-alive") || strstr(buffer, "Connection: keep-alive")) {
                    keep_alive_requested = true;
                }
                
                char method[16], path[256], version[16];
                string status_code;
                
                sscanf(buffer, "%s %s %s", method, path, version);
                cout << "Method : " << method << " || Path :" <<  path << " ||Version :" << version << endl;
                string body;
                string filepath;

                if (strcmp(path, "/") == 0) {
                    filepath = subdir + "/index.html";
                } else {
                    filepath = subdir + path;
                }
                
                cout << "Filepath: " << filepath << endl;
                
                string version_string(version);
                string method_string(method);

                if(version_string != "HTTP/1.1") {
                    string error_body = "505 HTTP Version Not Supported";
                    string response = "HTTP/1.1 505 HTTP Version Not Supported\r\n";
                    response += "Content-Length: " + to_string(error_body.size()) + "\r\n";
                    // NEW: Add the correct Connection header to our response.
                    response += keep_alive_requested ? "Connection: Keep-alive\r\n" : "Connection: Close\r\n";
                    response += "\r\n";
                    response += error_body;
                    send(newSocket, response.c_str(), response.size(), 0);
                } else if (method_string != "GET"){
                    string error_body = "405 Method Not Allowed";
                    string response = "HTTP/1.1 405 Method Not Allowed\r\n";
                    response += "Content-Length: " + to_string(error_body.size()) + "\r\n";
                    // NEW: Add the correct Connection header to our response.
                    response += keep_alive_requested ? "Connection: Keep-alive\r\n" : "Connection: Close\r\n";
                    response += "\r\n";
                    response += error_body;
                    send(newSocket, response.c_str(), response.size(), 0);
                } else if (isBinaryFile(filepath)) {
                    FILE* f = fopen(filepath.c_str(), "rb");
                    if (!f) {
                        string error_body = "404 Not Found";
                        string response = "HTTP/1.1 404 Not Found\r\n";
                        response += "Content-Length: " + to_string(error_body.size()) + "\r\n";
                        // NEW: Add the correct Connection header to our response.
                        response += keep_alive_requested ? "Connection: Keep-alive\r\n" : "Connection: Close\r\n";
                        response += "\r\n";
                        response += error_body;
                        send(newSocket, response.c_str(), response.size(), 0);
                    } else {
                        fseek(f, 0, SEEK_END);
                        long fsize = ftell(f);
                        rewind(f);

                        // NEW: Use new[] to allocate a buffer for the binary file.
                        char* file_buffer = new char[fsize];
                        fread(file_buffer, 1, fsize, f);
                        fclose(f);

                        string header = "HTTP/1.1 200 OK\r\n";
                        header += "Content-Type: " + get_content_type(filepath) + "\r\n";
                        header += "Content-Length: " + to_string(fsize) + "\r\n";
                        // NEW: Add the correct Connection header to our response.
                        header += keep_alive_requested ? "Connection: Keep-alive\r\n" : "Connection: Close\r\n";
                        header += "\r\n";

                        send(newSocket, header.c_str(), header.size(), 0);
                        send(newSocket, file_buffer, fsize, 0);

                        // NEW: Free the memory allocated with new[].
                        delete[] file_buffer;
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
                    // NEW: Add the correct Connection header to our response.
                    response += keep_alive_requested ? "Connection: Keep-alive\r\n" : "Connection: Close\r\n";
                    response += "\r\n";
                    response += body;
                    send(newSocket, response.c_str(), response.size(), 0);
                }

                // NEW: If the client didn't ask to keep the connection alive, we break
                // the loop and the connection will be closed.
                if (!keep_alive_requested) {
                    break;
                }
            }
            
            cout << "[+] Closing connection." << endl;
            close(newSocket);
            exit(0);
        } else {
            // parent process 
            close(newSocket);
        }
    }
    return 0;
}