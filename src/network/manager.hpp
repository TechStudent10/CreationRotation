#pragma once
#undef _WINSOCKAPI_ 
// #define WIN32_LEAN_AND_MEAN
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include <network/packets/packet.hpp>
#include <serialization.hpp>

#include <Geode/Geode.hpp>
using namespace geode::prelude;

class NetworkManager {
public:
    static NetworkManager& get() {
        static NetworkManager instance;
        return instance;
    }

    bool isConnected = false;

    void connect();
    void disconnect();

    template<typename T>
    requires std::is_base_of_v<Packet, T>
    inline void on(std::function<void(T*)> callback, std::string nspace = "", bool shouldUnbind = false) {
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
    inline void unbind(std::string nspace = "") {
        if (!this->isConnected) this->connect();

        if (!listeners.contains(T::PACKET_ID)) {
            log::error("unable to remove listener for {} ({}), listener does not exist", T::PACKET_NAME, T::PACKET_ID);
            return;
        }

        listeners.erase(T::PACKET_ID);

        log::debug("removed listener for {} ({}) on {}", T::PACKET_NAME, T::PACKET_ID, nspace);
    }

    template<typename _Packet>
    inline void send(_Packet* packet, std::string nspace = "") {
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
        socket.send(fmt::format("{}|{}", packet->getPacketID(), ss.str()));
    }
protected:
    // sio::client* client;
    ix::WebSocket socket;

    NetworkManager();

    // const sio::socket::ptr& getSocket(std::string nspace = "");
    void onMessage(const ix::WebSocketMessagePtr& msg);

    std::unordered_map<
        int,
        std::function<void(std::string)>
    > listeners;
};
