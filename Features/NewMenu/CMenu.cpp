#include "CMenu.h"
#include "../../SDK/L4D2/Interfaces/IInput.h"
#include "../Config/Config.h"
#include "../ESP/ESP.h"
#include "../Vars.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_stdlib.h"
#include "font_awesome_5.h"
#include <cstring>
#include <map>
#include <string>
#include <vector>

struct UIState {
  float hover_alpha = 0.0f;
  float active_alpha = 0.0f;
};

static std::map<ImGuiID, UIState> g_UIStates;

static void UpdateUIState(ImGuiID id, bool hovered, bool active) {
  auto &state = g_UIStates[id];
  float target_hover = hovered ? 1.0f : 0.0f;
  float target_active = active ? 1.0f : 0.0f;

  state.hover_alpha += (target_hover - state.hover_alpha) * (1.0f / 0.15f) *
                       ImGui::GetIO().DeltaTime;
  state.active_alpha += (target_active - state.active_alpha) * (1.0f / 0.1f) *
                        ImGui::GetIO().DeltaTime;

  if (state.hover_alpha < 0.001f)
    state.hover_alpha = 0.0f;
  if (state.hover_alpha > 0.999f)
    state.hover_alpha = 1.0f;
}

static const ImVec4 C_Bg = ImColor(5, 5, 5, 255);
static const ImVec4 C_Topbar = ImColor(7, 7, 7, 255);
static const ImVec4 C_Child = ImColor(5, 5, 5, 255);

static const char *GetKeyName(int key) {
  if (key == 1)
    return "M1";
  if (key == 2)
    return "M2";
  if (key == 3)
    return "M3";
  if (key == 4)
    return "M4";
  if (key == 5)
    return "M5";
  static char buf[16];
  if (key >= 0x41 && key <= 0x5A) {
    sprintf(buf, "%c", (char)key);
    return buf;
  }
  switch (key) {
  case 0:
    return "None";
  case VK_SHIFT:
    return "Shift";
  case VK_CONTROL:
    return "Ctrl";
  case VK_MENU:
    return "Alt";
  case VK_SPACE:
    return "Space";
  case VK_TAB:
    return "Tab";
  default:
    sprintf(buf, "%d", key);
    return buf;
  }
}

static bool PremiumTab(const char *label, const char *icon, bool active) {
  ImGuiWindow *window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;
  const ImGuiID id = window->GetID(label);
  const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
  const ImVec2 icon_size = ImGui::CalcTextSize(icon, NULL, true);
  ImVec2 pos = window->DC.CursorPos;
  ImVec2 size = ImVec2(label_size.x + icon_size.x + 35, 35);
  const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
  ImGui::ItemSize(size, 0.0f);
  if (!ImGui::ItemAdd(bb, id))
    return false;
  bool hovered, held;
  bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
  UpdateUIState(id, hovered, active);
  float alpha = active ? 1.0f : (hovered ? 0.8f : 0.6f);
  ImU32 text_col = ImGui::GetColorU32(ImVec4(1, 1, 1, alpha));
  if (active)
    window->DrawList->AddRectFilled(ImVec2(bb.Min.x + 5, bb.Max.y - 2),
                                    ImVec2(bb.Max.x - 5, bb.Max.y),
                                    ImGui::GetColorU32(ImVec4(1, 0, 0.2f, 1)));
  float content_height = (std::max)(label_size.y, icon_size.y);
  float y_offset = (size.y - content_height) / 2.0f;
  ImGui::PushFont(g_Menu.icons);
  window->DrawList->AddText(ImVec2(bb.Min.x + 5, bb.Min.y + y_offset - 1),
                            text_col, icon);
  ImGui::PopFont();
  window->DrawList->AddText(
      ImVec2(bb.Min.x + 15 + icon_size.x, bb.Min.y + y_offset), text_col,
      label);
  return pressed;
}

