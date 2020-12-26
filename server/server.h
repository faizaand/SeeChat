#ifndef SEECHAT_SERVER_H
#define SEECHAT_SERVER_H

struct ClientInfo {
public:
    int socketNumber;
    char ip[INET_ADDRSTRLEN];
};

struct ReceiveParams {
    void *server;
    void *socket;
};

using Client = struct ClientInfo;
using Address = struct sockaddr_in;

// SeeChat
namespace sc {
    class Server {
    public:
        void broadcastToAll(char *msg);

        void handleDisconnect(Client client);

        void *receiveMessage(void *socket);

        static void *receiveMessageHelper(void *context) {
            auto *params = static_cast<ReceiveParams *>(context);
            return ((Server *) params->server)->receiveMessage(params->socket);
        }

        void init();

    private:
        int clients[100]{};
        int n = 0;
    };
}

#endif
