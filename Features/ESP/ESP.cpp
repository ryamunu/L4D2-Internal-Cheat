#include "ESP.h"
#include "../../SDK/EntityCache/entitycache.h"
#include "../../SDK/L4D2/Interfaces/BaseClientDLL.h"
#include "../../SDK/L4D2/Interfaces/ClientEntityList.h"
#include "../../SDK/L4D2/Interfaces/EngineClient.h"
#include "../../SDK/L4D2/Interfaces/EngineVGui.h"
#include "../../SDK/L4D2/Interfaces/ModelInfo.h"
#include "../../Util/Math/Math.h"
#include "../../Util/Util.h"
#include "../NewMenu/ImGui/imgui.h"
#include "../Vars.h"
#include <algorithm>

bool IsUselessBone(const char *name) {
  if (strstr(name, "lean") || strstr(name, "attachment") ||
      strstr(name, "phys") || strstr(name, "jiggle"))
    return true;
  return false;
}

const char *GetZombieName(int classId) {
  if (classId == G::ClassID.Hunter)
    return "Hunter";
  if (classId == G::ClassID.Smoker)
    return "Smoker";
  if (classId == G::ClassID.Boomer)
    return "Boomer";
  if (classId == G::ClassID.Jockey)
    return "Jockey";
  if (classId == G::ClassID.Spitter)
    return "Spitter";
  if (classId == G::ClassID.Charger)
    return "Charger";
  if (classId == G::ClassID.Tank)
    return "Tank";
  if (classId == G::ClassID.Witch)
    return "Witch";
  if (classId == G::ClassID.Infected)
    return "Infected";
  return "Infected";
}

bool CFeatures_ESP::GetBounds(C_BaseEntity *pBaseEntity, int &x, int &y, int &w,
                              int &h) {
  C_BaseAnimating *pAnim = pBaseEntity->As<C_BaseAnimating *>();
  if (pAnim) {
    ClientClass *pCC = pBaseEntity->GetClientClass();
    if (pCC && (pCC->m_ClassID == G::ClassID.CTerrorPlayer ||
                pCC->m_ClassID == G::ClassID.Infected ||
                pCC->m_ClassID == G::ClassID.Tank ||
                pCC->m_ClassID == G::ClassID.Witch)) {
      matrix3x4_t boneMatrices[128];
      if (pAnim->SetupBones(boneMatrices, 128, 0x100, I::GlobalVars->curtime)) {
        const model_t *pModel = pAnim->GetModel();
        if (pModel) {
          studiohdr_t *pHdr = I::ModelInfo->GetStudiomodel(pModel);
          if (pHdr && pHdr->numbones > 0) {
            float left = 10000, top = 10000, right = -10000, bottom = -10000;
            int count = 0;
            for (int i = 0; i < pHdr->numbones; i++) {
              mstudiobone_t *pBone = pHdr->pBone(i);
              if (!pBone || !(pBone->flags & 0x100))
                continue;

              Vector vBonePos, vScr;
              U::Math.VectorTransform(Vector(0, 0, 0), boneMatrices[i],
                                      vBonePos);
              if (G::Util.W2S(vBonePos, vScr)) {
                left = (std::min)(left, vScr.x);
                top = (std::min)(top, vScr.y);
                right = (std::max)(right, vScr.x);
                bottom = (std::max)(bottom, vScr.y);
                count++;
              }
            }
            if (count > 0) {
              x = (int)left - 3;
              y = (int)top - 3;
              w = (int)(right - left) + 6;
              h = (int)(bottom - top) + 6;
              return true;
            }
          }
        }
      }
    }
  }

  Vector vMin, vMax;
  pBaseEntity->GetRenderBounds(vMin, vMax);

  Vector vPoints[] = {
      Vector(vMin.x, vMin.y, vMin.z), Vector(vMin.x, vMax.y, vMin.z),
      Vector(vMax.x, vMax.y, vMin.z), Vector(vMax.x, vMin.y, vMin.z),
      Vector(vMax.x, vMax.y, vMax.z), Vector(vMin.x, vMax.y, vMax.z),
      Vector(vMin.x, vMin.y, vMax.z), Vector(vMax.x, vMin.y, vMax.z)};

  const matrix3x4_t &trans = pBaseEntity->RenderableToWorldTransform();

  float left = 10000, top = 10000, right = -10000, bottom = -10000;

  int count = 0;
  for (int i = 0; i < 8; i++) {
    Vector vWorld, vScr;
    U::Math.VectorTransform(vPoints[i], trans, vWorld);
    if (G::Util.W2S(vWorld, vScr)) {
      left = (std::min)(left, vScr.x);
      top = (std::min)(top, vScr.y);
      right = (std::max)(right, vScr.x);
      bottom = (std::max)(bottom, vScr.y);
      count++;
    }
  }

  if (count == 0)
    return false;

  x = (int)left;
  y = (int)top;
  w = (int)(right - left);
  h = (int)(bottom - top);

  // Clamp to screen bounds to prevent giant off-screen boxes
  auto clamp = [](int val, int low, int high) {
    return (val < low) ? low : (val > high ? high : val);
  };
  x = clamp(x, -500, G::Draw.m_nScreenW + 500);
  y = clamp(y, -500, G::Draw.m_nScreenH + 500);
  w = clamp(w, 0, G::Draw.m_nScreenW * 2);
  h = clamp(h, 0, G::Draw.m_nScreenH * 2);

  return true;
}