static void MColorPicker(const char *label, Color &col) {
  float fCol[4] = {(float)col.r() / 255.0f, (float)col.g() / 255.0f,
                   (float)col.b() / 255.0f, (float)col.a() / 255.0f};
  if (ImGui::ColorEdit4(label, fCol,
                        ImGuiColorEditFlags_NoInputs |
                            ImGuiColorEditFlags_NoLabel |
                            ImGuiColorEditFlags_AlphaBar)) {
    col.SetColor((int)(fCol[0] * 255.0f), (int)(fCol[1] * 255.0f),
                 (int)(fCol[2] * 255.0f), (int)(fCol[3] * 255.0f));
  }
}

static void SectionHeader(const char *label) {
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.1f, 0.2f, 1.0f));
  ImGui::Text(label);
  ImGui::PopStyleColor();
  ImGui::Separator();
  ImGui::Spacing();
}

static void KeyBind(const char *label, int &key) {
  static bool waiting = false;
  static int *active_key = nullptr;
  if (waiting && active_key == &key) {
    if (ImGui::Button("[Waiting...]", ImVec2(100, 20))) {
      waiting = false;
      active_key = nullptr;
    }
    for (int i = 0; i < 5; i++)
      if (ImGui::IsMouseClicked(i)) {
        key = i + 1;
        waiting = false;
        active_key = nullptr;
        return;
      }
    for (int i = 0; i < 512; i++)
      if (ImGui::IsKeyPressed((ImGuiKey)i)) {
        key = i;
        waiting = false;
        active_key = nullptr;
        break;
      }
  } else {
    char buf[128];
    sprintf(buf, "%s: [%s]", label, GetKeyName(key));
    if (ImGui::Button(buf, ImVec2(120, 20))) {
      waiting = true;
      active_key = &key;
    }
  }
}

void TabAimbot_Premium() {
  ImGui::Columns(2, nullptr, false);
  ImGui::BeginChild("AimGeneral", ImVec2(0, 0), true);
  SectionHeader("General");
  ImGui::Checkbox("Enabled", &Vars::Aimbot::Enabled);
  ImGui::SameLine();
  KeyBind("Key", Vars::Aimbot::AimKey.m_Var);
  ImGui::Checkbox("Silent", &Vars::Aimbot::Silent);
  ImGui::Checkbox("Auto Fire", &Vars::Aimbot::AutoFire);
  ImGui::Checkbox("Prediction", &Vars::Aimbot::Prediction);
  ImGui::Checkbox("Visibility Check", &Vars::Aimbot::VisibilityCheck);
  ImGui::SliderFloat("FOV", &Vars::Aimbot::Fov, 1, 180, "%.0f");
  const char *hitboxes[] = {"Head", "Neck", "Chest", "Stomach", "Pelvis"};
  ImGui::Combo("Target", &Vars::Aimbot::Bone, hitboxes, IM_ARRAYSIZE(hitboxes));
  SectionHeader("Smooth & Precision");
  ImGui::Checkbox("Smoothing", &Vars::Aimbot::Smoothing);
  ImGui::SliderFloat("Smoothing Factor", &Vars::Aimbot::SmoothFactor, 1, 20,
                     "%.1f");
  ImGui::EndChild();
  ImGui::NextColumn();
  ImGui::BeginChild("AimFilters", ImVec2(0, 0), true);
  SectionHeader("RCS");
  ImGui::Checkbox("Enable RCS", &Vars::Aimbot::RSC);
  ImGui::SliderFloat("Vertical", &Vars::Aimbot::RSCFactorRateY, 0, 100,
                     "%.0f%%");
  ImGui::SliderFloat("Horizontal", &Vars::Aimbot::RSCFactorRateX, 0, 100,
                     "%.0f%%");
  SectionHeader("Filters");
  ImGui::Checkbox("Ignore Survivors", &Vars::Aimbot::IgnoreSurvivors);
  ImGui::Checkbox("Ignore Infected", &Vars::Aimbot::IgnoreInfected);
  if (ImGui::TreeNode("Special Infected Targets")) {
    ImGui::Checkbox("Hunter", &Vars::Aimbot::TargetSpecial::Hunter);
    ImGui::Checkbox("Smoker", &Vars::Aimbot::TargetSpecial::Smoker);
    ImGui::Checkbox("Boomer", &Vars::Aimbot::TargetSpecial::Boomer);
    ImGui::Checkbox("Jockey", &Vars::Aimbot::TargetSpecial::Jockey);
    ImGui::Checkbox("Spitter", &Vars::Aimbot::TargetSpecial::Spitter);
    ImGui::Checkbox("Charger", &Vars::Aimbot::TargetSpecial::Charger);
    ImGui::TreePop();
  }
  ImGui::Checkbox("Ignore Tank", &Vars::Aimbot::IgnoreTank);
  ImGui::Checkbox("Ignore Witch", &Vars::Aimbot::IgnoreWitch);
  ImGui::EndChild();
  ImGui::Columns(1);
}

