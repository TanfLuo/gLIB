////////////////////////////////////////////////////////////////////////////////
/// \file      GConfig.cpp
/// \version   0.2
/// \date      November 2020
/// \author    Gino Francesco Bogo
/// \copyright This file is released under the MIT license
////////////////////////////////////////////////////////////////////////////////

#include "GConfig.hpp"

#include "GLogger.hpp"

#include <fmt/core.h>

bool GConfig::ReadOptions(const std::string &filename) {
    if (m_options.Read(filename)) {
        LOG_WRITE(trace, fmt::format("The \"{}\" configuration file has been loaded", filename));
        return true;
    }
    else {
        LOG_WRITE(error, fmt::format("Unable to load the \"{}\" configuration file", filename));

        if (m_options.size() > 0) {
            m_options.Write("default.cfg");
            LOG_WRITE(warning, "A \"default.cfg\" configuration file was created for reference");
        }
    }
    return false;
}

bool GConfig::WriteOptions(const std::string &filename) {
    return m_options.Write(filename);
}