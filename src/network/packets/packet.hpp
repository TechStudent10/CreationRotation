#pragma once
#include <string>
#include <serialization.hpp>

#include <defs.hpp>

#define CR_ARGS(...) __VA_ARGS__

#define CR_PACKET(id, name) \
    public: \
        static constexpr int PACKET_ID = id; \
        static constexpr const char* PACKET_NAME = #name; \
        int getPacketID() const override { return this->PACKET_ID; } \
        const char* getPacketName() const override { return this->PACKET_NAME; } \
        template<typename... Args> \
        static name* create(Args ...args) { \
            return new name(args...); \
        } \
        void decode(std::string in) override { \
            \
        } \
        name() {}

class CR_DLL Packet {
public:
    static const int PACKET_ID = 0000;

    virtual void decode(std::string) = 0;

    virtual int getPacketID() const = 0; 
    virtual const char* getPacketName() const = 0;
};

// CR_SPACKET(Packet, PACKET_ARG(PACKET_ID));

// dont mind me just testing the syntax (istg if someone actually uses this)

class TestPacket : public Packet {
    CR_PACKET(0001, TestPacket)

    TestPacket(std::string hello, std::string world):
        hello(hello),
        world(world) {}

    std::string hello;
    std::string world;

    CR_SERIALIZE(hello, world)
};  

