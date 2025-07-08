#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H
#include <functional>
#include "WebDataDef.h"
#include "iostream"

typedef std::function<void(WebMessage*)> SendMessageCallBack;

class ServerMessage
{
public:
    ServerMessage()
    {
        m_sendMessageCallback = nullptr;
    }
    virtual ~ServerMessage() = default;

    virtual void MessageHanle(WebIOData *pWebIOData, WebMessage *webMessage, UserInfo *userInfo) = 0;

    void SetMessageCallBack(SendMessageCallBack smCallback){
        m_sendMessageCallback = smCallback;
    }

protected:
    SendMessageCallBack m_sendMessageCallback;
    WebMessage *m_webMessage;
};

#endif