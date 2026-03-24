#pragma once

#include <chrono>
#include <cstdint>

/** Lightweight frame-scoped counters for pathfinding (called from deep in findPath). */
namespace PathfindingPerf {

void beginFrame();
void recordFindPath(std::chrono::nanoseconds duration);

std::int64_t lastFrameNanos();
int lastFrameCalls();

struct FindPathScope {
    std::chrono::steady_clock::time_point t0{std::chrono::steady_clock::now()};
    ~FindPathScope() {
        recordFindPath(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - t0));
    }
};

}  // namespace PathfindingPerf
