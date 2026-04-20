#include "Game.h"
#include <spdlog/spdlog.h>
#include <exception>
#include <string>

int main(int argc, char **argv) {

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

    GameOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        try {
            if (a == "--csv" && i + 1 < argc) {
                opts.benchmarkCsvPath = argv[++i];
            } else if (a == "--unlimited-fps") {
                opts.uncappedFps = true;
            } else if (a == "--map" && i + 1 < argc) {
                opts.mapPath = argv[++i];
            } else if (a == "--benchmark-mode") {
                opts.benchmarkMode = true;
            } else if (a == "--benchmark-duration-s" && i + 1 < argc) {
                opts.benchmarkMode = true;
                opts.benchmarkDurationSeconds = std::stof(argv[++i]);
            } else if (a == "--benchmark-warmup-s" && i + 1 < argc) {
                opts.benchmarkMode = true;
                opts.benchmarkWarmupSeconds = std::stof(argv[++i]);
            } else if (a == "--benchmark-target-minions" && i + 1 < argc) {
                opts.benchmarkMode = true;
                opts.benchmarkTargetMinions = std::stoi(argv[++i]);
            } else if (a == "--benchmark-order-interval-s" && i + 1 < argc) {
                opts.benchmarkMode = true;
                opts.benchmarkOrderIntervalSeconds = std::stof(argv[++i]);
            } else if (a == "--benchmark-seed" && i + 1 < argc) {
                opts.benchmarkMode = true;
                opts.benchmarkSeed = static_cast<std::uint32_t>(std::stoul(argv[++i]));
            }
        } catch (const std::exception &e) {
            spdlog::error("Invalid value for {}: {}", a, e.what());
            return 2;
        }
    }

    Game game(opts);
    game.run();

    return 0;
}
