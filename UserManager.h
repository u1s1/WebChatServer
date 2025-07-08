#ifndef USER_MANAGER_H
#define USER_MANAGER_H
#include <map>
#include <winsock2.h>
#include "WebDataDef.h"

class UserManager
{
public:
    static UserInfo *GetUserInfoFromID(int nID);

    static UserInfo *GetUserInfoFromSocket(SOCKET socket);

    static int GetIDFromSocket(SOCKET socket);

    static int AddUserInfo(UserInfo* userInfo);

    static int RemoveUserInfoFromID(int id);

    static int RemoveUserInfoFromSocket(SOCKET socket);

    static int SetUserInfo(UserInfo* userInfo);

private:
    static UserManager *getInstance()
    {
        static UserManager manager;
        return &manager;
    }
    UserManager();
    ~UserManager();

private:
    std::map<SOCKET, int> m_mapSocketId;        //存放Socket与ID对应关系
    std::map<int, UserInfo *> m_mapIdUserInfo;  // 存放ID与Socket对应关系
    std::mutex m_mutexIdSocket;                 //ID与Socket对应关系表锁
};

#endif