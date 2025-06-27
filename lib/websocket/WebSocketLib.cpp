#include "WebSocketLib.h"

WebSocketLib *WebSocketLib::instance = nullptr;

WebSocketLib::WebSocketLib() : _serverAddress(nullptr), _port(0), _endpoint(nullptr), receivedMessage("")
{
    setInstance(this);
}

void WebSocketLib::begin()
{
    webSocket.begin(_serverAddress, _port, _endpoint);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
    delay(100);
}

void WebSocketLib::loop()
{
    webSocket.loop();
}

void WebSocketLib::sendData(const String &message)
{
    webSocket.sendTXT(message.c_str());
}

String WebSocketLib::getData()
{
    String data = receivedMessage;
    receivedMessage = "";
    return data;
}

void WebSocketLib::webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    if (instance != nullptr)
    {
        switch (type)
        {
        case WStype_DISCONNECTED:
            Serial.println("WebSocket Disconnected");
            break;
        case WStype_CONNECTED:
            Serial.println("WebSocket Connected");
            break;
        case WStype_TEXT:
            instance->receivedMessage = String((char *)payload);
            break;
        case WStype_BIN:
            Serial.println("Binary data received");
            break;
        }
    }
}

void WebSocketLib::setInstance(WebSocketLib *inst)
{
    instance = inst;
}

void WebSocketLib::setServerAddress(const char *serverAddress)
{
    _serverAddress = serverAddress;
}

const char *WebSocketLib::getServerAddress() const
{
    return _serverAddress;
}

void WebSocketLib::setPort(int port)
{
    _port = port;
}

int WebSocketLib::getPort() const
{
    return _port;
}
void WebSocketLib::disconnect()
{
    webSocket.disconnect();
}
void WebSocketLib::setEndpoint(const char *endpoint)
{
    _endpoint = endpoint;
}
bool WebSocketLib::isConnected()
{
    return webSocket.isConnected();
}
const char *WebSocketLib::getEndpoint() const
{
    return _endpoint;
}
