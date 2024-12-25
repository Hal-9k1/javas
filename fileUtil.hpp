#ifndef FILEUTIL_HPP
#define FILEUTIL_HPP
#include <string>
#include <fstream>

bool pathExists(const std::string &path, bool isDir);
void openConfFile(std::fstream &outFile, std::string &outUsedFilename);
bool isJavaDir(const std::string &path);

#endif
