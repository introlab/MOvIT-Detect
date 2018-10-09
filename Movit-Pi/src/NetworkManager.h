#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
#include <string>

class NetworkManager
{
  public:
    static bool IsConnected();
    static void ChangeNetwork(std::string message);
};

#endif // NETWORK_MANAGER_H
