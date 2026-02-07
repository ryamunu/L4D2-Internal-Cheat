#include "Config.h"
#include <filesystem>
#include <fstream>

void CConfig::WriteInt(const char *section, const char *key, int value,
                       const char *file) {
  char buf[32];
  sprintf(buf, "%d", value);
  WritePrivateProfileStringA(section, key, buf, file);
}

void CConfig::WriteFloat(const char *section, const char *key, float value,
                         const char *file) {
  char buf[32];
  sprintf(buf, "%f", value);
  WritePrivateProfileStringA(section, key, buf, file);
}

void CConfig::WriteBool(const char *section, const char *key, bool value,
                        const char *file) {
  WritePrivateProfileStringA(section, key, value ? "1" : "0", file);
}

void CConfig::WriteColor(const char *section, const char *key, Color value,
                         const char *file) {
  char buf[64];
  sprintf(buf, "%d %d %d %d", value.r(), value.g(), value.b(), value.a());
  WritePrivateProfileStringA(section, key, buf, file);
}

int CConfig::ReadInt(const char *section, const char *key, int defaultVal,
                     const char *file) {
  return GetPrivateProfileIntA(section, key, defaultVal, file);
}

float CConfig::ReadFloat(const char *section, const char *key, float defaultVal,
                         const char *file) {
  char buf[32];
  GetPrivateProfileStringA(section, key, "0.0", buf, 32, file);
  return (float)atof(buf);
}

bool CConfig::ReadBool(const char *section, const char *key, bool defaultVal,
                       const char *file) {
  return GetPrivateProfileIntA(section, key, defaultVal, file) == 1;
}

Color CConfig::ReadColor(const char *section, const char *key, Color defaultVal,
                         const char *file) {
  char buf[64];
  GetPrivateProfileStringA(section, key, "", buf, 64, file);
  int r, g, b, a;
  if (sscanf(buf, "%d %d %d %d", &r, &g, &b, &a) == 4) {
    return {(byte)r, (byte)g, (byte)b, (byte)a};
  }
  return defaultVal;
}

void CConfig::Save(const char *name) {
  char path[MAX_PATH];
  sprintf(path, "C:\\zeniiware\\configs\\%s.ini", name);
  std::filesystem::create_directories("C:\\zeniiware\\configs");

  const char *f = path;

  // ESP
  WriteBool("ESP", "Survivors", Vars::ESP::Survivors.Enabled, f);
  WriteBool("ESP", "SurvivorsSkeleton", Vars::ESP::Survivors.Skeleton, f);
  WriteInt("ESP", "MaxDist", Vars::ESP::MaxDistance, f);

  // Visuals
  WriteBool("Visuals", "Thirdperson", Vars::Visuals::Thirdperson, f);
  WriteInt("Visuals", "TPDistance", Vars::Visuals::ThirdpersonDist, f);

  // Misc
  WriteBool("Misc", "Bhop", Vars::Misc::Bunnyhop, f);
  WriteBool("Misc", "AutoStrafe", Vars::Misc::AutoStrafe, f);

  // Aimbot
  WriteBool("Aimbot", "Enabled", Vars::Aimbot::Enabled, f);
  WriteBool("Aimbot", "Silent", Vars::Aimbot::Silent, f);
  WriteBool("Aimbot", "Smoothing", Vars::Aimbot::Smoothing, f);
  WriteFloat("Aimbot", "SmoothFactor", Vars::Aimbot::SmoothFactor, f);
  WriteFloat("Aimbot", "FOV", Vars::Aimbot::Fov, f);
  WriteInt("Aimbot", "Bone", Vars::Aimbot::Bone, f);
  WriteBool("Aimbot", "RSC", Vars::Aimbot::RSC, f);
  WriteFloat("Aimbot", "RSCX", Vars::Aimbot::RSCFactorRateX, f);
  WriteFloat("Aimbot", "RSCY", Vars::Aimbot::RSCFactorRateY, f);

  // Special Filters
  WriteBool("Filters", "Hunter", Vars::Aimbot::TargetSpecial::Hunter, f);
  WriteBool("Filters", "Smoker", Vars::Aimbot::TargetSpecial::Smoker, f);
  WriteBool("Filters", "Boomer", Vars::Aimbot::TargetSpecial::Boomer, f);
  WriteBool("Filters", "Jockey", Vars::Aimbot::TargetSpecial::Jockey, f);
  WriteBool("Filters", "Spitter", Vars::Aimbot::TargetSpecial::Spitter, f);
  WriteBool("Filters", "Charger", Vars::Aimbot::TargetSpecial::Charger, f);
  WriteBool("Filters", "IgnoreSurvivors", Vars::Aimbot::IgnoreSurvivors, f);

  // AutoShove
  WriteBool("AutoShove", "Enabled", Vars::AutoShove::Enabled, f);
}

