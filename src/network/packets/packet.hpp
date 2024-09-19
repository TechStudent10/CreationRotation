#pragma once
#include <sio_client.h>
#include <map>
#include <string>

#define CR_PACKET(id, name) \
    public: \
        static constexpr int PACKET_ID = id; \
        static constexpr const char* PACKET_NAME = #name; \
        int getPacketID() const override { return this->PACKET_ID; } \
        const char* getPacketName() const override { return this->PACKET_NAME; } \
        template<typename... Args> \
        static name* create(Args ...args) { \
            return new name(args...); \
        }; \
        sio::message::ptr encode() override { \
            auto msg = sio::object_message::create(); \
            msg->get_map() = this->members; \
            return msg; \
        } \
        void decode(sio::message::ptr msg) override { \
            this->from_msg(msg); \
        }

class Packet {
public:
    static const int PACKET_ID = 0000;

    virtual int getPacketID() const = 0; 
    virtual const char* getPacketName() const = 0;

    virtual sio::message::ptr encode() = 0;
    virtual void decode(sio::message::ptr) = 0;

    virtual void from_msg(sio::message::ptr) = 0;
protected:
    std::map<std::string, sio::message::ptr> members;
};

