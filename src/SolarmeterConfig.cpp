#include <iostream>
#include <fstream>
#include <cstring>
#include "SolarmeterConfig.h"

bool SolarmeterConfig::Begin(const std::string &file)
{
  if (file.empty())
  {
    ErrorMessage = "Solarmeter config: Config argument empty.";
    return false;
  }
  std::ifstream ifs(file);
  if (!ifs)
  {
    ErrorMessage = std::string("Opening config file failed: ")
      + strerror(errno) + " (" + std::to_string(errno) + ")";
    return false;
  }
	std::string line;
	while (std::getline(ifs, line))
	{
		if (line.empty())
    {
			continue;
    }
    size_t pos = 0;
		if ((pos = line.find('#')) != std::string::npos)
    {
      line.erase(pos);
    }
    if ((pos = line.find_first_not_of(' ')) == std::string::npos)
    {
			continue;
    }
    std::string key, val;
    std::istringstream iss(line);
    std::getline(std::getline(iss >> std::ws, key, ' ') >> std::ws, val, ' ');
    if ((pos = val.find_first_not_of('\"')) != std::string::npos)
    {
      val.erase(0, pos);
    }
    if ((pos = val.find('\"')) != std::string::npos)
    {
      val.erase(pos);
    }
    KeyValuePair.insert(std::map<std::string, std::string>::value_type(key, val));
	}
	ifs.close();

  return true;
}

std::string SolarmeterConfig::GetErrorMessage(void) const
{
  return ErrorMessage;
}

void SolarmeterConfig::ShowConfig(void) const
{
  for(auto it = KeyValuePair.cbegin(); it != KeyValuePair.cend(); ++it)
  {
    std::cout << '{' << it->first << " => " << it->second << '}' << std::endl;
  }
}

bool SolarmeterConfig::KeyExists(const std::string &key)
{
  if (KeyValuePair.find(key) == KeyValuePair.cend())
  {
    ErrorMessage = std::string("Solarmeter config: Key \"") + key + "\" not found.";
    return false;
  }
  return true;
}

std::string SolarmeterConfig::GetValue(const std::string &key) const
{
  return KeyValuePair.find(key)->second;
}

bool SolarmeterConfig::ValidateKeys(std::set<std::string> valid_keys)
{
  for(auto it = KeyValuePair.cbegin(); it != KeyValuePair.cend(); ++it)
  {
    if ((valid_keys.find(it->first) == valid_keys.cend()))
    {
      ErrorMessage = std::string("Solarmeter config: Invalid key \"") + it->first + "\""; 
      return false;
    }
    if ((it->second).empty())
    {
      ErrorMessage = std::string("Solarmeter config: Key \"") + it->first + "\" has no value.";
      return false;
    }
  }
  return true;
}
