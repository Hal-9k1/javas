#ifndef CONFDATA_HPP
#define CONFDATA_HPP
#include "VersionEntry.hpp"
#include <fstream>
#include <string>
#include <vector>

class ConfData
{
public:
  ConfData();
  void read(std::string confFilename, std::istream &confFile);
  void addEntry(const std::string &name, const std::string &path);
  void listEntries(bool currentOnly);
  void removeEntry(const std::string &name);
  void makeCurrent(const std::string &javasDir, const std::string &name);
  bool isEntry(const std::string &name);
  void write(std::ostream &confFile);
private:
  bool maybeFindEntry(const std::string &name, int *pIdx);
  int findEntry(const std::string &name);
  int currentIdx;
  std::vector<VersionEntry> entries;
};

#endif
