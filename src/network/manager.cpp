#include "manager.hpp"

NetworkManager::NetworkManager() {}

void NetworkManager::connect() {
    if (this->isConnected) return; // why are you connecting twice

    client = new sio::client();
    client->connect("http://127.0.0.1:3000"); // TODO: make this not localhost
    this->isConnected = true;
}

void NetworkManager::disconnect() {
    this->getSocket()->close();
    this->isConnected = false;
}

void NetworkManager::send(Packet* packet, std::string nspace) {
    if (!this->isConnected) this->connect();

    log::debug("sending packet {} ({})", packet->getPacketName(), packet->getPacketID());
    auto socket = this->getSocket(nspace);
    auto msgList = packet->encode();
    msgList->get_map()["packet_id"] = sio::int_message::create(packet->getPacketID());
    socket->emit("packet", msgList);
}

const sio::socket::ptr& NetworkManager::getSocket(std::string nspace) {
    return client->socket(nspace);
}
