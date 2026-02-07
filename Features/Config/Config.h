#pragma once
#include "../Vars.h"
#include <string>
#include <vector>
#include <windows.h>

class CConfig {
public:
  void Save(const char *name);
  void Load(const char *name);
  void Remove(const char *name);
  std::vector<std::string> GetConfigList();

private:
  void WriteInt(const char *section, const char *key, int value,
                const char *file);
  void WriteFloat(const char *section, const char *key, float value,
                  const char *file);
  void WriteBool(const char *section, const char *key, bool value,
                 const char *file);
  void WriteColor(const char *section, const char *key, Color value,
                  const char *file);

  int ReadInt(const char *section, const char *key, int defaultVal,
              const char *file);
  float ReadFloat(const char *section, const char *key, float defaultVal,
                  const char *file);
  bool ReadBool(const char *section, const char *key, bool defaultVal,
                const char *file);
  Color ReadColor(const char *section, const char *key, Color defaultVal,
                  const char *file);
};

namespace f {
inline CConfig config;
}
