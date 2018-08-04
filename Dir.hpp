//
// Created by arroganz on 1/23/18.
//

#ifndef DEMO_DIR_HPP
#define DEMO_DIR_HPP


#ifdef _WIN32
# include <filesystem>
#endif
#include <experimental/filesystem>

namespace futils
{
    class Dir
    {
        std::string _path;
        std::vector<std::string> content;
    public:
        Dir(std::string const &path): _path(path)
        {
            std::experimental::filesystem::path folder{_path.c_str()};
            for (auto &p : std::experimental::filesystem::directory_iterator(folder)) {
                auto pathFile = p.path().string();
                content.push_back(pathFile);
            }
        }

        std::vector<std::string> const &getContent() const {
            return content;
        }
    };
}

#endif //DEMO_DIR_HPP
