#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <memory>
#include "WebDataDef.h"

#define BUFF_SIZE 1024
#define MAX_LINK 1024
#define SERVER_IP "127.0.0.1"

int nSendWebData(int socketFd, const char *pData, int nSize, int sourceId, int destId, MessageType type)
{
    std::cout << "Expected Pure Size:" << nSize << std::endl;
    int allSize = 0;
    int nSendSize = 0;
    WebMessageHead head;
    head.bufSize = nSize;
    head.sourceId = sourceId;
    head.destnationId = destId;
    head.messageType = type;
    while (nSendSize < sizeof(WebMessageHead))
    {
        nSendSize += send(socketFd, (char*)&head + nSendSize, sizeof(WebMessageHead) - nSendSize, 0);
        allSize += nSendSize;
    }
    nSendSize = 0;
    while (nSendSize < nSize)
    {
        nSendSize += send(socketFd, pData + nSendSize, nSize - nSendSize, 0);
        allSize += nSendSize;
    }
    std::cout << "Send Finish! All Size:" << allSize << std::endl;
    std::cout << "Pure Size:" << nSendSize << std::endl;
    std::cout << "Send Pure Message:" << pData << std::endl;
    return nSendSize;
}

int nRecvWebData(int socketFd, char *pData)
{
    int nRecvSize = 0;
    int allSize = 0;
    while (nRecvSize < sizeof(WebMessageHead))
    {
        nRecvSize += recv(socketFd, pData + nRecvSize, sizeof(WebMessageHead) - nRecvSize, 0);
        allSize += nRecvSize;
    }
    nRecvSize = 0;
    WebMessage *message = (WebMessage *)pData;
    message->SetDataBufSize(message->head.bufSize);
    while (nRecvSize < message->head.bufSize)
    {
        nRecvSize += recv(socketFd, message->messageData + nRecvSize, message->head.bufSize - nRecvSize, 0);
        allSize += nRecvSize;
    }
    std::cout << "Recv Finish! All Size:" << allSize << std::endl;
    std::cout << "Recv All Data:" << message->messageData << std::endl;
    return nRecvSize;
}

void RecvThread(int socketFd)
{
    WebMessage message(128);
    while (true)
    {
        nRecvWebData(socketFd, (char*)&message);
        std::cout /*<< message.head.sourceId << ":" */<< message.messageData << std::endl;
        Sleep(500);
    }
    
}

void closeExecu()
{
    std::cin.clear();
    std::cin.sync();
    std::cin.get();
}

int  main()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed with error: " << result << "\n";
        closeExecu();
        return 1;
    }
    struct sockaddr_in socketAddr;
    std::shared_ptr<char[]> ptrBuffer(new char[BUFF_SIZE]);
    int socketFd;
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1)
    {
        std::cout << "socket assign erro!";
        closeExecu();
        return -1;
    }
    memset(&socketAddr, 0, sizeof(socketAddr));
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr=inet_addr(SERVER_IP);
    socketAddr.sin_port = htons(DEFAULT_SERVER_PORT);
    if (connect(socketFd, (struct sockaddr*)&socketAddr, sizeof(socketAddr)) == -1)
    {
        std::cout << "connect erro! code = " << WSAGetLastError() << "\n";  // 添加这一行
        closeExecu();
        return -1;
    }
    std::cout << "connect success!\n";

    char commandChar = CommandType::ADD_USER;
    memcpy(ptrBuffer.get(), &commandChar, sizeof(commandChar));
    std::cout << "Please Enter ID:\n";
    int nID = 0;
    std::cin >> nID;
    std::string strHead = std::to_string(nID) + ":";
    std::cout << "Your ID Is:" << nID << std::endl;
    memcpy(ptrBuffer.get() + sizeof(commandChar), &nID, sizeof(nID));
    nSendWebData(socketFd, (char *)ptrBuffer.get(), sizeof(commandChar) + sizeof(nID), nID, nID, MessageType::COMMAND);

    std::thread th(&RecvThread, socketFd);
    th.detach();

    int destId;
    while (1)
    {
        std::cout << "Dest ID Is:" << std::endl;
        std::cin >> destId;
        memset((void*)ptrBuffer.get(), 0, BUFF_SIZE);
        memcpy((void *)ptrBuffer.get(), strHead.data(), strlen(strHead.data()));
        std::cin >> ptrBuffer.get() + strlen(strHead.data());
        nSendWebData(socketFd, (char *)ptrBuffer.get(), strlen(ptrBuffer.get()), nID, destId, MessageType::WORDS);
    }

    closesocket(socketFd);
    WSACleanup();

    return 0;
}