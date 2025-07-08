#include "ServerMultiClass.h"

void ServerMulti::startServer()
{
    if (getInstance()->Init() != 0)
    {
        return;
    }

    getInstance()->Working();

    return;
}

ServerMulti::ServerMulti()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed with error: " << result << "\n";
        return;
    }
    m_lpfnAcceptEx = NULL;

    ServerMessageManager::SetMessageCallBack([this](WebMessage *webMessage){ 
        this->CreateSend(webMessage); 
    });
}

ServerMulti::~ServerMulti()
{
    closesocket(m_socket);
    WSACleanup();
}

int ServerMulti::Init()
{
    //支持IOCP的socket
    m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (m_socket == INVALID_SOCKET)
    {
        std::cout << "Socket Init Error!\n";
        return -1;
    }

    memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_port = htons(DEFAULT_SERVER_PORT);
    m_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(m_socket, (struct sockaddr *)&m_serverAddr, sizeof(m_serverAddr)) == SOCKET_ERROR)
    {
        std::cout << "Bind Error!\n";
        return -1;
    }

    if (listen(m_socket, MAX_LINK) == SOCKET_ERROR)
    {
        std::cout << "Listen Error!\n";
        return -1;
    }
    std::cout << "Listening...\n";

    //加载 AcceptEx 函数指针（使用 GUID）
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes = 0;
    WSAIoctl(
        m_socket,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &GuidAcceptEx,
        sizeof(GuidAcceptEx),
        &m_lpfnAcceptEx,
        sizeof(m_lpfnAcceptEx),
        &dwBytes,
        NULL,
        NULL
    );
    if (!m_lpfnAcceptEx) 
    {
        std::cerr << "Failed to load AcceptEx function!\n";
        return -1;
    }

    // 创建 IOCP
    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    // 将监听Socket绑定到IOCP
    HANDLE hTemp = CreateIoCompletionPort(
        (HANDLE)m_socket, 
        m_hIOCP, 
        (ULONG_PTR)0,  // 使用0标识监听Socket
        0
    );
    if (hTemp == NULL) {
        std::cerr << "Bind listen socket to IOCP failed: " << GetLastError() << "\n";
        return -1;
    }

    //提前投递多个 AcceptEx 请求（准备连接）
    for (int i = 0; i < 10; ++i) {
        PostAccept();
    }

    return 0;
}

int ServerMulti::Working()
{
    for (size_t i = 0; i < MAX_THREAD_NUM; i++)
    {
        //m_threadPool->PushThread(getInstance()->WorkThread());
        std::thread(&ServerMulti::WorkThread, this).detach();
    }
    
    return 0;
}

void ServerMulti::WorkThread()
{
    DWORD transferBuffSize;
    ULONG_PTR transferSocket;
    LPOVERLAPPED transferOverlapped;

    while (1)
    {
        // 等待IO完成通知（阻塞直到某个IO完成）
        BOOL result = GetQueuedCompletionStatus(m_hIOCP, &transferBuffSize, &transferSocket, &transferOverlapped, INFINITE);
        std::cout << result << " " << transferBuffSize << std::endl;
        UserInfo *userInfo = (UserInfo *)transferSocket;
        //通过overlapped成员地址来获取整个结构体对象地址
        WebIOData *webIOData = (WebIOData *)transferOverlapped;
        
        //若是IO结果失败则处理结束事件
        if (!result || (transferBuffSize == 0 && webIOData->operate != WebIOOperate::ACCEPT) ||
                transferOverlapped == nullptr)
        {
            std::cout << "close socket!\n";
            SOCKET socketTemp = (SOCKET)transferSocket;
            closesocket(socketTemp);

            UserManager::RemoveUserInfoFromSocket(socketTemp);

            if (transferOverlapped != nullptr)
            {
                delete (WebIOData*)transferOverlapped;
            }
            continue;
        }
        webIOData->realRecvSize = transferBuffSize;
        switch (webIOData->operate)
        {
        case WebIOOperate::ACCEPT:
            std::cout << "SWITCH ACCEPT" << std::endl;
            WorkAccept(webIOData);
            break;
        case WebIOOperate::RECV:
            if (userInfo)
            {
                std::cout << "SWITCH RECV" << std::endl;
                WorkRecv(webIOData, userInfo);
            }
            break;
        case WebIOOperate::SEND:
            //在响应事件中，send不响应任何操作
            //WorkSend(webIOData);
            break;
        default:
            std::cout << "LAST MESSAGE!!!" << std::endl;
            break;
        }

        //删除此次IO操作对象，可设计对象池放入
        delete webIOData;
    }
    
}

void ServerMulti::PostAccept()
{
    //准备IO操作数据
    WebIOData *webIOData = new WebIOData();
    webIOData->socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    webIOData->operate = WebIOOperate::ACCEPT;
    memset(&webIOData->overlapped, 0, sizeof(webIOData->overlapped));

    DWORD bytes = 0;
    // 发起异步 accept 连接请求
    BOOL bRet = m_lpfnAcceptEx(
        m_socket,
        webIOData->socket,
        webIOData->wsabuf.buf,
        0,  // 不预接收数据，仅建立连接
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        &bytes,
        &webIOData->overlapped
    );
    if (bRet == FALSE && WSAGetLastError() != ERROR_IO_PENDING)
    {
        std::cout << "AcceptEx failed: " << WSAGetLastError() << std::endl;
        delete webIOData;
    }
}

