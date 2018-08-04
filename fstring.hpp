//
// Created by arroganz on 11/26/17.
//

#ifndef UTILS_FSTRING_HPP
#define UTILS_FSTRING_HPP

# include <sstream>
# include <vector>

namespace futils {
    static const std::string endl = "\0";

    inline bool     isNumber(std::string const &str)
    {
        return str.find_first_not_of("0123456789") == std::string::npos;
    }

    inline std::vector<std::string>    split(std::string const &parent, char delim)
    {
        std::vector<std::string>    result;
        std::stringstream           ss;
        ss.str(parent);
        std::string elem;
        while (std::getline(ss, elem, delim))
        {
            if (elem != "")
                result.push_back(elem);
        }
        return result;
    }
}

#endif //UTILS_FSTRING_HPP
