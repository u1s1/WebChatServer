#ifndef SERVER_MESSAGE_WORD_H
#define SERVER_MESSAGE_WORD_H
#include "ServerMessage.h"
#include "UserManager.h"

class ServerMessageWord : public ServerMessage
{
public:
    ServerMessageWord();
    virtual ~ServerMessageWord();

    virtual void MessageHanle(WebIOData *pWebIOData, WebMessage *webMessage, UserInfo *userInfo) override;
};

#endif