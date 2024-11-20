#ifndef VERSIONENTRY_HPP
#define VERSIONENTRY_HPP
#include <iostream>
#include <string>

class VersionEntry
{
public:
  VersionEntry(std::ifstream &reader);
  void write(std::ostream &writer);
private:
  std::string name;
  std::string path;
};

#endif
