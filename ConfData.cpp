#include "ConfData.hpp"

#include "VersionEntry.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

const std::string MAGIC = "javas1.0";

ConfData::ConfData() : currentIdx(-1)
{
  entries.reserve(1);
}

void ConfData::read(std::string confFilename, std::istream &confFile)
{
  std::string magic;
  std::getline(confFile, magic, '\0');
  if (magic != MAGIC)
  {
    std::cerr << "FATAL: '" << confFilename << "' does not appear to be a javas"
      << " configuration file." << std::endl;
    std::exit(EXIT_FAILURE);
  }
  char entryCountBuf[4];
  confFile.read(entryCountBuf, 4);
  if (confFile.fail())
  {
    std::cerr << "FATAL: Failed to read version entry count." << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::uint32_t entryCount = 0
    | (static_cast<std::uint32_t>(entryCountBuf[0]) << 0)
    | (static_cast<std::uint32_t>(entryCountBuf[1]) << 8)
    | (static_cast<std::uint32_t>(entryCountBuf[2]) << 16)
    | (static_cast<std::uint32_t>(entryCountBuf[3]) << 24);
  // Ensures vector is never reallocated during program
  entries.reserve(entryCount + 1);
  for (std::size_t i = 0; i < entryCount; ++i)
  {
    if (confFile.peek() == '*')
    {
      if (currentIdx != -1)
      {
        std::cerr << "FATAL: javas configuration file contains more than one"
          << " current version." << std::endl;
        std::exit(EXIT_FAILURE);
      }
      currentIdx = i;
      confFile.ignore(1);
    }
    entries.emplace_back(confFile);
  }
}
void ConfData::addEntry(const std::string &name, const std::string &path)
{
  entries.emplace_back(name, path);
}
void ConfData::listEntries(bool currentOnly)
{
  if (currentOnly)
  {
    if (currentIdx == -1)
    {
      std::cout << "No current version selected." << std::endl;
    }
    else
    {
      entries[currentIdx].writePretty(std::cout);
    }
  }
  else
  {
    std::cout << entries.size() << " versions registered." << std::endl;
    for (int i = 0; i < static_cast<int>(entries.size()); ++i)
    {
      std::cout << "    " << (i == currentIdx ? "(current) " : "");
      entries[i].writePretty(std::cout);
      std::cout << std::endl;
    }
  }
}
void ConfData::removeEntry(const std::string &name)
{
  int idx = findEntry(name);
  if (currentIdx == idx)
  {
    currentIdx = -1;
  }
  else if (currentIdx > idx)
  {
    --currentIdx;
  }
  entries.erase(entries.begin() + idx);
}
void ConfData::makeCurrent(const std::string &javasDir, const std::string &name)
{
  int idx = findEntry(name);
  currentIdx = idx;
  entries[idx].makeCurrent(javasDir);
}
bool ConfData::isEntry(const std::string &name)
{
  return maybeFindEntry(name, nullptr);
}
void ConfData::write(std::ostream &confFile)
{
  confFile << MAGIC << '\0';
  confFile
    << static_cast<char>(entries.size() >> 0 & 0xff)
    << static_cast<char>(entries.size() >> 8 & 0xff)
    << static_cast<char>(entries.size() >> 16 & 0xff)
    << static_cast<char>(entries.size() >> 24 & 0xff);
  for (int i = 0; i < static_cast<int>(entries.size()); ++i)
  {
    if (currentIdx == i)
    {
      confFile << '*';
    }
    entries[i].writeData(confFile);
  }
}
bool ConfData::maybeFindEntry(const std::string &name, int *pIdx)
{
  for (int i = 0; i < static_cast<int>(entries.size()); ++i)
  {
    if (entries[i].getName() == name)
    {
      if (pIdx)
      {
        *pIdx = i;
      }
      return true;
    }
  }
  return false;
}
int ConfData::findEntry(const std::string &name)
{
  int idx;
  if (!maybeFindEntry(name, &idx))
  {
    std::cerr << "FATAL: Unknown entry " << name << "." << std::endl;
    std::exit(EXIT_FAILURE);
  }
  return idx;
}
