//#include "FL/Fl.H"
//#include "FL/Fl_Box.H"
//#include "FL/Fl_Window.H"
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void *receiveMessage(void *socket) {
    int theirSocket = *((int *) socket);
    char message[500];
    int len;
    while ((len = recv(theirSocket, message, 500, 0)) > 0) {
        message[len] = '\0';
        fputs(message, stdout);
        memset(message, '\0', sizeof message); // clear
    }
}

int main() {
//    Fl_Window window(400, 600, "SeeChat Client");
//    Fl_Box box(0, 50, 200, 20, "Hello, SeeChat!");
//    window.show();
//    return Fl::run();

    struct sockaddr_in theirAddress;
    int mySocket, theirSocket, theirAddressSize, port;
    pthread_t sendThread, receiveThread;
    char message[500];
    char username[100];
    char res[600];
    char ip[INET_ADDRSTRLEN];
    int len;

    std::cout << "Welcome to SeeChat Client!" << std::endl;
    std::cout << "Enter your port number: ";
    std::cin >> port;
    std::cout << std::endl << "Enter your username: ";
    std::cin >> username;
    std::cout << std::endl << "Cool! Connecting you to 127.0.0.1:" << port << " as " << username << "..." << std::endl;

    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(theirAddress.sin_zero, '\0', sizeof theirAddress.sin_zero);
    theirAddress.sin_family = AF_INET;
    theirAddress.sin_port = htons(port);
    theirAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(mySocket, (struct sockaddr *) &theirAddress, sizeof theirAddress) < 0) {
        perror("Failed to establish connection");
        exit(1);
    }

    inet_ntop(AF_INET, (struct sockaddr *) &theirAddress, ip, INET_ADDRSTRLEN);
    std::cout << "You're connected! Chat away.\n" << std::endl;
    pthread_create(&receiveThread, nullptr, receiveMessage, &mySocket);
    while (fgets(message, 500, stdin) != nullptr) {
        strcpy(res, username);
        strcat(res, ": ");
        strcat(res, message);
        len = write(mySocket, res, strlen(res));
        if (len < 0) {
            perror("Failed to send message.");
            continue;
        }
        memset(message, '\0', sizeof message);
        memset(res, '\0', sizeof res);
    }
    pthread_join(receiveThread, nullptr);
    close(mySocket);
    return 0;
}
