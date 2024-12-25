#include "VersionEntry.hpp"

#include "unixish.hpp"
#include "consts.hpp"
#include "fileUtil.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#ifdef UNIXISH
# include <unistd.h>
#else
# define WIN32_LEAN_AND_MEAN
# undef UNICODE
# include <objbase.h>
# include <shellapi.h>
# include <windows.h>

static void maybeCoInitialize()
{
  HRESULT coInitResult = CoInitialize(nullptr);
  if (coInitResult == S_OK)
  {
    std::atexit([] { CoUninitialize(); });
  }
  else if (coInitResult != S_FALSE)
  {
    std::cerr << "FATAL: switch: failed to initialize COM for shell file operations." << std::endl;
    std::exit(EXIT_FAILURE);
  }
}
#endif // else UNIXISH

VersionEntry::VersionEntry(std::istream &reader)
{
  std::getline(reader, name, '\0');
  std::getline(reader, path, '\0');
  if (reader.fail())
  {
    std::cerr << "FATAL: failed to read version entry." << std::endl;
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
    std::cerr << "FATAL: switch: error when deleting old " << javasDir << " symlink: "
      << strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::string binDir = path + "/bin";
  if (symlink(binDir.data(), javasDir.data()))
  {
    std::cerr << "FATAL: switch: error when creating " << javasDir << " symlink: "
      << strerror(errno) << std::endl;
    std::exit(EXIT_FAILURE);
  }
#else
  maybeCoInitialize();
  if (!pathExists(javasDir, true) && mkdir(path.data()))
  {
    std::cerr << "FATAL: failed to create directory " << javasDir << "." << std::endl;
  }
  std::string toDelete = javasDir + "\\*";
  toDelete.append(1, '\0');
  SHFILEOPSTRUCT operation{};
  operation.wFunc = FO_DELETE;
  operation.pFrom = toDelete.data();
  operation.fFlags = FOF_NO_UI;
  int result = SHFileOperation(&operation);
  if (result || operation.fAnyOperationsAborted)
  {
    std::cerr << "FATAL: switch: error when deleting old alias files. Error: " << result
      << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::string binDir = path + "\\bin";
  aliasDirectory(javasDir, binDir);
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
  std::string srcFindPath = src + "\\*";
  HANDLE hFinder = FindFirstFile(srcFindPath.data(), &findData);
  if (hFinder != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
      {
        continue;
      }
      std::string destFile = dest + '\\' + findData.cFileName;
      std::string srcFile = src + '\\' + findData.cFileName;
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        aliasDirectory(destFile, srcFile);
      }
      else if (SHGetFileInfo(srcFile.data(), 0, nullptr, 0, SHGFI_EXETYPE))
      {
        // File is executable, make an alias
        if (destFile.size() >= 4 && destFile.at(destFile.size() - 4) == '.')
        {
          destFile = destFile.substr(0, destFile.size() - 4) + ".bat";
        }
        std::ofstream alias(destFile);
        if (!alias.good())
        {
          std::cerr << "FATAL: switch: Failed to open " << destFile << " to write alias."
            << std::endl;
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
    std::cerr << "FATAL: switch: failed to enumerate directory " << src << " to create aliases."
      << std::endl;
    std::exit(EXIT_FAILURE);
  }
#else
  (void)dest;
  (void)src;
#endif
}