void ServerMulti::WorkAccept(WebIOData *pWebIOData)
{
    //为此socket创建一个新用户对象，并绑定此socket
    UserInfo *ptrUser = new UserInfo();
    ptrUser->socket = pWebIOData->socket;

    //将此socket绑定IOCP
    if (!CreateIoCompletionPort((HANDLE)ptrUser->socket, m_hIOCP, (ULONG_PTR)ptrUser, 0)) 
    {
        std::cerr << "Bind socket to IOCP failed: " << GetLastError() << "\n";
        closesocket(ptrUser->socket);
        delete ptrUser;
        return;
    }
    setsockopt(pWebIOData->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&m_socket, sizeof(m_socket));
    //为此socket开启接收操作
    CreateRecv(ptrUser->socket);

    //开启新的accept请求
    PostAccept();
}

void ServerMulti::WorkRecv(WebIOData *pWebIOData, UserInfo *userInfo)
{
    if (HandleRecvStick(pWebIOData, userInfo, userInfo->message) != 0)
    {
        //为此socket开启下一次接收操作
        CreateRecv(pWebIOData->socket);
        return;
    }
    std::cout << "Get Full Message!\n";
    std::cout << "Full Message Is:\n"
              << userInfo->message->messageData << std::endl;
    ServerMessageManager::MessageHandle(pWebIOData, userInfo);
    //为此socket开启下一次接收操作
    CreateRecv(pWebIOData->socket);
}

int ServerMulti::HandleRecvStick(WebIOData *pWebIOData, UserInfo *userInfo, WebMessage *webMessage)
{
    std::lock_guard<std::mutex> lock(userInfo->lock);
    std::cout << "Handle Start\n";
    //将单次接收到的数据存入user缓存中
    userInfo->context->buffer.insert(userInfo->context->buffer.end(),
                                     pWebIOData->buff,
                                     pWebIOData->buff + pWebIOData->realRecvSize);
    std::cout << "user All Buff Size:" << userInfo->context->buffer.size() << std::endl;
    while (1)
    {
        //处理消息头
        if (userInfo->context->expectedLength == -1)
        {
            if (userInfo->context->buffer.size() >= sizeof(WebMessageHead))
            {
                //取出消息头并剔除user中的此消息头数据
                std::copy(userInfo->context->buffer.begin(), 
                        std::next(userInfo->context->buffer.begin(), sizeof(WebMessageHead)),
                        (char*)webMessage);
                userInfo->context->buffer.erase(userInfo->context->buffer.begin(),
                                                std::next(userInfo->context->buffer.begin(), sizeof(WebMessageHead)));
                userInfo->context->expectedLength = 0;
                webMessage->SetDataBufSize(webMessage->head.bufSize);
                std::cout << "user Pure Buff Size:" << webMessage->head.bufSize << std::endl;
                std::cout << "No Head Buff Size:" << userInfo->context->buffer.size() << std::endl;
            }
            else
            {
                //消息头没接完，返回接着接数据
                std::cout << "Head Less!\n";
                return -1;
            }
        }
        //处理消息段
        if (userInfo->context->expectedLength != -1 && userInfo->context->buffer.size() >= webMessage->head.bufSize)
        {
            //开始获取完整的消息段数据
            std::copy(userInfo->context->buffer.begin(), 
                        std::next(userInfo->context->buffer.begin(), webMessage->head.bufSize),
                        webMessage->messageData);
            //删除缓存中的消息段数据
            userInfo->context->buffer.erase(userInfo->context->buffer.begin(),
                                            std::next(userInfo->context->buffer.begin(), webMessage->head.bufSize));
            //重置长度标记位
            userInfo->context->expectedLength = -1;
            std::cout << "Message Data Full!\n";
            return 0;
        }
        else
        {
            //消息段没接完，返回接着接数据
            std::cout << "Message Data Less!\n";
            return -1;
        }
    }
    
    return 0;
}

void ServerMulti::CreateSend(WebMessage *webMessage)
{
    WebIOData *newWebIOData = new WebIOData();
    newWebIOData->operate = WebIOOperate::SEND;

    //如果不存在此目标ID信息，提前结束
    if (UserManager::GetUserInfoFromID(webMessage->head.sourceId) == nullptr ||
            UserManager::GetUserInfoFromID(webMessage->head.destnationId) == nullptr)
    {
        delete newWebIOData;
        return;
    }
    //此处socket是发送者的socket
    newWebIOData->socket = UserManager::GetUserInfoFromID(webMessage->head.sourceId)->socket;
    UserInfo *destUserInfo = UserManager::GetUserInfoFromID(webMessage->head.destnationId);
    DWORD sent = 0;
    //将消息数据复制到IOData中
    newWebIOData->wsabuf.len = sizeof(WebMessageHead) + webMessage->head.bufSize;
    memcpy(newWebIOData->wsabuf.buf, webMessage, sizeof(WebMessageHead));
    memcpy(newWebIOData->wsabuf.buf + sizeof(WebMessageHead), webMessage->messageData, webMessage->head.bufSize);
    std::cout << "WSA BUF Is:" << newWebIOData->wsabuf.buf << std::endl;
    WSASend(destUserInfo->socket, &newWebIOData->wsabuf, 1, &sent, 0, &newWebIOData->overlapped, NULL);
}

void ServerMulti::CreateRecv(SOCKET socket)
{
    WebIOData *newWebIOData = new WebIOData();
    newWebIOData->socket = socket;
    newWebIOData->operate = WebIOOperate::RECV;
    memset(&newWebIOData->overlapped, 0, sizeof(newWebIOData->overlapped));
    DWORD dFlag = 0;
    //此处接收的buff地址是一整个包括数据头的messageData，接收到后需要先处理数据头
    WSARecv(newWebIOData->socket, &newWebIOData->wsabuf, 1, NULL, &dFlag, &newWebIOData->overlapped, NULL);
}
