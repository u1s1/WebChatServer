#ifndef SERVER_MESSAGE_MANAGER_H
#define SERVER_MESSAGE_MANAGER_H

#include <functional>
#include "ServerMessageCommand.h"
#include "ServerMessageWord.h"

class ServerMessageManager
{
public:
    static void MessageHandle(WebIOData *pWebIOData, UserInfo *userInfo);

    static void SetMessageCallBack(SendMessageCallBack smCallback);

private:
    static ServerMessageManager *getInstance()
    {
        static ServerMessageManager manager;
        return &manager;
    }
    ServerMessageManager();
    ~ServerMessageManager();

    std::map<MessageType, ServerMessage *> m_mapMessager;
};

#endif