void CFeatures_ESP::DrawSkeleton(C_BaseEntity *pEntity, Color color) {
  C_BaseAnimating *pAnim = pEntity->As<C_BaseAnimating *>();
  if (pAnim) {
    matrix3x4_t boneMatrices[128];
    if (pAnim->SetupBones(boneMatrices, 128, 0x100, I::GlobalVars->curtime)) {
      const model_t *pModel = pAnim->GetModel();
      if (pModel) {
        studiohdr_t *pHdr = I::ModelInfo->GetStudiomodel(pModel);
        if (pHdr && pHdr->numbones > 0 && pHdr->numbones <= 128) {
          for (int i = 0; i < pHdr->numbones; i++) {
            mstudiobone_t *pBone = pHdr->pBone(i);
            if (!pBone || !(pBone->flags & 0x100) || pBone->parent == -1)
              continue;
            if (IsUselessBone(pBone->pszName()))
              continue;

            Vector vBonePos, vParentPos;
            U::Math.VectorTransform(Vector(0, 0, 0), boneMatrices[i], vBonePos);
            U::Math.VectorTransform(Vector(0, 0, 0),
                                    boneMatrices[pBone->parent], vParentPos);

            Vector vBoneScreen, vParentScreen;
            if (G::Util.W2S(vBonePos, vBoneScreen) &&
                G::Util.W2S(vParentPos, vParentScreen)) {
              G::Draw.Line((int)vBoneScreen.x, (int)vBoneScreen.y,
                           (int)vParentScreen.x, (int)vParentScreen.y, color);
            }
          }
        }
      }
    }
  }
}

void CFeatures_ESP::Render() {
  if (!I::EngineClient || !I::EngineVGui || !I::ClientEntityList)
    return;
  if (!I::EngineClient->IsInGame() || I::EngineVGui->IsGameUIVisible())
    return;
  if (!Vars::ESP::GlobalEnable)
    return;

  const int nLocalIndex = I::EngineClient->GetLocalPlayer();
  IClientEntity *pLocalEnt = I::ClientEntityList->GetClientEntity(nLocalIndex);
  if (!pLocalEnt)
    return;
  C_TerrorPlayer *pLocal = pLocalEnt->As<C_TerrorPlayer *>();

  auto ProcessGroup = [&](EGroupType groupType, auto &config) {
    if (!config.Enabled)
      return;
    const auto &entities = gEntityCache.GetGroup(groupType);
    for (const auto &pEntity : entities) {
      if (!pEntity || pEntity->IsDormant() ||
          pEntity->entindex() == nLocalIndex)
        continue;
      C_BaseEntity *pBaseEnt = pEntity->As<C_BaseEntity *>();
      if (!pBaseEnt || !pBaseEnt->IsAlive())
        continue;

      int x, y, w, h;
      if (!GetBounds(pBaseEnt, x, y, w, h))
        continue;

      Color drawCol = config.DrawColor;

      if (config.Box) {
        G::Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, {0, 0, 0, 255});
        G::Draw.OutlinedRect(x, y, w, h, drawCol);
        G::Draw.OutlinedRect(x + 1, y + 1, w - 2, h - 2, {0, 0, 0, 255});
      }

      if (config.Name) {
        const char *name =
            (groupType == EGroupType::CTERRORPLAYER)
                ? "Survivor"
                : GetZombieName(pEntity->GetClientClass()->m_ClassID);
        G::Draw.String(EFonts::ESP_NAME, x + (w / 2), y - 15,
                       {255, 255, 255, 255}, TXT_CENTERX, name);
      }

      if (config.Health) {
        int nHealth = pBaseEnt->GetHealth();
        int nMaxHealth = pBaseEnt->GetMaxHealth();
        if (nMaxHealth < 1)
          nMaxHealth = 100;
        float ratio = (float)nHealth / (float)nMaxHealth;
        G::Draw.Rect(x - 6, y - 1, 4, h + 2, Color(0, 0, 0, 200));
        G::Draw.Rect(x - 5, y + (h - (int)(h * ratio)), 2, (int)(h * ratio),
                     Color(0, 255, 0, 255));
      }

      if (groupType == EGroupType::TANK && Vars::ESP::Tank.Frustration) {
        C_TerrorPlayer *pTank = pBaseEnt->As<C_TerrorPlayer *>();
        if (pTank) {
          int frust = pTank->m_frustration();
          float ratio = (float)frust / 100.0f;
          int barW = 4, barH = h, barX = x + w + 10, barY = y;
          G::Draw.Rect(barX, barY - 1, barW, barH + 2, Color(0, 0, 0, 200));
          G::Draw.Rect(barX + 1, barY + (barH - (int)(barH * ratio)), barW - 2,
                       (int)(barH * ratio), Color(255, 150, 0, 255));
          G::Draw.String(EFonts::METER_THING, barX + 6,
                         barY + (barH - (int)(barH * ratio)),
                         {255, 255, 255, 255}, TXT_DEFAULT, "%d%%", frust);
        }
      }

      if (config.Skeleton)
        DrawSkeleton(pBaseEnt, drawCol);
    }
  };

  ProcessGroup(EGroupType::CTERRORPLAYER, Vars::ESP::Survivors);
  ProcessGroup(EGroupType::SPECIAL_INFECTED, Vars::ESP::SpecialInfected);
  ProcessGroup(EGroupType::INFECTED, Vars::ESP::Common);
  ProcessGroup(EGroupType::TANK, Vars::ESP::Tank);
  ProcessGroup(EGroupType::WITCH, Vars::ESP::Witch);

  // ITEM ESP
  if (Vars::ESP::Items::Enabled) {
    const auto &items = gEntityCache.GetGroup(EGroupType::ITEM);
    for (const auto &pEntity : items) {
      C_BaseEntity *pBase = pEntity->As<C_BaseEntity *>();
      if (!pBase || pBase->IsDormant() || pBase->m_hOwnerEntity().IsValid())
        continue;

      Vector screen;
      if (G::Util.W2S(pBase->WorldSpaceCenter(), screen)) {
        const char *itemName = "Item";
        const model_t *pModel = pBase->GetModel();
        if (pModel) {
          const char *modelName = I::ModelInfo->GetModelName(pModel);
          if (modelName) {
            if (strstr(modelName, "medkit"))
              itemName = "Medkit";
            else if (strstr(modelName, "pills"))
              itemName = "Pills";
            else if (strstr(modelName, "adrenaline"))
              itemName = "Adrenaline";
            else if (strstr(modelName, "m16"))
              itemName = "M16";
            else if (strstr(modelName, "ak47"))
              itemName = "AK47";
            else if (strstr(modelName, "shotgun"))
              itemName = "Shotgun";
            else if (strstr(modelName, "pistol"))
              itemName = "Pistol";
            else if (strstr(modelName, "molotov"))
              itemName = "Molotov";
            else if (strstr(modelName, "pipebomb"))
              itemName = "Pipe Bomb";
            else if (strstr(modelName, "vomitjar"))
              itemName = "Bile Jar";
            else if (strstr(modelName, "ammo"))
              itemName = "Ammo";
          }
        }
        G::Draw.String(EFonts::ESP_NAME, (int)screen.x, (int)screen.y,
                       {255, 255, 255, 255}, TXT_CENTERXY, itemName);
      }
    }
  }

  if (Vars::HUD::CustomCrosshair)
    DrawCrosshair();
}

