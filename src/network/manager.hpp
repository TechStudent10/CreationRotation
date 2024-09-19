#pragma once
#include <sio_client.h>
#include <network/packets/packet.hpp>

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

        auto socket = this->getSocket(nspace);
        log::debug("registering listener for {} ({}) on {}", T::PACKET_NAME, T::PACKET_ID, nspace);
        socket->on(std::to_string(T::PACKET_ID), [this, callback, nspace, shouldUnbind](sio::event& ev) {
            log::debug("handling packet {} ({}) on {}", T::PACKET_NAME, T::PACKET_ID, nspace);
            T* packet = T::create();
            packet->decode(ev.get_message());
            Loader::get()->queueInMainThread([this, callback, nspace, packet, shouldUnbind] {
                callback(packet);
                log::debug("called callback for {} ({})", T::PACKET_NAME, T::PACKET_ID);
                if (shouldUnbind) {
                    this->unbind<T>(nspace);
                }
            });
        });
    }

    template<typename T>
    requires std::is_base_of_v<Packet, T>
    inline void unbind(std::string nspace = "") {
        if (!this->isConnected) this->connect();

        auto socket = this->getSocket(nspace);
        socket->off(std::to_string(T::PACKET_ID));
        log::debug("removed listener for {} ({}) on {}", T::PACKET_NAME, T::PACKET_ID, nspace);
    }

    void send(Packet* packet, std::string nspace = "");
protected:
    sio::client* client;

    NetworkManager();

    const sio::socket::ptr& getSocket(std::string nspace = "");
};
