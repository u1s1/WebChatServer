#include "ServerMessageCommand.h"

ServerMessageCommand::ServerMessageCommand()
{
    m_webMessage = new WebMessage();
}

ServerMessageCommand::~ServerMessageCommand()
{
    delete m_webMessage;
}

void ServerMessageCommand::MessageHanle(WebIOData *pWebIOData, WebMessage *webMessage, UserInfo *userInfo)
{
    m_webMessage->DeepClone(webMessage);

    char CommandTypeNum;
    memcpy(&CommandTypeNum, m_webMessage->messageData, sizeof(char));
    std::cout << CommandTypeNum << std::endl;
    switch (CommandTypeNum)
    {
    case CommandType::ADD_USER:
        std::cout << "ADD_USER\n";
        AddUser(pWebIOData, m_webMessage, userInfo);
        break;
    default:
        break;
    }
}

void ServerMessageCommand::AddUser(WebIOData *pWebIOData, WebMessage *webMessage, UserInfo *userInfo)
{
    userInfo->id = webMessage->head.sourceId;
    userInfo->socket = pWebIOData->socket;
    userInfo->permission = UserPermission::USER;
    if (UserManager::AddUserInfo(userInfo) != 0)
    {
        std::cout << "Add Failed User ID:" << userInfo->id << std::endl;
        return;
    }
    std::cout << "Add User ID:" << userInfo->id << std::endl;
}