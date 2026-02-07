#include "ModelRender.h"
#include "../../Features/Vars.h"
using namespace Hooks;

void __fastcall ModelRender::ForcedMaterialOverride::Detour(
    void *ecx, void *edx, IMaterial *newMaterial,
    OverrideType_t nOverrideType) {
  Table.Original<FN>(Index)(ecx, edx, newMaterial, nOverrideType);
}

void OverridematerialXQZ(IMaterial *mat, float r, float g, float b) {
  mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
  mat->ColorModulate(r / 255, g / 255, b / 255);
  I::ModelRender->ForcedMaterialOverride(mat);
}

void Overridematerial(IMaterial *mat, float r, float g, float b) {
  mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
  mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
  mat->ColorModulate(r / 255, g / 255, b / 255);
  I::ModelRender->ForcedMaterialOverride(mat);
}

void __fastcall ModelRender::DrawModelExecute::Detour(
    void *ecx, void *edx, const DrawModelState_t &state,
    const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld) {
  if (!I::EngineClient->IsInGame())
    Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);

  // Use single reliable material for Chams
  static IMaterial *matChams = nullptr;

  static bool materialsInitialized = false;
  if (!materialsInitialized && I::MaterialSystem) {
    matChams = I::MaterialSystem->FindMaterial("debug/debugambientcube",
                                               "Model textures");
    materialsInitialized = true;
  }

  if (!matChams)
    matChams = I::MaterialSystem->FindMaterial("debug/debugambientcube",
                                               "Model textures");

  IMaterial *usedMat = matChams;

  // Style: 0 = Textured, 1 = Wireframe
  if (usedMat && !usedMat->IsErrorMaterial()) {
    usedMat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME,
                                (Vars::Chams::Style == 1));
    usedMat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
  }

  static IMaterial *vomitboomer = I::MaterialSystem->FindMaterial(
      ("particle/screenspaceboomervomit"), "Particle textures");

  if (pInfo.pModel && pInfo.entity_index && usedMat &&
      !usedMat->IsErrorMaterial()) {
    C_BaseEntity *pEntity =
        I::ClientEntityList->GetClientEntity(pInfo.entity_index)
            ->As<C_BaseEntity *>();
    if (pEntity && !pEntity->IsDormant() && pEntity->IsAlive()) {
      bool isPlayer = (pEntity->As<C_TerrorPlayer *>()->IsPlayer());
      bool isZombie = (pEntity->IsZombie());

      if ((Vars::Chams::Players && isPlayer) ||
          (Vars::Chams::Infected && isZombie)) {
        Color clrTeam = Vars::Chams::InfectedColor;
        if (isPlayer) {
          clrTeam = (pEntity->As<C_TerrorPlayer *>()->GetTeamNumber() ==
                     TEAM_SURVIVOR)
                        ? Vars::Chams::PlayerColor
                        : Vars::Chams::PlayerInfectedColor;
        }

        // XQZ PASS (Hidden items)
        OverridematerialXQZ(usedMat, clrTeam.r(), clrTeam.g(), clrTeam.b());
        Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);

        // NORMAL PASS (Visible items)
        Overridematerial(usedMat, clrTeam.r(), clrTeam.g(), clrTeam.b());
        Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);

        I::ModelRender->ForcedMaterialOverride(nullptr);
        return; // Important: Don't call original again below
      }
    }
  }

  Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
  I::ModelRender->ForcedMaterialOverride(nullptr);
}

void ModelRender::Init() {
  XASSERT(Table.Init(I::ModelRender) == false);
  XASSERT(Table.Hook(&ForcedMaterialOverride::Detour,
                     ForcedMaterialOverride::Index) == false);
  XASSERT(Table.Hook(&DrawModelExecute::Detour, DrawModelExecute::Index) ==
          false);
}