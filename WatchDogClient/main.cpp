#include "WatchDogClient/WatchDogClient.hpp"

int main() {
    SERVER::WATCHDOG::CLIENT::CWatchDogClient watchDogClient("Config//Config.json");
    SocketAddress serverAddress("127.0.0.1", 3590);

    if (watchDogClient.Initialize(EPROTOCOLTYPE::EPT_TCP, serverAddress)) {
        while (watchDogClient.GetClientRunState())
            watchDogClient.Run();

        watchDogClient.Destroy();
    }
    return 0;
}
