class ConfData {
public:
  ConfData(std::string confFilename, std::fstream &confFile);
private:
  std::vector<VersionEntry> entries;
  std::string current;
}
