#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <fstream>
#include <stdexcept>
#include <string>

namespace io {

    std::string read_file(std::string const& path) {
        boost::iostreams::mapped_file_source src(path);
        return std::string(src.data(), src.size());
    }

}
