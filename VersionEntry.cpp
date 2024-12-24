#include "VersionEntry.hpp"

#include "unixish.hpp"
#include "consts.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#ifdef UNIXISH
# include <unistd.h>
#else
# define WIN32_LEAN_AND_MEAN
# undef UNICODE
# include <windows.h>
# include <shellapi.h>
#endif

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

void VersionEntry::makeCurrent(const std::string &javasDir)
{
#ifdef UNIXISH
  if (unlink(javasDir.data()) && errno != ENOENT)
  {
    std::cerr << "FATAL: error when deleting old " << javasDir << " symlink: "
      << strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
  }
  if (symlink(path.data(), javasDir.data()))
  {
    std::cerr << "FATAL: error when creating " << javasDir << " symlink: "
      << strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
  }
#else
  std::string toDelete = javasDir + "\\*";
  toDelete.append(1, '\0');
  SHFILEOPSTRUCT operation{};
  operation.wFunc = FO_DELETE;
  operation.pFrom = toDelete.data();
  operation.fFlags = FOF_NO_UI;
  if (SHFileOperation(&operation) || operation.fAnyOperationsAborted)
  {
    std::cerr << "FATAL: error when deleting old alias files." << std::endl;
    std::exit(EXIT_FAILURE);
  }
  aliasDirectory(path, javasDir);
#endif
}

const std::string &VersionEntry::getName()
{
  return name;
}

void VersionEntry::aliasDirectory(const std::string &dest, const std::string &src)
{
#ifndef UNIXISH
  WIN32_FIND_DATA findData;
  HANDLE hFinder = FindFirstFile(path.data(), &findData);
  if (hFinder != INVALID_HANDLE_VALUE)
  {
    do
    {
      std::string destFile = dest + '\\' + findData.cFileName;
      const std::string srcFile = src + '\\' + findData.cFileName;
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        aliasDirectory(destFile, srcFile);
      }
      else if (SHGetFileInfo(srcFile.data(), 0, nullptr, 0, SHGFI_EXETYPE))
      {
        // File is executable, make an alias
        if (destFile.size() >= 4 && destFile.at(destFile.size() - 4) == '.')
        {
          destFile = destFile.substr(destFile.size() - 4) + ".bat";
        }
        std::ofstream alias(destFile);
        if (!alias.good())
        {
          std::cerr << "Failed to open " << destFile << " to write alias." << std::endl;
          std::exit(EXIT_FAILURE);
        }
        alias << "@echo off" << std::endl << srcFile << " %*" << std::flush;
        alias.close();
      }
    }
    while (FindNextFile(hFinder, &findData));
  }
  if (hFinder == INVALID_HANDLE_VALUE || GetLastError() != ERROR_NO_MORE_FILES)
  {
    std::cerr << "Failed to enumerate directory " << dest << " to create aliases."
      << std::endl;
    std::exit(EXIT_FAILURE);
  }
#endif
}
