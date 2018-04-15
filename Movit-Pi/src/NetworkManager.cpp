#include "NetworkManager.h"
#include <stdlib.h>

bool NetworkManager::IsConnected()
{
	int status = system("ping -c 2 google.com > /dev/null");

	if (-1 != status)
	{
		return WEXITSTATUS(status) == 0;
	}

	return false;
}
