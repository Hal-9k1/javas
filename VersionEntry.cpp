#include "VersionEntry.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

VersionEntry::VersionEntry(std::istream &reader)
{
  std::getline(reader, name, '\0');
  std::getline(reader, path, '\0');
  if (reader.fail())
  {
    std::cerr << "FATAL: Failed to read version entry." << std::endl;
    std::exit(EXIT_FAILURE);
  }
}
VersionEntry::VersionEntry(const std::string &name, const std::string &path)
  : name(name), path(path)
{ }

void VersionEntry::writePretty(std::ostream &writer)
{
  writer << name << " : " << path;
}
void VersionEntry::writeData(std::ostream &writer)
{
  writer << name << '\0' << path << '\0';
}

void VersionEntry::makeCurrent()
{

}

const std::string &VersionEntry::getName()
{
  return name;
}
