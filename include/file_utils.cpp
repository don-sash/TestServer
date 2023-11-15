#include "file_utils.hpp"

#include <filesystem>
#include <iostream>
#include <iomanip>

namespace fs = std::filesystem;
using namespace exchange;

std::vector<FileInfo> FileUtils::getDirInfo(std::string _path)
{
    std::vector<FileInfo> result;
    fs::path path = _path;
    
    std::error_code ec; 
    if(!exists(path, ec))
    {
        result.push_back({FileType::error, 0, "Directory not exists!"});
        return result;
    }

    try{    
        for (const auto & entry : fs::directory_iterator(path))
        {
            FileInfo info;
            info.name = entry.path().filename().string();
            info.type = entry.is_directory() ? FileType::dir : FileType::file;
        
            if(fs::is_regular_file(entry.path()))
			    info.size = static_cast<uint32_t>(fs::file_size(entry.path()));

            result.emplace_back(info);        
        }

    }
    catch(fs::filesystem_error& e)
    {
        result.push_back({FileType::error, 0, e.what()});
        return result;
    }   
    
    return result;
}

void FileUtils::printFileInfoVec(const std::vector<exchange::FileInfo> &infoVec)
{
    if(infoVec.size())
        std::cout << "type      size      name" << std::endl;

    for(const auto & info : infoVec)
    {
        std::string type;
        if(info.type == FileType::dir)
            type = "dir";
        else if(info.type == FileType::error)
            type = "err";

        std::string size;
        if(info.type == FileType::file)
            size = std::to_string(info.size);

        std::cout << std::setw(4)  << type
                  << std::setw(12) << size
                  << "   " << info.name << std::endl;
    }
}