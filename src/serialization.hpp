#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/archives/json.hpp>

// general serialization stuff
// using cereal

#define CR_SERIALIZE(...) \
    template<class Archive> \
    void serialize(Archive & ar) { ar(__VA_ARGS__); } \
    // CEREAL_REGISTER_TYPE(name);

// #define CR_SPACKET(name, ...) \
//     CR_SERIALIZABLE(name, packet, __VA_ARGS__)

// #define PACKET_ARG(arg) cereal::make_nvp(#arg, packet.arg)

// #define CR_SOBJ(name, ...) \
//     CR_SERIALIZABLE(name, obj, __VA_ARGS__)