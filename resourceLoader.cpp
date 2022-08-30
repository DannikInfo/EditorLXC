#include <fstream>
#include <cstring>
#include "resourceLoader.h"

void resourceLoader::load() {
    auto fs = cmrc::xcle::get_filesystem();
    std::string path = "assets";

    if(!std::filesystem::exists(path))
        std::filesystem::create_directories(path);

    for (const auto &item : fs.iterate_directory(path)){
        if(!std::filesystem::exists(path+"/"+item.filename()) && !strstr(item.filename().c_str(), ".hpp") && !strstr(item.filename().c_str(), ".cpp")) {
            auto f = fs.open(path + "/" + item.filename());
            std::ofstream o(path+"/"+item.filename());
            for (const auto &ch : f)
                o << ch;
            o.close();
        }
    }
}
