#include "UserManager.h"

UserInfo *UserManager::GetUserInfoFromID(int nID)
{
    if (!getInstance()->m_mapIdUserInfo.count(nID))
    {
        return nullptr;
    }
    return getInstance()->m_mapIdUserInfo[nID];
}

UserInfo *UserManager::GetUserInfoFromSocket(SOCKET socket)
{
    if (!getInstance()->m_mapSocketId.count(socket))
    {
        return nullptr;
    }
    return getInstance()->m_mapIdUserInfo[getInstance()->m_mapSocketId[socket]];
}

int UserManager::GetIDFromSocket(SOCKET socket)
{
    if (!getInstance()->m_mapSocketId.count(socket))
    {
        return -1;
    }
    return getInstance()->m_mapSocketId[socket];
}

int UserManager::AddUserInfo(UserInfo *userInfo)
{
    std::lock_guard<std::mutex> lock(getInstance()->m_mutexIdSocket);
    if (getInstance()->m_mapIdUserInfo.count(userInfo->id))
    {
        return -1;
    }
    getInstance()->m_mapIdUserInfo[userInfo->id] = userInfo;
    getInstance()->m_mapSocketId[userInfo->socket] = userInfo->id;

    return 0;
}

int UserManager::RemoveUserInfoFromID(int id)
{
    std::lock_guard<std::mutex> lock(getInstance()->m_mutexIdSocket);
    if (!getInstance()->m_mapIdUserInfo.count(id))
    {
        return -1;
    }
    getInstance()->m_mapSocketId.erase(getInstance()->m_mapIdUserInfo[id]->socket);
    delete getInstance()->m_mapIdUserInfo[id];
    getInstance()->m_mapIdUserInfo.erase(id);

    return 0;
}

int UserManager::RemoveUserInfoFromSocket(SOCKET socket)
{
    std::lock_guard<std::mutex> lock(getInstance()->m_mutexIdSocket);
    if (!getInstance()->m_mapSocketId.count(socket))
    {
        return -1;
    }
    delete getInstance()->m_mapIdUserInfo[getInstance()->m_mapSocketId[socket]];
    getInstance()->m_mapIdUserInfo.erase(getInstance()->m_mapSocketId[socket]);
    getInstance()->m_mapSocketId.erase(socket);
    return 0;
}

int UserManager::SetUserInfo(UserInfo *userInfo)
{
    std::lock_guard<std::mutex> lock(getInstance()->m_mutexIdSocket);
    getInstance()->m_mapIdUserInfo[userInfo->id] = userInfo;
    getInstance()->m_mapSocketId[userInfo->socket] = userInfo->id;
    return 0;
}

UserManager::UserManager()
{
}

UserManager::~UserManager()
{
    for (auto& item : getInstance()->m_mapIdUserInfo)
    {
        delete item.second;
    }
}
