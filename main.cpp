#if (defined(__linux__) || defined(__CYGWIN__)) && !defined(FORCE_WINDOWS)
#  define UNIXISH
#endif

#include "ConfData.hpp"
#include "res/help.txt.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#ifndef UNIXISH
#  define WIN32_LEAN_AND_MEAN
#  include <winreg.h>
#endif

const std::string CONF_OPT = "--conf=";
const std::string HELP_OPT = "--help";
const std::string JAVAS_FILE_NAME = ".javas";
const std::string JAVAS_DIR_SUFFIX = ".d";
#ifdef UNIXISH
const std::string PLATFORM_SEPARATOR = "/";
const std::string EXECUTABLE_SUFFIX = "";
#else
const std::string PLATFORM_SEPARATOR = "\\";
const std::string EXECUTABLE_SUFFIX = ".exe";
#endif

void writeHelp()
{
  std::cerr << resourceLength_res_help_txt << std::endl;
  std::cerr.write(pResourceFile_res_help_txt, resourceLength_res_help_txt);
  std::cerr << std::endl;
}

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
  const char *pFilename = getenv("JAVAS");
  if (pFilename)
  {
    filename = std::string(pFilename);
  }
  else
  {
#ifdef UNIXISH
    filename = std::string(getenv("HOME"));
#else
    const char *pFilename = getenv("USERPROFILE");
    if (pFilename)
    {
      filename = std::string(pFilename);
    }
    else
    {
      const char *var = getenv("HOMEDRIVE");
      if (var)
      {
        filename += var;
      }
      var = getenv("HOMEPATH");
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

int main(int argc, char **argv)
{
  int i;
  for (i = 1; i < argc; ++i)
  {
    const std::string arg = argv[i];
    if (arg.substr(0, CONF_OPT.size()) == CONF_OPT)
    {
      setenv("JAVAS", arg.substr(CONF_OPT.size()).c_str(), 0);
    }
    else if (arg == HELP_OPT)
    {
      writeHelp();
      return EXIT_SUCCESS;
    }
    else if (arg[0] == '-')
    {
      std::cerr << "FATAL: Unrecognized option " << arg << "." << std::endl;
      return EXIT_FAILURE;
    }
    else
    {
      // End of global options.
      break;
    }
  }
  if (i == argc)
  {
    std::cerr << "FATAL: Missing subcommand. Try 'javas --help'." << std::endl;
    return EXIT_FAILURE;
  }
  const std::string subcmd = argv[i++];
  if (subcmd == "help")
  {
    writeHelp();
    return EXIT_SUCCESS;
  }
  std::string confFilename;
  std::fstream confFile;
  openConfFile(confFile, confFilename);
  ConfData confData;
  if (confFile.is_open())
  {
    confData.read(confFilename, confFile);
  }
  else
  {
    std::cerr << "No javas config file found, writing new one on exit" << std::endl;
  }
  std::string javasDir = confFilename + JAVAS_DIR_SUFFIX;
  if (subcmd == "add")
  {
    if (i == argc)
    {
      std::cerr << "FATAL: add: missing version name." << std::endl;
      return EXIT_FAILURE;
    }
    const std::string name = argv[i++];
    if (i == argc)
    {
      std::cerr << "FATAL: add: missing version path." << std::endl;
      return EXIT_FAILURE;
    }
    const std::string path = argv[i++];
    if (!isJavaDir(path))
    {
      std::cerr << "FATAL: add: " << path << " does not appear to be the root"
        << " directory of a java installation." << std::endl;
      return EXIT_FAILURE;
    }
    confData.addEntry(name, path);
  }
  else if (subcmd == "ls" || subcmd == "list")
  {
    confData.listEntries(false);
  }
  else if (subcmd == "rm" || subcmd == "remove")
  {
    if (i == argc)
    {
      std::cerr << "FATAL: " << subcmd << ": missing version name." << std::endl;
      return EXIT_FAILURE;
    }
    const std::string name = argv[i++];
    confData.removeEntry(name);
  }
  else if (subcmd == "current")
  {
    confData.listEntries(true);
  }
#ifdef UNIXISH
  else if (subcmd == "init")
  {
    std::cout << "PATH=\"" << javasDir << ":$PATH\"" << std::endl;
    return EXIT_SUCCESS;
  }
#else
  else if (subcmd == "install" || subcmd == "uninstall")
  {
    unsigned int pathVarBufLen;
    RegGetValueA(
      HKEY_CURRENT_USER,
      "Environment",
      "PATH",
      RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
      nullptr,
      nullptr,
      &pathVarBufLen
    );
    std::string pathVar;
    pathVar.resize(pathVarBufLen);
    RegGetValueA(
      HKEY_CURRENT_USER,
      "Environment",
      "PATH",
      RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
      nullptr,
      pathVar.data(),
      &pathVarBufLen
    );
    if (subcmd == "install")
    {
      pathVar = ';';
      pathVar += javasDir;
    }
    else
    {
      std::size_t pathEntryPos = pathVar.find(javasDir);
      std::size_t eraseSize = javasDir.size();
      if (pathEntryPos != std::string::npos)
      {
        if (pathEntryPos)
        {
          // Delete leading semicolon
          --pathEntryPos;
          ++eraseSize;
        }
        if (pathEntryPos + javasDir.size() != pathVar.size())
        {
          // Delete trailing semicolon
          ++eraseSize;
        }
        pathVar.erase(pathEntryPos, eraseSize); 
      }
      else
      {
        std::cerr << "Did not find '" << javasDir << "' in PATH. If you're sure javas is not"
          << " already uninstalled, remove the .javas.d directory from PATH yourself."
          << std::endl;
        return EXIT_FAILURE;
      }
    }
    RegSetKeyValueA(
      HKEY_CURRENT_USER,
      "Environment",
      "PATH",
      REG_EXPAND_SZ,
      pathVar.data(),
      pathVar.size() + 1
    );
  }
#endif
  else if (subcmd == "switch")
  {
    if (i == argc)
    {
      std::cerr << "FATAL: switch: missing version name." << std::endl;
      return EXIT_FAILURE;
    }
    const std::string name = argv[i++];
    confData.makeCurrent(name);
  }
  else if (confData.isEntry(subcmd))
  {
    confData.makeCurrent(subcmd);
  }
  else
  {
    std::cerr << "FATAL: Unknown subcommand '" << subcmd << "'. Try 'javas --help'."
      << std::endl;
    return EXIT_FAILURE;
  }
  confFile.close();
  confFile.open(confFilename, std::ios::out | std::ios::trunc);
  if (confFile.fail())
  {
    std::cerr << "FATAL: can't open config file for writing at path " << confFilename
      << "." << std::endl;
    return EXIT_FAILURE;
  }
  confData.write(confFile);
  confFile.close();

  return EXIT_SUCCESS;
}
