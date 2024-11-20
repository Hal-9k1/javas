#ifndef CONFDATA_HPP
#define CONFDATA_HPP
#include "VersionEntry.hpp"
#include <iostream>
#include <string>
#include <vector>

class ConfData
{
public:
  ConfData(std::string confFilename, std::fstream &confFile);
private:
  std::vector<VersionEntry> entries;
  std::string current;
};

#endif
