#if defined(__linux__) || defined(__CYGWIN__)
#  define UNIXISH
#endif

#include "ConfData.hpp"
#include "res/help.txt.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const std::string CONF_OPT = "--conf=";
const std::string HELP_OPT = "--help";

void writeHelp()
{
  std::cerr << _binary_res_help_txt_end - _binary_res_help_txt_start << std::endl;
  std::cerr << _binary_res_help_txt_end << " - " << _binary_res_help_txt_start << std::endl;
  std::cerr.write(_binary_res_help_txt_start,
    _binary_res_help_txt_end - _binary_res_help_txt_start);
  std::cerr << std::endl;
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
      filename += getenv("HOMEDRIVE") || "";
      filename += getenv("HOMEPATH") || "";
    }
#endif
    filename += ".javas";
  }
  outFile.open(filename, std::ios_base::in | std::ios_base::out | std::ios_base::ate);
  if (!outFile.good())
  {
    std::cerr << "FATAL: can't open config file for writing at path " << filename << "."
      << std::endl;
  }
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
      return 0;
    }
    else if (arg[0] == '-')
    {
      std::cerr << "FATAL: Unrecognized option " << arg << "." << std::endl;
      return 1;
    }
    else
    {
      // End of global options.
      break;
    }
  }
  if (i == argc)
  {
    std::cerr << "Missing subcommand. Try 'javas --help'." << std::endl;
    return 1;
  }
  const std::string subcmd = argv[i];
  if (subcmd == "help")
  {
    writeHelp();
    return 0;
  }
  else
  {
    std::cerr << "Unknown subcommand '" << subcmd << "'. Try 'javas --help'." << std::endl;
    return 1;
  }
  std::string confFilename;
  std::fstream confFile;
  openConfFile(confFile, confFilename);
  ConfData confData(confFilename, confFile);

  return 0;
}
