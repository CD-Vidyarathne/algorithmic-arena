#pragma once

#include <fstream>
#include <string>

class CsvLogger {
  public:
    explicit CsvLogger(const std::string &filename)
        : file_(filename, std::ios::out | std::ios::trunc) {
        if (file_)
            file_ << "timestamp_s,fps,entity_count,collision_us,pathfinding_us,path_calls,minion_count\n";
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
