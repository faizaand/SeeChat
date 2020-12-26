#include "FL/Fl.H"
#include "FL/Fl_Browser.H"
#include "FL/Fl_Input.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Window.H"
#include "FL/fl_ask.H"
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std::chrono_literals;

Fl_Browser *browser;
int mySocket;
std::string nextMessage;
bool running = true;

struct ClickData {
    Fl_Input* input;
};

void *receiveMessage(void *socket) {
    int theirSocket = *((int *) socket);
    char message[500];
    int len;
    while ((len = recv(theirSocket, message, 500, 0)) > 0) {
        std::cout << running << std::endl;
        message[len] = '\0';
        Fl::lock();
        browser->add(message);
        Fl::unlock();
        Fl::awake();
        std::cout << message;
        memset(message, '\0', sizeof message); // clear
    }
}

void doNetworking(int port, char* username) {
    struct sockaddr_in theirAddress{};
    char res[600];
    char ip[INET_ADDRSTRLEN];
    int len;

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

    Fl::lock();
    browser->add("You're connected! Chat away.");
    browser->redraw();
    Fl::unlock();
    Fl::awake();

    std::cout << std::this_thread::get_id() << std::endl;

    std::thread receiverThread(receiveMessage, &mySocket);
    while (running) {
        std::this_thread::sleep_for(100ms);
        if(nextMessage.empty()) continue;
        strcpy(res, username);
        strcat(res, ": ");
        strcat(res, (nextMessage + "\n").c_str());
        len = write(mySocket, res, strlen(res));
        if (len < 0) {
            perror("Failed to send message.");
            continue;
        }
        nextMessage = "";
        memset(res, '\0', sizeof res);
    }
    close(mySocket);
}

void sendCallback(Fl_Widget *w, void *data) {
    nextMessage = ((ClickData*) data)->input->value();
    ((ClickData*) data)->input->value("");
}

int main() {
    Fl_Window window(400, 600, "SeeChat Client");
    browser = new Fl_Browser(10, 10, 380, 555);
    Fl_Input input(10, 570, 330, 25);
    Fl_Button send(340, 570, 50, 25, "Send");
    window.end();
    Fl::lock();

    auto* data = new ClickData;
    data->input = &input;
    input.when(FL_WHEN_ENTER_KEY_ALWAYS);
    input.callback(sendCallback, data);
    send.callback(sendCallback, data);

    window.show();

    int port;
    char username[100];
    const char* in = fl_input("Enter your desired port number:");
    port = atoi(in);
    in = fl_input("Enter your username:");
    strcpy(username, in);

    std::cout << port << std::endl;

    std::thread networkThread(doNetworking, port, username);
    int result = Fl::run();
    running = false;
    networkThread.join();
    return result;
}
