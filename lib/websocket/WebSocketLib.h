#ifndef WEBSOCKETLIB_H
#define WEBSOCKETLIB_H

#include <WiFi.h>
#include <WebSocketsClient.h>

class WebSocketLib
{
public:
    WebSocketLib();
    void begin();
    void loop();
    void sendData(const String &message);
    String getData();

    void setServerAddress(const char *serverAddress);
    const char *getServerAddress() const;

    void setPort(int port);
    int getPort() const;
    bool isConnected();
    void disconnect();
    void setEndpoint(const char *endpoint);
    const char *getEndpoint() const;

private:
    const char *_serverAddress;
    int _port;
    const char *_endpoint;
    WebSocketsClient webSocket;
    static void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);
    String receivedMessage;
    static WebSocketLib *instance; // Pointer to the instance

    // Helper function to set the instance pointer
    static void setInstance(WebSocketLib *inst);
};

#endif
