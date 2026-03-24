#include "PathfindingPerf.h"

namespace PathfindingPerf {

namespace {
std::int64_t g_frameNanos = 0;
int g_frameCalls = 0;
}  // namespace

void beginFrame() {
    g_frameNanos = 0;
    g_frameCalls = 0;
}

void recordFindPath(std::chrono::nanoseconds duration) {
    g_frameNanos += duration.count();
    ++g_frameCalls;
}

std::int64_t lastFrameNanos() {
    return g_frameNanos;
}

int lastFrameCalls() {
    return g_frameCalls;
}

}  // namespace PathfindingPerf
