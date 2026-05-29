#pragma once

#include <SDL.h>

class Timer {
    Uint64 startCount_;
    Uint64 frequency_;
public:
    Timer() : frequency_(SDL_GetPerformanceFrequency()) {
        Start();
    }

    void Start() {
        startCount_ = SDL_GetPerformanceCounter();
    }

    float elapsed() const {
        Uint64 now = SDL_GetPerformanceCounter();
        return static_cast<float>(now - startCount_) / static_cast<float>(frequency_);
    }

    float elapsedMs() const { return elapsed() * 1000.0f; }

    float reset() {
        float e = elapsed();
        Start();
        return e;
    }
}; // Usage : Timer timer;
//         float dt = timer.reset();