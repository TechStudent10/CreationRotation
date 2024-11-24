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
    void serialize(Archive & ar) { ar(__VA_ARGS__); }