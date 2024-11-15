#if defined(__linux__) || defined(__CYGWIN__)
#  define UNIXISH
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

const std::string CONF_OPT = "--conf=";
const std::string HELP_OPT = "--help";

std::fstream &&openConfFile()
{
  std::string filename;
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
  std::fstream confFile(filename, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
  if (!confFile.good())
  {
    std::cerr << "FATAL: can't open config file at path " << filename << "." << std::endl;
  }
  return std::move(confFile);
}

int main(int argc, char **argv)
{
  for (int i = 0; i < argc; ++i)
  {
    const std::string &arg = argv[i];
    if (arg.substr(0, CONF_OPT.size()) == CONF_OPT)
    {
      setenv("JAVAS", arg.substr(CONF_OPT.size()));
    }
    else if (arg == HELP_OPT)
    {
      std::cerr << "Help text here." << std::endl;
      return 0;
    }
    else if (arg[0] == '-')
    {
      std::cerr << "Unrecognized option " << arg << "." << std::endl;
      return 1;
    }
    else
    {
      // End of global options.
      break;
    }
  }
  
  return 0;
}
