#ifndef WEB_DATA_DEF
#define WEB_DATA_DEF

#include <vector>
#include <list>
#include <mutex>
#include <iostream>
#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <mswsock.h>
#endif

#define DEFAULT_SERVER_PORT 11188
#define MAX_LINK 1024
#define WEB_IO_DATA_BUFF_SIZE 1024

enum CommandType
{
    ADD_USER
};

enum MessageType
{
    WORDS,
    PHOTO,
    AUDIO,
    VIDEO,
    FILES,
    COMMAND
};
struct WebMessageHead
{
    int sourceId;
    int destnationId;
    int bufSize;                //此消息的数据段长度
    MessageType messageType;
};

struct WebMessage
{
    WebMessageHead head;
    char* messageData;

    WebMessage(int nSize = 0)
    {
        if (nSize == 0)
        {
            messageData = nullptr;
        }
        else
        {
            messageData = new char[nSize];
        }
        head.bufSize = nSize;
    }
    WebMessage(WebMessage* messageCloned)
    {
        memcpy(&head, &messageCloned->head, sizeof(head));
        SetDataBufSize(head.bufSize);
        memcpy(messageData, messageCloned->messageData, head.bufSize);
    }
    ~WebMessage()
    {
        if (messageData != nullptr)
        {
            delete[] messageData;
        }
        
    }
    void SetDataBufSize(int nSize)
    {
        if (messageData != nullptr)
        {
            delete[] messageData;
        }
        messageData = new char[nSize];
        head.bufSize = nSize;
    }
    void DeepClone(WebMessage* messageCloned)
    {
        memcpy(&head, &messageCloned->head, sizeof(head));
        SetDataBufSize(head.bufSize);
        memcpy(messageData, messageCloned->messageData, head.bufSize);
    }
};

enum WebIOOperate
{
    ACCEPT,
    SEND,
    RECV
};
//用于表示每次IO操作的上下文信息
struct WebIOData {
    OVERLAPPED overlapped;  // 异步IO结构，必须放在首位，以便以此成员地址获取整个结构体地址
    SOCKET socket;          //此次操作的socket
    WSABUF wsabuf;
    WebIOOperate operate;   //操作类型
    char buff[WEB_IO_DATA_BUFF_SIZE];
    int realRecvSize;       //真正接收到的数据大小
    WebIOData()
    {
        wsabuf.buf = buff;
        wsabuf.len = WEB_IO_DATA_BUFF_SIZE;
    }
};

//每个客户端的数据集合体，接收到数据后缓存在此处，再分段处理
struct ClientContext
{
    std::list<char> buffer;
    int expectedLength = -1;    //已接收到的消息长度，-1表示此条数据尚未处理，需先处理消息头
};

enum UserPermission
{
    MANAGER,
    USER
};

struct UserInfo
{
    SOCKET socket;              //此socket要放在第一个，要由此socket获取到此对象地址
    int id;
    UserPermission permission;
    std::mutex lock;
    ClientContext *context;
    WebMessage *message;
    UserInfo()
    {
        context = new ClientContext();
        message = new WebMessage();
    }
    ~UserInfo()
    {
        delete context;
        delete message;
    }
};

#endif