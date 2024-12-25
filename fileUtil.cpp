#include "fileUtil.hpp"

#include "consts.hpp"

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#ifndef UNIXISH
# define WIN32_LEAN_AND_MEAN
# undef UNICODE
# include <windows.h>
#endif

bool pathExists(const std::string &path, bool isDir)
{
  struct stat info;
  int result = stat(path.data(), &info);
  if (result && (errno == ENOENT || errno == ENOTDIR))
  {
    return false;
  }
  else if (result)
  {
    std::cerr << "FATAL: failed to stat " << path << "." << std::endl;
    std::exit(EXIT_FAILURE);
  }
  return isDir
    ? ((info.st_mode & S_IFDIR) != 0)
    : ((info.st_mode & S_IFREG) != 0);
}

void openConfFile(std::fstream &outFile, std::string &outUsedFilename)
{
  std::string &filename = outUsedFilename;
  const char *pFilename = std::getenv("JAVAS");
  if (pFilename)
  {
    filename = std::string(pFilename);
  }
  else
  {
#ifdef UNIXISH
    filename = std::string(std::getenv("HOME"));
#else
    const char *pFilename = std::getenv("USERPROFILE");
    if (pFilename)
    {
      filename = std::string(pFilename);
    }
    else
    {
      const char *var = std::getenv("HOMEDRIVE");
      if (var)
      {
        filename += var;
      }
      var = std::getenv("HOMEPATH");
      if (var)
      {
        filename += var;
      }
    }
#endif
    filename += PLATFORM_SEPARATOR + JAVAS_FILE_NAME;
  }
  if (pathExists(filename, false))
  {
    outFile.open(filename, std::ios_base::in);
    if (outFile.fail())
    {
      std::cerr << "FATAL: can't open config file for reading at path " << filename << "."
        << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }
}

bool isJavaDir(const std::string &path)
{
  std::string bin = path + PLATFORM_SEPARATOR + "bin" + PLATFORM_SEPARATOR;
  return pathExists(path, true)
    && pathExists(bin + "java" + EXECUTABLE_SUFFIX, false)
    && pathExists(bin + "javac" + EXECUTABLE_SUFFIX, false);
}
