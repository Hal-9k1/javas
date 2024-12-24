#ifndef VERSIONENTRY_HPP
#define VERSIONENTRY_HPP
#include <iostream>
#include <string>

class VersionEntry
{
public:
  VersionEntry(std::istream &reader);
  VersionEntry(const std::string &name, const std::string &path);
  void writePretty(std::ostream &writer);
  void writeData(std::ostream &writer);
  void makeCurrent(const std::string &javasDir);
  const std::string &getName();

private:
  void aliasDirectory(const std::string &dest, const std::string &src);
  std::string name;
  std::string path;
};

#endif
