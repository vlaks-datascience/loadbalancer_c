#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <ws2tcpip.h>
#include <iostream>
#include <pthread.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 5000
#define DEFAULT_PORT_REC "5001"

bool InitializeWindowsSockets();
void* RegisterToLoadBalancer(void* arguments);

int main()
{
    pthread_t RegisterThread;
    pthread_create(&RegisterThread, NULL, &RegisterToLoadBalancer, NULL);
    (void)pthread_join(RegisterThread, NULL);
    return 0;
}

bool InitializeWindowsSockets()
{
    WSADATA wsaData;
    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

void* RegisterToLoadBalancer(void* arguments) {
    // socket used to communicate with server
    SOCKET connectSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // message to send
    char m[] = "conn";
    char* messageToSend = m;
    char test[] = "";
    char* testingChar = test;
    int sum = 0;



    if (InitializeWindowsSockets() == false) { }

    // create a socket
    connectSocket = socket(AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
    }

    // create and initialize address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(DEFAULT_PORT);

    if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("Unable to connect to server.\n");
        closesocket(connectSocket);
        WSACleanup();
    }

    iResult = send(connectSocket, messageToSend, DEFAULT_BUFLEN, 0);

    char* memory[1024];
    int count = 0;
    char recvbuf[DEFAULT_BUFLEN];
    //int iRes;
    printf("Waiting for numbers!\n");
    do
    {
        iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
        memory[count] = recvbuf + '\0';

        if (iResult > 0)
        {
            int x = atoi(recvbuf);
            sum += x;
            printf("====================================\n");
            printf("Number received from LB: %d.\n", x);
            printf("Current sum is: %d.\n", sum);
            printf("====================================\n");
            count++;
            char mtos[] = "Worker (SOCKET *%d*) finished action!", connectSocket;
            testingChar = mtos;
        }
        else if (iResult == 0)
        {
            // connection was closed gracefully
            printf("Connection with client closed.\n");
        }
        else
        {
            // there was an error during recv
            printf("recv failed with error: %d\n", WSAGetLastError());
        }
        Sleep(5000);
        //iRes = send(connectSocket, testingChar, DEFAULT_BUFLEN, 0);
    } while (true);

    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        WSACleanup();
    }

    closesocket(connectSocket);
    WSACleanup();
    return NULL;
}