void CMenu::Render(IDirect3DDevice9 *pDevice) {
  static bool bInit = false;
  if (!bInit) {
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(FindWindowA("Valve001", nullptr));
    ImGui_ImplDX9_Init(pDevice);
    ImGuiIO &io = ImGui::GetIO();
    font = io.Fonts->AddFontFromMemoryTTF((void *)MuseoFont, sizeof(MuseoFont),
                                          15.0f);
    name = io.Fonts->AddFontFromMemoryTTF((void *)MuseoFont, sizeof(MuseoFont),
                                          22.0f);
    ImFontConfig cfg;
    cfg.MergeMode = true;
    cfg.PixelSnapH = true;
    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    icons = io.Fonts->AddFontFromMemoryTTF(
        (void *)fa_solid_900, sizeof(fa_solid_900), 16.0f, &cfg, icon_ranges);
    ImGuiStyle &s = ImGui::GetStyle();
    s.WindowPadding = ImVec2(0, 0);
    s.WindowRounding = 0;
    s.ChildRounding = 0;
    s.FrameRounding = 2;
    s.PopupRounding = 2;
    s.ScrollbarRounding = 0;
    s.Colors[ImGuiCol_WindowBg] = C_Bg;
    s.Colors[ImGuiCol_ChildBg] = C_Child;
    s.Colors[ImGuiCol_Border] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);
    s.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    s.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    s.Colors[ImGuiCol_CheckMark] = ImVec4(1, 1, 1, 1);
    s.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    s.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    bInit = true;
  }

  if (!Vars::HUD::SpectatorList && !Vars::Menu::Opened)
    return;

  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  if (Vars::HUD::SpectatorList) {
    IClientEntity *pLocalEnt =
        I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
    if (pLocalEnt) {
      C_TerrorPlayer *pLocal = pLocalEnt->As<C_TerrorPlayer *>();
      if (pLocal)
        F::ESP.DrawSpectatorList(pLocal);
    }
  }

  if (Vars::Menu::Opened) {
    ImGui::SetNextWindowSize(ImVec2(850, 550), ImGuiCond_Once);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin("zeniiware_v2", &Vars::Menu::Opened,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
      static int activeTab = 0;
      ImGui::PushStyleColor(ImGuiCol_ChildBg, C_Topbar);
      ImGui::BeginChild("TopBar", ImVec2(0, 45), false);
      ImGui::SetCursorPos(ImVec2(15, 10));
      ImGui::PushFont(name);
      ImGui::TextColored(ImVec4(0.85f, 0.1f, 0.2f, 1.0f), "zenii");
      ImGui::SameLine(0, 0);
      ImGui::TextColored(ImVec4(1, 1, 1, 1), "ware");
      ImGui::PopFont();
      ImGui::SameLine(180);
      if (PremiumTab("Aimbot", ICON_FA_CROSSHAIRS, activeTab == 0))
        activeTab = 0;
      ImGui::SameLine(0, 0);
      if (PremiumTab("ESP", ICON_FA_EYE, activeTab == 1))
        activeTab = 1;
      ImGui::SameLine(0, 0);
      if (PremiumTab("Items", ICON_FA_CHART_BAR, activeTab == 2))
        activeTab = 2;
      ImGui::SameLine(0, 0);
      if (PremiumTab("Visuals", ICON_FA_PALETTE, activeTab == 3))
        activeTab = 3;
      ImGui::SameLine(0, 0);
      if (PremiumTab("HUD", ICON_FA_MOBILE_ALT, activeTab == 5))
        activeTab = 5;
      ImGui::SameLine(0, 0);
      if (PremiumTab("Misc", ICON_FA_COGS, activeTab == 6))
        activeTab = 6;
      ImGui::SameLine(0, 0);
      if (PremiumTab("Config", ICON_FA_SAVE, activeTab == 7))
        activeTab = 7;
      ImGui::EndChild();
      ImGui::PopStyleColor();

      ImGui::SetCursorPos(ImVec2(10, 55));
      ImGui::BeginChild("Content", ImVec2(830, 485), false);
      if (activeTab == 0)
        TabAimbot_Premium();
      else if (activeTab == 1) { // ESP
        ImGui::Columns(3, nullptr, false);
        ImGui::BeginChild("ESP_Surv", ImVec2(0, 0), true);
        SectionHeader("Survivors");
        ImGui::Checkbox("Enabled##S", &Vars::ESP::Survivors.Enabled);
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
        MColorPicker("##ColS", Vars::ESP::Survivors.DrawColor);
        ImGui::Checkbox("Box##S", &Vars::ESP::Survivors.Box);
        ImGui::Checkbox("Skeleton##S", &Vars::ESP::Survivors.Skeleton);
        ImGui::Checkbox("Name##S", &Vars::ESP::Survivors.Name);
        ImGui::Checkbox("Health##S", &Vars::ESP::Survivors.Health);
        ImGui::Checkbox("Distance##S", &Vars::ESP::Survivors.Distance);
        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("ESP_Inf", ImVec2(0, 0), true);
        SectionHeader("Common Infected");
        ImGui::Checkbox("Enabled##C", &Vars::ESP::Common.Enabled);
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
        MColorPicker("##ColC", Vars::ESP::Common.DrawColor);
        ImGui::Checkbox("Box##C", &Vars::ESP::Common.Box);
        ImGui::Checkbox("Skeleton##C", &Vars::ESP::Common.Skeleton);
        ImGui::Checkbox("Name##C", &Vars::ESP::Common.Name);
        ImGui::Checkbox("Health##C", &Vars::ESP::Common.Health);
        ImGui::Spacing();
        SectionHeader("Special Infected");
        ImGui::Checkbox("Enabled##SI", &Vars::ESP::SpecialInfected.Enabled);
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
        MColorPicker("##ColSI", Vars::ESP::SpecialInfected.DrawColor);
        ImGui::Checkbox("Box##SI", &Vars::ESP::SpecialInfected.Box);
        ImGui::Checkbox("Skeleton##SI", &Vars::ESP::SpecialInfected.Skeleton);
        ImGui::Checkbox("Name##SI", &Vars::ESP::SpecialInfected.Name);
        ImGui::Checkbox("Health##SI", &Vars::ESP::SpecialInfected.Health);
        ImGui::Checkbox("Distance##SI", &Vars::ESP::SpecialInfected.Distance);
        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("ESP_Boss", ImVec2(0, 0), true);
        SectionHeader("Bosses (Tank/Witch)");
        ImGui::Checkbox("Tank Enabled", &Vars::ESP::Tank.Enabled);
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
        MColorPicker("##ColT", Vars::ESP::Tank.DrawColor);
        ImGui::Checkbox("Box##T", &Vars::ESP::Tank.Box);
        ImGui::Checkbox("Skeleton##T", &Vars::ESP::Tank.Skeleton);
        ImGui::Checkbox("Frustration", &Vars::ESP::Tank.Frustration);
        ImGui::Spacing();
        ImGui::Checkbox("Witch Enabled", &Vars::ESP::Witch.Enabled);
        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
        MColorPicker("##ColW", Vars::ESP::Witch.DrawColor);
        ImGui::Checkbox("Box##W", &Vars::ESP::Witch.Box);
        ImGui::Checkbox("Skeleton##W", &Vars::ESP::Witch.Skeleton);
        ImGui::EndChild();
        ImGui::Columns(1);
      } else if (activeTab == 2) { // ITEMS
        ImGui::Columns(2, nullptr, false);
        ImGui::BeginChild("Items1", ImVec2(0, 0), true);
        SectionHeader("World ESP");
        ImGui::Checkbox("Items Enabled", &Vars::ESP::Items::Enabled);
        ImGui::SliderInt("Max Distance", &Vars::ESP::Items::MaxItemDistance, 0,
                         3000);
        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("Items2", ImVec2(0, 0), true);
        SectionHeader("Filters");
        ImGui::Checkbox("Weapons", &Vars::ESP::Items::ShowWeapons);
        ImGui::Checkbox("Medkits", &Vars::ESP::Items::ShowMedkits);
        ImGui::Checkbox("Pills", &Vars::ESP::Items::ShowPills);
        ImGui::Checkbox("Adrenaline", &Vars::ESP::Items::ShowAdrenaline);
        ImGui::Checkbox("Defib", &Vars::ESP::Items::ShowDefib);
        ImGui::Checkbox("Props", &Vars::ESP::Items::ShowProps);
        ImGui::EndChild();
        ImGui::Columns(1);
      } else if (activeTab == 3) { // VISUALS
        ImGui::Columns(2, nullptr, false);
        ImGui::BeginChild("Vis1", ImVec2(0, 0), true);
        SectionHeader("View");
        ImGui::Checkbox("Thirdperson", &Vars::Visuals::Thirdperson);
        ImGui::SameLine();
        KeyBind("Key", Vars::Misc::ThirdpersonKey.m_Var);
        ImGui::SliderInt("Distance", &Vars::Visuals::ThirdpersonDist, 30, 250);
        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("Vis2", ImVec2(0, 0), true);
        SectionHeader("Removals");
        ImGui::Checkbox("No Boomer Vomit",
                        &Vars::Visuals::Removals::NoBoomerVomit);
        ImGui::Checkbox("No Visual Recoil",
                        &Vars::Visuals::Removals::NoVisualRecoil);
        ImGui::Checkbox("No Spread", &Vars::Visuals::Removals::NoSpread);
        ImGui::Checkbox("Disable Post Process",
                        &Vars::Visuals::Removals::DisablePostProcessing);
        ImGui::EndChild();
        ImGui::Columns(1);
      } else if (activeTab == 5) { // HUD
        ImGui::Columns(2, nullptr, false);
        ImGui::BeginChild("HUD1", ImVec2(0, 0), true);
        SectionHeader("Informational");
        ImGui::Checkbox("Spectator List", &Vars::HUD::SpectatorList);
        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("HUD2", ImVec2(0, 0), true);
        SectionHeader("Crosshair");
        ImGui::Checkbox("Custom Crosshair", &Vars::HUD::CustomCrosshair);
        ImGui::SliderInt("Size", &Vars::HUD::CrosshairSize, 2, 50);
        ImGui::SliderInt("Gap", &Vars::HUD::CrosshairGap, 0, 20);
        ImGui::SliderInt("Thickness", &Vars::HUD::CrosshairThickness, 1, 5);
        MColorPicker("Color##CH", Vars::HUD::CrosshairColor);
        const char *styles[] = {"Cross", "Circle", "Point", "Gap Cross"};
        ImGui::Combo("Style", &Vars::HUD::CrosshairStyle, styles,
                     IM_ARRAYSIZE(styles));
        ImGui::EndChild();
        ImGui::Columns(1);
      } else if (activeTab == 6) { // MISC
        ImGui::Columns(2, nullptr, false);
        ImGui::BeginChild("Misc1", ImVec2(0, 0), true);
        SectionHeader("Movement");
        ImGui::Checkbox("Bunnyhop", &Vars::Misc::Bunnyhop);
        ImGui::SliderInt("Bhop Chance", &Vars::Misc::BhopChance, 1, 100);
        ImGui::Checkbox("Auto Strafe", &Vars::Misc::AutoStrafe);
        ImGui::Checkbox("Edge Jump", &Vars::Misc::EdgeJump);
        ImGui::Checkbox("Duck Jump", &Vars::Misc::DuckJump);
        SectionHeader("Other");
        ImGui::Checkbox("Rank Revealer", &Vars::Misc::RankRevealer);
        ImGui::EndChild();
        ImGui::NextColumn();
        ImGui::BeginChild("Misc2", ImVec2(0, 0), true);
        SectionHeader("Auto Shove");
        ImGui::Checkbox("Enabled##AS", &Vars::AutoShove::Enabled);
        ImGui::Checkbox("Common Infected", &Vars::AutoShove::Infected);
        ImGui::Checkbox("Tank", &Vars::AutoShove::Tank);
        if (ImGui::TreeNode("Special Infected Filters")) {
          ImGui::Checkbox("Hunter", &Vars::AutoShove::Hunter);
          ImGui::Checkbox("Smoker", &Vars::AutoShove::Smoker);
          ImGui::Checkbox("Jockey", &Vars::AutoShove::Jockey);
          ImGui::Checkbox("Charger", &Vars::AutoShove::Charger);
          ImGui::Checkbox("Boomer", &Vars::AutoShove::Boomer);
          ImGui::Checkbox("Spitter", &Vars::AutoShove::Spitter);
          ImGui::TreePop();
        }
        SectionHeader("Exploits");
        ImGui::Checkbox("Speedhack", &Vars::Exploits::Speedhack);
        ImGui::SameLine();
        KeyBind("Key##SH", Vars::Exploits::Key.m_Var);
        ImGui::SliderInt("Factor##SH", &Vars::Exploits::SpeedHackValue, 1, 25);
        ImGui::Checkbox("Teleport", &Vars::Misc::Teleport);
        ImGui::SameLine();
        KeyBind("Key##TP", Vars::Misc::TeleportKey.m_Var);
        ImGui::Checkbox("Sequence Freezing", &Vars::Misc::SequenceFreezing);
        ImGui::SliderInt("Value##SF", &Vars::Misc::SequenceFreezingValue, 1,
                         100);
        ImGui::EndChild();
        ImGui::Columns(1);
      } else if (activeTab == 7) { // CONFIG
        static char cfgBur[64] = "default";
        ImGui::InputText("Configuration Name", cfgBur, 64);
        if (ImGui::Button("Save Configuration", ImVec2(200, 30)))
          f::config.Save(cfgBur);
        if (ImGui::Button("Load Configuration", ImVec2(200, 30)))
          f::config.Load(cfgBur);
        ImGui::Separator();
        SectionHeader("Existing Configurations");
        const auto &configs = f::config.GetConfigList();
        for (const auto &cfg : configs) {
          if (ImGui::Selectable(cfg.c_str(), !strcmp(cfgBur, cfg.c_str())))
            strncpy(cfgBur, cfg.c_str(), 63);
        }
      }
      ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }

  ImGui::Render();
  ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}
