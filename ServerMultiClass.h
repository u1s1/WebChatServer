#ifndef SERVER_MULTI_CLASS
#define SERVER_MULTI_CLASS

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <iostream>
#include <memory>
#include <atomic>
#include <map>
#include <thread>
#include "WebDataDef.h"
#include "ServerMessageManager.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#define MAX_THREAD_NUM 4

class ServerMulti
{
public:
    static void startServer();
private:
    static ServerMulti* getInstance()
    {
        static ServerMulti instance;
        return &instance;
    }
    ServerMulti();
    ~ServerMulti();

    int Init();

    //启动工作
    int Working();

    //真正工作的线程
    void WorkThread();

    //投递一个 AcceptEx 请求（准备接受新连接）
    void PostAccept();

    //IO Accept事件
    void WorkAccept(WebIOData *pWebIOData);

    //IO 接收事件
    void WorkRecv(WebIOData *pWebIOData, UserInfo *userInfo);
    //处理接收粘包
    int HandleRecvStick(WebIOData *pWebIOData, UserInfo *userInfo, WebMessage *webMessage);
    //为socket创建一次接收操作
    void CreateRecv(SOCKET socket);

    //为socket创建一次发送操作
    void CreateSend(WebMessage *webMessage);

private:
    SOCKET m_socket;
    struct sockaddr_in m_serverAddr;        //存放server信息
    HANDLE m_hIOCP;                        //全局IO完成端口句柄
    LPFN_ACCEPTEX m_lpfnAcceptEx;           //AcceptEx函数指针（通过GUID获取）
};

#endif