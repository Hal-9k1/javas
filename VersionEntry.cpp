#include "VersionEntry.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

VersionEntry::VersionEntry(std::ifstream &reader)
{
  std::string version;
  std::string path;
  std::getline(reader, version, '\0');
  std::getline(reader, path, '\0');
  if (reader.fail())
  {
    std::cerr << "Failed to read version entry." << std::endl;
    std::exit(1);
  }
}

void VersionEntry::write(std::ostream &writer)
{
  writer << version << '\0' << path << '\0';
}
