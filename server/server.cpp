#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include "server.h"

std::mutex m;

namespace sc {
    void Server::broadcastToAll(char *msg) {
        m.lock();
        for (int i = 0; i < n; i++) {
            if (send(clients[i], msg, strlen(msg), 0) < 0) {
                perror("Failure to send to a client");
                continue;
            }
        }
        m.unlock();
    }

    void Server::handleDisconnect(Client client) {
        m.lock();
        printf("%s disconnected\n", client.ip);
        // shift clients down one to keep array contiguous
        for (int i = 0; i < n; i++) {
            if (clients[i] == client.socketNumber) {
                int j = i;
                while (j < n - 1) {
                    clients[j] = clients[j + 1];
                    j++;
                }
            }
        }
        n--;
        m.unlock();
    }

    void *Server::receiveMessage(void *socket) {
        Client client = *((Client *) socket);
        char message[500];
        int len;
        while ((len = recv(client.socketNumber, message, 500, 0)) > 0) {
            message[len] = '\0';
            broadcastToAll(message);
        }
        handleDisconnect(client);
        return nullptr;
    }

    void Server::init() {
        Address myAddress, theirAddress;
        int mySocket, theirSocket;
        socklen_t theirAddressSize;
        int port;
        pthread_t receiveThread;
        Client client;
        char ip[INET_ADDRSTRLEN];

        std::cout << "Welcome to SeeChat Server v0.0.1" << std::endl << std::endl;
        std::cout << "Please enter your port number: ";
        std::cin >> port;

        mySocket = socket(AF_INET, SOCK_STREAM, 0);
        memset(myAddress.sin_zero, '\0', sizeof myAddress.sin_zero);
        myAddress.sin_family = AF_INET;
        myAddress.sin_port = htons(port);
        myAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        theirAddressSize = sizeof theirAddress;

        if (bind(mySocket, (struct sockaddr *) &myAddress, sizeof myAddress) != 0) {
            perror("Unsuccessful bind");
            exit(1);
        }

        if (listen(mySocket, 5) != 0) {
            perror("Unsuccessful listen");
            exit(1);
        }

        std::cout << "Great! Listening for connections on 127.0.0.1:" << port << "..." << std::endl;

        while (true) {
            if ((theirSocket = accept(mySocket, (struct sockaddr *) &theirAddress, &theirAddressSize)) < 0) {
                perror("Accept unsuccessful");
                exit(1);
            }
            m.lock();
            inet_ntop(AF_INET, (struct sockaddr *) &theirAddress, ip, INET_ADDRSTRLEN);
            printf("%s connected\n", ip);
            client.socketNumber = theirSocket;
            strcpy(client.ip, ip);
            clients[n] = theirSocket;
            n++;

            auto params = new ReceiveParams;
            params->server = this;
            params->socket = &client;
            pthread_create(&receiveThread, nullptr, Server::receiveMessageHelper, params);
            m.unlock();
        }
    }
}

int main() {
    sc::Server srv;
    srv.init();
    return 0;
}
