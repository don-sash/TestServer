#pragma once

#include "exchange_protocol/excange_protocol.hpp"

namespace FileUtils
{
    std::vector<exchange::FileInfo> getDirInfo(std::string _path);
    void printFileInfoVec(const std::vector<exchange::FileInfo> &infoVec);
    
} // namespace FileUtils

