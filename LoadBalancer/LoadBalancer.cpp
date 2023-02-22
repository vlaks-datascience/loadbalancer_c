#pragma comment(lib, "Ws2_32.lib")
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <pthread.h>
#include <ws2tcpip.h>
#include "CircularBuffer.h"
#include "Data.h"
#include "Sorting.h"

#define DEFAULT_BUFLEN 512
#define CBLEN 1024
#define DEFAULT_PORT "5059"
#define DEFAULT_PORT_WORKER "5000"
#define DEFAULT_PORT_REC 5001

bool InitializeWindowsSockets();
void* ListenClient(void* arguments);
void* ListeningForNewClients(void* arguments);
void* ListeningForNewWorker(void* arguments);
void* AcceptWorkers(void* arguments);
void* Send(void* arguments);
char recvbuf[DEFAULT_BUFLEN];

struct arg_struct {
    int iResult;
    SOCKET acceptSocket;
} *args;

int main()
{
    if (bufferCheck()) {
        printf("Buffer successfully initialized!\n");
    }

    // Accept clients and receive data
    pthread_t ClientThread;
    // Accept workers and put them in sorted list
    pthread_t WorkerThread;
    // Decides to which worker data is sent

    pthread_create(&ClientThread, NULL, &ListeningForNewClients, NULL);
    pthread_create(&WorkerThread, NULL, &ListeningForNewWorker, NULL);

    (void)pthread_join(ClientThread, NULL);
    (void)pthread_join(WorkerThread, NULL);

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

void* ListeningForNewWorker(void* arguments) {
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data

    if (InitializeWindowsSockets() == false) { }

    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT_WORKER, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_STREAM,                    // stream socket
        IPPROTO_TCP);                   // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
    }

    // Setup the TCP listening socket - bind port number and local address 
    // to socket
    iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
    }

    // Since we don't need resultingAddress any more, free it
    freeaddrinfo(resultingAddress);

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
    }

    printf("Server initialized, waiting for workers to connect.\n");
    pthread_t ListenForNewWorkerThread;
    pthread_t SendWorkerThread;

    args = (arg_struct*)malloc(sizeof(struct arg_struct) * 1);
    args->acceptSocket = listenSocket;

    pthread_create(&ListenForNewWorkerThread, NULL, &AcceptWorkers, args);
    pthread_create(&SendWorkerThread, NULL, &Send, NULL);

    (void)pthread_join(ListenForNewWorkerThread, NULL);
    (void)pthread_join(SendWorkerThread, NULL);

    // shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
    }

    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();

    return NULL;
}

void* ListeningForNewClients(void* arguments) {
    // Socket used for listening for new clients 
    SOCKET listenSocket = INVALID_SOCKET;
    // Socket used for communication with client
    SOCKET acceptedSocket = INVALID_SOCKET;
    // variable used to store function return value
    int iResult;
    // Buffer used for storing incoming data

    if (InitializeWindowsSockets() == false) { }

    // Prepare address information structures
    addrinfo* resultingAddress = NULL;
    addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4 address
    hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
    hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
    hints.ai_flags = AI_PASSIVE;     // 

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
    }

    // Create a SOCKET for connecting to server
    listenSocket = socket(AF_INET,      // IPv4 address famly
        SOCK_STREAM,                    // stream socket
        IPPROTO_TCP);                   // TCP

    if (listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        WSACleanup();
    }

    // Setup the TCP listening socket - bind port number and local address 
    // to socket
    iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(resultingAddress);
        closesocket(listenSocket);
        WSACleanup();
    }

    // Since we don't need resultingAddress any more, free it
    freeaddrinfo(resultingAddress);

    // Set listenSocket in listening mode
    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
    }

    printf("Server initialized, waiting for client to connect.\n");

    do
    {
        acceptedSocket = accept(listenSocket, NULL, NULL);

        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
        }

        pthread_t receiveThread;
        args = (arg_struct*)malloc(sizeof(struct arg_struct) * 1);
        args -> acceptSocket = acceptedSocket;
        args -> iResult = iResult;

        pthread_create(&receiveThread, NULL, &ListenClient, args);
    } while (1);

    // shutdown the connection since we're done
    iResult = shutdown(acceptedSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(acceptedSocket);
        WSACleanup();
    }

    closesocket(listenSocket);
    closesocket(acceptedSocket);
    WSACleanup();
}

void* ListenClient(void* arguments) {
    struct arg_struct* args = (arg_struct*)arguments;
    int iResult = args -> iResult;
    SOCKET acceptedSocket = args -> acceptSocket;

    do {
        //Receive data until the client shuts down the connection
        iResult = recv(acceptedSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            printf("Client sent: %s\n", recvbuf);
            circularBufferPush(recvbuf + '\0');

            printf("Successfully pushed to buffer!\n");
        }
        else if (iResult == 0)
        {
            printf("Connection with client closed.\n");
            closesocket(acceptedSocket);
        }
        else
        {
            // there was an error during recv
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(acceptedSocket);
            break;
        }
    } while (1);
    return NULL;
}

void* AcceptWorkers(void* arguments) {
    struct arg_struct* args = (arg_struct*)arguments;

    SOCKET listenSocket = args -> acceptSocket;
    int iResult;
    SOCKET acceptedSocket;
    do
    {
        acceptedSocket = accept(listenSocket, NULL, NULL);
        if (acceptedSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listenSocket);
            WSACleanup();
        }

        printf("*** NEW WORKER CONNECTED (SOCKET: %d) ***\n", acceptedSocket);
        struct Data* a = (Data*)malloc(sizeof(struct Data));
        a -> DataCount = 0;
        a -> acceptedSocket = acceptedSocket;
        insert(a);
    } while (true);
    return NULL;
}

void* Send(void* arguments) {
    do
    {
        SortedList* c = Current();
        char item[1024] = "";
        int iResult;
        if (bufferCheck() && c != NULL)
        {
            strcpy_s(item, circularBufferPop());
            strcat_s(item, "\0");
            iResult = send(c -> data -> acceptedSocket, item, strlen(item), 0);
            AddToCurrent(strlen(item));
           
            strcpy_s(item, "");
            sort();
            Display();
        }
        Sleep(2000);
    } while (1);
}