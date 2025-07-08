#include "ServerMessageManager.h"

void ServerMessageManager::MessageHandle(WebIOData *pWebIOData, UserInfo *userInfo)
{
    switch (userInfo->message->head.messageType)
    {
    case MessageType::COMMAND:
        std::cout << "MessageType::COMMAND\n";
        getInstance()->m_mapMessager[MessageType::COMMAND]->MessageHanle(pWebIOData, userInfo->message, userInfo);
        break;
    default:
        //普通消息转到发送事件进行转发
        std::cout << "CreateSend\n";
        getInstance()->m_mapMessager[MessageType::WORDS]->MessageHanle(pWebIOData, userInfo->message, userInfo);
        break;
    }
}

void ServerMessageManager::SetMessageCallBack(SendMessageCallBack smCallback)
{
    for (auto& item : getInstance()->m_mapMessager)
    {
        item.second->SetMessageCallBack(smCallback);
    }
}

ServerMessageManager::ServerMessageManager()
{
    m_mapMessager[MessageType::COMMAND] = new ServerMessageCommand();
    m_mapMessager[MessageType::WORDS] = new ServerMessageWord();
}

ServerMessageManager::~ServerMessageManager()
{
}
