#pragma once

#include <cstddef>
#include <stdint.h>

extern "C" {
typedef struct __attribute__((__packed__)) _asset {
    uint8_t* buf;
    size_t size;
} asset;
}

#define ASSET(x)                                                               \
    extern "C" {                                                               \
    extern uint8_t _binary_static_##x##_start[], _binary_static_##x##_size[];  \
    static asset x = {_binary_static_##x##_start,                              \
                      (size_t)_binary_static_##x##_size};                      \
    }

namespace voss::utils {

std::vector<Pose> decode_traj_txt(const asset& path_file) {
    std::vector<Pose> robotPath;

    return robotPath;
}
}; // namespace voss::utils