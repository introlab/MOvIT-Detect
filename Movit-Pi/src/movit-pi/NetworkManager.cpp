#include "NetworkManager.h"
#include <stdlib.h>
#include <string>
#include <fstream>

bool NetworkManager::IsConnected()
{
    int status = system("ping -c 2 google.com > /dev/null");

    if (-1 != status)
    {
        return WEXITSTATUS(status) == 0;
    }

    return false;
}
void NetworkManager::ChangeNetwork(std::string message)
{
    std::ofstream myfile;
    myfile.open("/etc/wpa_supplicant/wpa_supplicant.conf");
    myfile << "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev \n";
    myfile << "update_config=1 \n \n";
    myfile << "network={ \n";
    myfile << "ssid=\"";
    myfile << message.substr(0,message.find(";"));
    myfile << "\" \n psk=\"";
    myfile << message.substr(message.find(";")+1);
    myfile << "\" \n } \n";
    myfile.close();	
    system("sudo wpa_cli -i wlan1 reconfigure");
    printf("Wifi have been change to %s\n", message.substr(0,message.find(";")).c_str());
} 
