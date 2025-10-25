#pragma once

#include <random>

inline float randf()
{
    auto gen = std::mt19937(std::random_device {}());
    auto dist = std::uniform_real_distribution<float>(0.0f, 1.0f);
    return dist(gen);
}

inline float randf_normal()
{
    auto gen = std::mt19937(std::random_device {}());
    auto dist = std::normal_distribution<float>(0.0f, 1.0f);
    return dist(gen);
}
