#include "Game.h"
#include <spdlog/spdlog.h>
#include <string>

int main(int argc, char **argv) {

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");

    GameOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--csv" && i + 1 < argc) {
            opts.benchmarkCsvPath = argv[++i];
        } else if (a == "--unlimited-fps") {
            opts.uncappedFps = true;
        } else if (a == "--map" && i + 1 < argc) {
            opts.mapPath = argv[++i];
        }
    }

    Game game(opts);
    game.run();

    return 0;
}
