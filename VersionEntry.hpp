#include <iostream>
#include <string>

class VersionEntry {
public:
  VersionEntry(std::ifstream &reader);
  void write(std::ostream &writer);
private:
  const std::string name;
  const std::string path;
}