void CConfig::Load(const char *name) {
  char path[MAX_PATH];
  sprintf(path, "C:\\zeniiware\\configs\\%s.ini", name);
  const char *f = path;

  if (!std::filesystem::exists(path))
    return;

  Vars::ESP::Survivors.Enabled = ReadBool("ESP", "Survivors", false, f);
  Vars::ESP::Survivors.Skeleton =
      ReadBool("ESP", "SurvivorsSkeleton", false, f);
  Vars::ESP::MaxDistance = ReadInt("ESP", "MaxDist", 5000, f);

  Vars::Visuals::Thirdperson = ReadBool("Visuals", "Thirdperson", false, f);
  Vars::Visuals::ThirdpersonDist = ReadInt("Visuals", "TPDistance", 120, f);

  Vars::Misc::Bunnyhop = ReadBool("Misc", "Bhop", false, f);
  Vars::Misc::AutoStrafe = ReadBool("Misc", "AutoStrafe", false, f);

  Vars::AutoShove::Enabled = ReadBool("AutoShove", "Enabled", false, f);

  // Aimbot
  Vars::Aimbot::Enabled = ReadBool("Aimbot", "Enabled", false, f);
  Vars::Aimbot::Silent = ReadBool("Aimbot", "Silent", false, f);
  Vars::Aimbot::Smoothing = ReadBool("Aimbot", "Smoothing", false, f);
  Vars::Aimbot::SmoothFactor = ReadFloat("Aimbot", "SmoothFactor", 5.0f, f);
  Vars::Aimbot::Fov = ReadFloat("Aimbot", "FOV", 10.0f, f);
  Vars::Aimbot::Bone = ReadInt("Aimbot", "Bone", 0, f);
  Vars::Aimbot::RSC = ReadBool("Aimbot", "RSC", true, f);
  Vars::Aimbot::RSCFactorRateX = ReadFloat("Aimbot", "RSCX", 100.0f, f);
  Vars::Aimbot::RSCFactorRateY = ReadFloat("Aimbot", "RSCY", 100.0f, f);
  Vars::Aimbot::IgnoreInfected = ReadBool("Aimbot", "IgnoreInfected", false, f);
  Vars::Aimbot::IgnoreSpecial = ReadBool("Aimbot", "IgnoreSpecial", false, f);

  // Special Filters
  Vars::Aimbot::TargetSpecial::Hunter = ReadBool("Filters", "Hunter", true, f);
  Vars::Aimbot::TargetSpecial::Smoker = ReadBool("Filters", "Smoker", true, f);
  Vars::Aimbot::TargetSpecial::Boomer = ReadBool("Filters", "Boomer", true, f);
  Vars::Aimbot::TargetSpecial::Jockey = ReadBool("Filters", "Jockey", true, f);
  Vars::Aimbot::TargetSpecial::Spitter =
      ReadBool("Filters", "Spitter", true, f);
  Vars::Aimbot::TargetSpecial::Charger =
      ReadBool("Filters", "Charger", true, f);
  Vars::Aimbot::IgnoreSurvivors =
      ReadBool("Filters", "IgnoreSurvivors", true, f);
}

void CConfig::Remove(const char *name) {
  char path[MAX_PATH];
  sprintf(path, "C:\\zeniiware\\configs\\%s.ini", name);
  if (std::filesystem::exists(path)) {
    std::filesystem::remove(path);
  }
}

std::vector<std::string> CConfig::GetConfigList() {
  std::vector<std::string> configs;
  if (!std::filesystem::exists("C:\\zeniiware\\configs"))
    return configs;

  for (const auto &entry :
       std::filesystem::directory_iterator("C:\\zeniiware\\configs")) {
    if (entry.path().extension() == ".ini") {
      configs.push_back(entry.path().stem().string());
    }
  }
  return configs;
}
