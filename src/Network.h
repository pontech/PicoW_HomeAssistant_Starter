#ifndef SRC_NETWORK
#define SRC_NETWORK

class Network {
    public:
    static void connect();
    static void disconnect();
    static const char* WiFiStatusString(uint8_t wifi_status);
};

#endif // SRC_NETWORK
