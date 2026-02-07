#include "Menu.h"
#include "../Vars.h"
#include "GUI.h"

/*
 * ToDo for this cheat.
 * NameStealer (cathook one?)
 * Sequence Freezing
 * CL_Sendmove Rebuild.. Not really needed..?
 * Charger Free turn/move turn.
 * AutoPistol
 */

void Menu::Init() {
  m_isVisible = false;

  CWindow Main(100, 100, 800, 500); // Increased height and width

  auto ESPTab = new CChild(270, 100, 250, L"ESP");
  {
    ESPTab->AddControl(
        new CCheckBox(L"Survivors", &Vars::ESP::Survivors.Enabled));
    ESPTab->AddControl(
        new CCheckBox(L"Common Infected", &Vars::ESP::Common.Enabled));
    ESPTab->AddControl(new CCheckBox(L"Special Infected",
                                     &Vars::ESP::SpecialInfected.Enabled));
    ESPTab->AddControl(new CCheckBox(L"Witch", &Vars::ESP::Witch.Enabled));
    ESPTab->AddControl(new CCheckBox(L"Tank", &Vars::ESP::Tank.Enabled));
  }
  Main.AddControl(ESPTab);

  auto Removals = new CChild(530, 10, 250, L"Removals");
  {
    Removals->AddControl(new CCheckBox(L"NoSpread", &Vars::Removals::NoSpread));
    Removals->AddControl(new CCheckBox(L"NoRecoil", &Vars::Removals::NoRecoil));
    Removals->AddControl(
        new CCheckBox(L"No Visual Recoil", &Vars::Removals::NoVisualRecoil));
  }
  Main.AddControl(Removals);

  auto MiscAgainTab = new CChild(530, 100, 250, L"Misc Base");
  {
    MiscAgainTab->AddControl(new CCheckBox(L"BunnyHop", &Vars::Misc::Bunnyhop));
    MiscAgainTab->AddControl(
        new CCheckBox(L"AutoStrafe", &Vars::Misc::AutoStrafe));
  }
  Main.AddControl(MiscAgainTab);

  MenuForm.AddWindow(Main);
}
void Menu::Kill() {}

void Menu::PaintTraverse() {
  if (m_isVisible)
    MenuForm.Paint();
}

void Menu::Click() {
  if (m_isVisible)
    MenuForm.Click();
}

void Menu::HandleInput(WPARAM vk) {
  if (m_isVisible)
    MenuForm.HandleInput(vk);
}

void Menu::Toggle() { m_isVisible = !m_isVisible; }