#include "unixish.hpp"
#include "consts.hpp"
#include "ConfData.hpp"
#include "fileUtil.hpp"
#include "res/help.txt.hpp"

#include <climits>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>

#ifndef UNIXISH
# include "windowsInstall.hpp"
#endif

void writeHelp()
{
  std::cerr.write(pResourceFile_res_help_txt, resourceLength_res_help_txt);
  std::cerr << std::endl;
}

void writeVersion()
{
  std::cerr << "javas " GIT_TAG " (commit " GIT_COMMIT ")" << std::endl
    << "Compiled for " COMPILE_TRIPLET " on " __DATE__ " " __TIME__ << std::endl;
}

int main(int argc, char **argv)
{
  int i;
  for (i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg.substr(0, CONF_OPT.size()) == CONF_OPT)
    {
      arg = "JAVAS=" + arg;
      putenv(arg.data());
    }
    else if (arg == HELP_OPT)
    {
      writeHelp();
      return EXIT_SUCCESS;
    }
    else if (arg == VERSION_OPT)
    {
      writeVersion();
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
  else if (subcmd == "version")
  {
    writeVersion();
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
    std::cerr << "No javas config file found, writing new one on exit." << std::endl;
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
    std::string path = argv[i++];
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
  }
  else if (subcmd == "install" || subcmd == "uninstall")
  {
    std::cerr << "FATAL: " << subcmd << " subcommand is not supported on"
      << " Unixish targets." << std::endl;
    return EXIT_FAILURE;
  }
#else
  else if (subcmd == "init")
  {
    std::cerr << "FATAL: init subcommand is not supported on Windows targets."
      << std::endl;
    return EXIT_FAILURE;
  }
  else if (subcmd == "install" || subcmd == "uninstall")
  {
    windowsChangeInstall(subcmd, javasDir);
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
    confData.makeCurrent(javasDir, name);
  }
  else if (confData.isEntry(subcmd))
  {
    confData.makeCurrent(javasDir, subcmd);
  }
  else
  {
    std::cerr << "FATAL: unknown subcommand '" << subcmd << "'. Try 'javas --help'."
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
