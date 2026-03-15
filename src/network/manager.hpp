#pragma once
#include <functional>
#undef _WINSOCKAPI_ 
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include <network/packets/packet.hpp>
#include <serialization.hpp>

#include <Geode/Geode.hpp>
#include <matjson.hpp>

#include <defs.hpp>

using namespace geode::prelude;

using DisconnectCallback = geode::Function<void(std::string)>;
using MiddlewareCb = geode::Function<void(std::function<void()> callback)>;

class CR_DLL NetworkManager : public CCObject {
public:
    static NetworkManager& get() {
        static NetworkManager instance;
        return instance;
    }

    void setDisconnectCallback(DisconnectCallback callback) {
        disconnectBtnCallback = std::move(callback);
    }

    void onDisconnect(DisconnectCallback callback) {
        disconnectEventCb = std::move(callback);
    }

    template<typename T>
    inline void addToQueue(T* packet) {
        this->packetQueue.push_back([this, packet]() { this->send(packet); });
    }

    bool showDisconnectPopup = true;
    bool isConnected = false;

    float responseTime = 0;

    MiddlewareCb middleware;

    void connect(bool shouldReconnect = true, std::function<void()> callback = []() {});
    void disconnect();

    template<typename T>
    requires std::is_base_of_v<Packet, T>
    inline void on(std::function<void(T)> callback, bool shouldUnbind = false) {
        listeners[T::PACKET_ID] = [callback = std::move(callback)](std::string msg) mutable {
            std::stringstream ss(std::move(msg));

            T packet = T::create();

            {
                cereal::JSONInputArchive iarchive(ss);

                iarchive(cereal::make_nvp("packet", packet));
            }

            Loader::get()->queueInMainThread([callback, packet = std::move(packet)]() mutable {
                callback(std::move(packet));
            });
        };
    }

    template<typename T>
    requires std::is_base_of_v<Packet, T>
    inline void unbind() {
        if (!this->isConnected) return;

        if (!listeners.contains(T::PACKET_ID)) {
            log::error("unable to remove listener for {} ({}), listener does not exist", T::PACKET_NAME, T::PACKET_ID);
            return;
        }

        listeners.erase(T::PACKET_ID);

        log::debug("removed listener for {} ({})", T::PACKET_NAME, T::PACKET_ID);
    }

    template<typename T>
    inline void send(T const& packet, std::function<void()> callback = nullptr) {
        if (!this->isConnected) {
            return;
        }

#ifdef CR_DEBUG
        log::debug("sending packet {} ({})", packet.getPacketName(), packet.getPacketID());
#endif
        std::stringstream ss;
        // cereal uses RAII, meaning
        // the contents of `ss` is guaranteed
        // to be filled at the end of the braces
        {
            cereal::JSONOutputArchive oarchive(ss);
            oarchive(cereal::make_nvp("packet", packet));
        }
        auto json = matjson::parse(ss.str()).mapErr([](std::string err) { return err; }).unwrap();

        json["packet_id"] = packet.getPacketID();
        auto uncompressedStr = json.dump(0);
        unsigned char* compressedData;

        size_t compressedSize = ZipUtils::ccDeflateMemory(
            reinterpret_cast<unsigned char*>(uncompressedStr.data()),
            uncompressedStr.size(),
            &compressedData
        );
        ix::IXWebSocketSendData data(
            reinterpret_cast<const char*>(compressedData),
            compressedSize
        );
        socket.sendBinary(
            data, [callback = std::move(callback)](int current, int total) mutable {
                if (current + 1 == total && callback) {
                    Loader::get()->queueInMainThread([callback = std::move(callback)]() mutable {
                        callback();
                    });
                }
                return true;
            }
        );
    }
protected:
    NetworkManager();
    ix::WebSocket socket;

    void onMessage(const ix::WebSocketMessagePtr& msg);
    void update(float dt);

    DisconnectCallback disconnectBtnCallback;
    DisconnectCallback disconnectEventCb;

    std::unordered_map<
        int,
        geode::Function<void(std::string)>
    > listeners;

    std::vector<
        geode::Function<void()>
    > packetQueue;
};
