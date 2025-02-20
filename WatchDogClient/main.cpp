#include "WatchDogClient/WatchDogClient.hpp"

int main() {
    SERVER::WATCHDOG::CLIENT::CWatchDogClient watchDogClient("Config//Config.json");
    SocketAddress serverAddress("13.209.119.115", 19980);

    if (watchDogClient.Initialize(EPROTOCOLTYPE::EPT_TCP, serverAddress)) {
        while (watchDogClient.GetClientRunState())
            watchDogClient.Run();

        watchDogClient.Destroy();
    }
    return 0;
}
