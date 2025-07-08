#ifndef SERVER_MESSAGE_COMMAND_H
#define SERVER_MESSAGE_COMMAND_H
#include "ServerMessage.h"
#include "UserManager.h"

class ServerMessageCommand : public ServerMessage
{
public:
    ServerMessageCommand();
    virtual ~ServerMessageCommand();

    virtual void MessageHanle(WebIOData *pWebIOData, WebMessage *webMessage, UserInfo *userInfo) override;

private:
    //添加用户
    void AddUser(WebIOData *pWebIOData, WebMessage *webMessage, UserInfo *userInfo);
};

#endif