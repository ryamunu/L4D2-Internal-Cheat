#pragma once
#include "../SDK/L4D2/Includes/color.h"
#include "../SDK/SDK.h"

template <class T> class CVar {
public:
  T m_Var;
  const wchar_t *m_szDisplayName;
};

namespace Vars {
namespace Menu {
inline bool Opened = false;
inline Color Text = {255, 255, 255, 255};
inline Color WidgetActive = {255, 0, 50, 255};
inline Color ShadowColor = {20, 20, 20, 200};
inline int ShadowSize = 12;
} // namespace Menu

struct ESPConfig {
  bool Enabled = false;
  bool Box = false;
  bool Name = false;
  bool Health = false;
  bool NumericHP = false;
  bool Skeleton = false;
  bool Distance = false;
  bool Frustration = false;
  Color DrawColor = {255, 255, 255, 255};
};

namespace Aimbot {
inline bool Enabled = false;
inline bool Silent = false;
inline bool FriendlyFire = false;
inline bool AutoFire = false;
inline bool Prediction = true;
inline bool VisibilityCheck = true;
inline bool Multipoint = true;
inline float MultipointScale = 0.7f;
inline float Fov = 10.0f;
inline int Bone = 0; // Hitbox Index (0: Head, 1: Neck, 2: Chest, etc.)
inline int RcsFovLimit = 20;
inline bool Smoothing = false;
inline float SmoothFactor = 5.0f;
inline bool RSC = true;
inline float RSCFactorRateX = 100.0f;
inline float RSCFactorRateY = 100.0f;

// Filters
inline bool IgnoreSurvivors = true;
inline bool IgnoreInfected = false;
inline bool IgnoreSpecial = false;
inline bool IgnoreWitch = true;
inline bool IgnoreTank = false;

namespace TargetSpecial {
inline bool Hunter = true;
inline bool Smoker = true;
inline bool Boomer = true;
inline bool Jockey = true;
inline bool Spitter = true;
inline bool Charger = true;
} // namespace TargetSpecial

inline CVar<int> AimKey{VK_LBUTTON, L"Aim Key"};
} // namespace Aimbot

namespace ESP {
inline bool GlobalEnable = true;
inline int MaxDistance = 5000;
inline ESPConfig Survivors;
inline ESPConfig Common;
inline ESPConfig SpecialInfected;
inline ESPConfig Tank;
inline ESPConfig Witch;

namespace Items {
inline bool Enabled = true;
inline bool ShowWeapons = true;
inline bool ShowMedkits = true;
inline bool ShowPills = true;
inline bool ShowAdrenaline = true;
inline bool ShowDefib = true;
inline bool ShowProps = true;
inline int MaxItemDistance = 1500;
} // namespace Items

inline bool OutOfFOVArrows = false;

namespace Colors {
inline Color Survivors = {255, 255, 255, 255};
inline Color Common = {200, 200, 200, 255};
inline Color Special = {255, 100, 0, 255};
inline Color Boss = {255, 0, 0, 255};
} // namespace Colors
} // namespace ESP

namespace Visuals {
inline bool Thirdperson = false;
inline int ThirdpersonDist = 120;
inline float HeightOffset = 0.f;
inline bool ThirdpersonToggle = true;
inline float HorizontalOffset = 0.f;
inline float VerticalOffset = 0.f;

namespace Removals {
inline bool NoBoomerVomit = false;
inline bool NoVisualRecoil = false;
inline bool NoSpread = false;
inline bool DisablePostProcessing = false;
} // namespace Removals
} // namespace Visuals

namespace Misc {
inline bool Bunnyhop = false;
inline bool AutoStrafe = false;
inline int BhopChance = 100;
inline bool RankRevealer = false;
inline CVar<int> ThirdpersonKey{VK_XBUTTON1, L"TP Key"};
inline bool SequenceFreezing = false;
inline int SequenceFreezingValue = 1;
inline CVar<int> Key{0, L"SeqKey"};
inline bool Teleport = false;
inline CVar<int> TeleportKey{0, L"TpKey"};
inline bool DisableInterp = false;
inline bool EdgeJump = false;
inline bool DuckJump = false;
inline bool ShadowColorFix = false;
} // namespace Misc

namespace AutoShove {
inline bool Enabled = false;
inline bool Infected = true;
inline bool Specials = true;
inline bool Tank = false;
inline bool Spitter = true;

// Granular Specials
inline bool Hunter = true;
inline bool Smoker = true;
inline bool Jockey = true;
inline bool Charger = true;
inline bool Boomer = true;
} // namespace AutoShove

namespace Chams {
inline bool Players = false;
inline int Style = 0;
inline Color PlayerColor = {0, 255, 0, 255};
inline Color PlayerInfectedColor = {255, 0, 0, 255};
inline bool Infected = false;
inline Color InfectedColor = {255, 0, 0, 255};
} // namespace Chams

namespace HUD {
inline bool SpectatorList = true;
inline bool CustomCrosshair = false;
inline int CrosshairStyle = 0; // 0: Cross, 1: Circle, 2: Point, 3: Gap
inline int CrosshairSize = 10;
inline int CrosshairGap = 5;
inline int CrosshairThickness = 2;
inline Color CrosshairColor = {0, 255, 0, 255};
} // namespace HUD

namespace Exploits {
inline bool Speedhack = false;
inline CVar<int> Key{0, L"SpeedKey"};
inline int SpeedHackValue = 1;
} // namespace Exploits
} // namespace Vars