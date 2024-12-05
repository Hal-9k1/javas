#ifndef CONFDATA_HPP
#define CONFDATA_HPP
#include "VersionEntry.hpp"
#include <fstream>
#include <string>
#include <vector>

class ConfData
{
public:
  ConfData(std::string confFilename, std::fstream &confFile);
  void addEntry(const std::string &entry, const std::string &path);
  void listEntries(bool currentOnly);
  void removeEntry(const std::string &entry);
  void makeCurrent(const std::string &entry);
private:
  std::vector<VersionEntry> entries;
  std::string current;
};

#endif