void CFeatures_ESP::DrawCrosshair() {
  int x = G::Draw.m_nScreenW / 2, y = G::Draw.m_nScreenH / 2;
  int size = Vars::HUD::CrosshairSize, gap = Vars::HUD::CrosshairGap,
      thickness = Vars::HUD::CrosshairThickness;
  Color col = Vars::HUD::CrosshairColor;
  switch (Vars::HUD::CrosshairStyle) {
  case 0: // Cross
    G::Draw.Rect(x - (size / 2), y - (thickness / 2), size, thickness, col);
    G::Draw.Rect(x - (thickness / 2), y - (size / 2), thickness, size, col);
    break;
  case 3: // Gap Cross
    G::Draw.Rect(x - gap - size, y - (thickness / 2), size, thickness, col);
    G::Draw.Rect(x + gap, y - (thickness / 2), size, thickness, col);
    G::Draw.Rect(x - (thickness / 2), y - gap - size, thickness, size, col);
    G::Draw.Rect(x - (thickness / 2), y + gap, thickness, size, col);
    break;
  default:
    G::Draw.Rect(x - 2, y - 2, 4, 4, col);
    break;
  }
}

void CFeatures_ESP::DrawSpectatorList(C_TerrorPlayer *pLocal) {
  if (!pLocal || !Vars::HUD::SpectatorList)
    return;

  std::vector<std::string> spectators;
  for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++) {
    IClientEntity *pEnt = I::ClientEntityList->GetClientEntity(i);
    if (!pEnt || pEnt->IsDormant() || pEnt->entindex() == pLocal->entindex())
      continue;
    C_TerrorPlayer *pPlayer = pEnt->As<C_TerrorPlayer *>();
    if (!pPlayer || pPlayer->IsAlive())
      continue;
    CBaseHandle hObsTarget = pPlayer->m_hObserverTarget();
    if (I::ClientEntityList->GetClientEntityFromHandle(hObsTarget) ==
        (IClientEntity *)pLocal) {
      player_info_t pi;
      if (I::EngineClient->GetPlayerInfo(i, &pi))
        spectators.push_back(pi.name);
    }
  }

  if (spectators.empty())
    return;

  ImGui::SetNextWindowSize(ImVec2(150, 0), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Spectators", &Vars::HUD::SpectatorList,
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_AlwaysAutoResize)) {
    for (const auto &name : spectators) {
      ImGui::Text(name.c_str());
    }
    ImGui::End();
  }
}
