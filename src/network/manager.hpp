#pragma once
#undef _WINSOCKAPI_ 
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include <network/packets/packet.hpp>
#include <serialization.hpp>

#include <Geode/Geode.hpp>
#include <matjson.hpp>
using namespace geode::prelude;

using DisconnectCallback = std::function<void(std::string)>;

class NetworkManager {
public:
    static NetworkManager& get() {
        static NetworkManager instance;
        return instance;
    }

    void setDisconnectCallback(DisconnectCallback callback) {
        disconnectCallback = callback;
    }
    bool showDisconnectPopup = true;

    bool isConnected = false;

    void connect(bool shouldReconnect = true);
    void disconnect();

    template<typename T>
    requires std::is_base_of_v<Packet, T>
    inline void on(std::function<void(T*)> callback, bool shouldUnbind = false) {
        if (!this->isConnected) this->connect();

        listeners[T::PACKET_ID] = [callback](std::string msg) {
            std::stringstream ss;
            ss << msg;

            T* packet = T::create();

            {
                cereal::JSONInputArchive iarchive(ss);

                iarchive(cereal::make_nvp("packet", *packet));
            }

            Loader::get()->queueInMainThread([callback, packet]() {
                callback(packet);
            });
        };
    }

    template<typename T>
    requires std::is_base_of_v<Packet, T>
    inline void unbind() {
        if (!this->isConnected) this->connect();

        if (!listeners.contains(T::PACKET_ID)) {
            log::error("unable to remove listener for {} ({}), listener does not exist", T::PACKET_NAME, T::PACKET_ID);
            return;
        }

        listeners.erase(T::PACKET_ID);

        log::debug("removed listener for {} ({})", T::PACKET_NAME, T::PACKET_ID);
    }

    template<typename _Packet>
    inline void send(_Packet* packet) {
        if (!this->isConnected) this->connect();

        log::debug("sending packet {} ({})", packet->getPacketName(), packet->getPacketID());
        std::stringstream ss;
        // cereal uses RAII, meaning
        // the contents of `ss` is guaranteed
        // to be filled at the end of the braces
        {
            cereal::JSONOutputArchive oarchive(ss);
            oarchive(cereal::make_nvp("packet", *packet));
        }
        auto json = matjson::parse(ss.str());
        json["packet_id"] = packet->getPacketID();
        auto uncompressedStr = json.dump(0);
        unsigned char* compressedData;

        size_t compressedSize = ZipUtils::ccDeflateMemory(
            reinterpret_cast<unsigned char*>(uncompressedStr.data()),
            uncompressedStr.size(),
            &compressedData
        );
        socket.sendBinary(
            std::string(std::string_view(
                reinterpret_cast<const char*>(compressedData),
                compressedSize
            ))
        );
    }
protected:
    NetworkManager();
    ix::WebSocket socket;

    void onMessage(const ix::WebSocketMessagePtr& msg);

    DisconnectCallback disconnectCallback;

    std::unordered_map<
        int,
        std::function<void(std::string)>
    > listeners;
};
