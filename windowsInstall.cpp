#ifndef UNIXISH

#include "windowsInstall.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

#define WIN32_LEAN_AND_MEAN
#undef UNICODE
#include <windows.h>

void windowsChangeInstall(const std::string &subcmd, const std::string &javasDir)
{
  DWORD pathVarBufLen;
  LSTATUS result;
  result = RegGetValue(
    HKEY_CURRENT_USER,
    "Environment",
    "Path",
    RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
    nullptr,
    nullptr,
    &pathVarBufLen
  );
  if (result != ERROR_SUCCESS)
  {
    std::cerr << "FATAL: " << subcmd << ": failed to get length of PATH registry value. Error "
      "code: " << result << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::string pathVar;
  pathVar.resize(pathVarBufLen);
  result = RegGetValue(
    HKEY_CURRENT_USER,
    "Environment",
    "Path",
    RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
    nullptr,
    pathVar.data(),
    &pathVarBufLen
  );
  if (result != ERROR_SUCCESS)
  {
    std::cerr << "FATAL: " << subcmd << ": failed to get PATH registry value. Error code: "
      << result << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::string::size_type firstNulPos = pathVar.find("\0", 0, 1);
  if (firstNulPos != std::string::npos)
  {
    // Delete trailing NULs
    pathVar.resize(firstNulPos);
  }
  std::size_t pathEntryPos = pathVar.find(javasDir);
  if (subcmd == "install")
  {
    if (pathEntryPos != std::string::npos)
    {
      std::cerr << "FATAL: install: Already found '" << javasDir << "' in PATH. If you're sure "
        "javas is not already installed, append the directory to PATH yourself." << std::endl;
      std::exit(EXIT_FAILURE);
    }
    pathVar += ';';
    pathVar += javasDir;
  }
  else
  {
    // Uninstall
    if (pathEntryPos != std::string::npos)
    {
      std::size_t eraseSize = javasDir.size();
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
      std::cerr << "FATAL: uninstall: Did not find '" << javasDir << "' in PATH. If you're sure "
        "javas is not already uninstalled, remove the .javas.d directory from PATH yourself."
        << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }
  result = RegSetKeyValue(
    HKEY_CURRENT_USER,
    "Environment",
    "Path",
    REG_EXPAND_SZ,
    pathVar.data(),
    pathVar.size() + 1
  );
  if (result != ERROR_SUCCESS)
  {
    std::cerr << "FATAL: " << subcmd << ": failed to set PATH registry value. Error code: "
      << result << std::endl;
    std::exit(EXIT_FAILURE);
  }
  const int USER_SETTING_CHANGE = 0;
  if (!SendMessageTimeout(
    HWND_BROADCAST,
    WM_SETTINGCHANGE,
    USER_SETTING_CHANGE,
    reinterpret_cast<LPARAM>("Environment"),
    SMTO_ABORTIFHUNG,
    5000,
    nullptr
  ))
  {
    std::cerr << "Failed to broadcast environment update message. Some programs may not recognize "
      "the change to PATH until this computer is restarted. Otherwise " << subcmd
      << "ed successfully." << std::endl;
  }
  else
  {
    std::cerr << "Successfully " << subcmd << "ed. You may need to open a new terminal window for "
      "the change to PATH to take effect." << std::endl;
  }
}

#endif // UNIXISH
