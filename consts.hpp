#ifndef CONSTS_HPP
#define CONSTS_HPP
#include "unixish.hpp"

const std::string CONF_OPT = "--conf=";
const std::string HELP_OPT = "--help";
const std::string JAVAS_FILE_NAME = ".javas";
const std::string JAVAS_DIR_SUFFIX = ".d";
#ifdef UNIXISH
const std::string PLATFORM_SEPARATOR = "/";
const std::string EXECUTABLE_SUFFIX = "";
#else
const std::string PLATFORM_SEPARATOR = "\\";
const std::string EXECUTABLE_SUFFIX = ".exe";
#endif

#endif
