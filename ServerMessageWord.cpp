#include "ServerMessageWord.h"

ServerMessageWord::ServerMessageWord()
{
    m_webMessage = new WebMessage();
}

ServerMessageWord::~ServerMessageWord()
{
    delete m_webMessage;
}

void ServerMessageWord::MessageHanle(WebIOData *pWebIOData, WebMessage *webMessage, UserInfo *userInfo)
{
    m_sendMessageCallback(webMessage);
}
