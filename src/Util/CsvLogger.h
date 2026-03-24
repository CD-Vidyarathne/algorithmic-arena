#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

class CsvLogger {
  public:
    explicit CsvLogger(const std::string &filename) {
        const std::filesystem::path p(filename);
        if (p.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(p.parent_path(), ec);
        }
        file_.open(filename, std::ios::out | std::ios::trunc);
        if (file_)
            file_ << "timestamp_s,fps,entity_count,collision_us_sum_1s,pathfinding_us_sum_1s,"
                     "path_calls_sum_1s,minion_count\n";
    }

    bool isOpen() const { return file_.is_open(); }

    void log(float ts, float fps, int entities, double collisionUs, double pathfindingUs,
             int pathCalls, int minionCount) {
        if (!file_)
            return;
        file_ << ts << ',' << fps << ',' << entities << ',' << collisionUs << ','
              << pathfindingUs << ',' << pathCalls << ',' << minionCount << '\n';
        file_.flush();
    }

  private:
    std::ofstream file_;
};
