#include "Visuals.h"
#include "../../SDK/L4D2/Interfaces/MaterialSystem.h"
#include "../Vars.h"

void CVisuals::OnFrame() { RemoveVomit(); }

// Standard optimized vomit removal - static material lookup (runs once)
// Why: Iterating ALL materials every frame kills FPS. Standard cheats cache the
// pointer.
void CVisuals::RemoveVomit() {
  // Static material pointers - FindMaterial called only ONCE
  static IMaterial *pVomitOverlay1 = nullptr;
  static IMaterial *pVomitOverlay2 = nullptr;
  static IMaterial *pVomitOverlay3 = nullptr;
  static bool bInitialized = false;

  if (!bInitialized && I::MaterialSystem) {
    // L4D2 vomit material names (verified from game files)
    pVomitOverlay1 = I::MaterialSystem->FindMaterial(
        "effects/screenoverlay_vomit", nullptr, false);
    pVomitOverlay2 = I::MaterialSystem->FindMaterial(
        "particle/screen_effect/screen_overlay_vomit", nullptr, false);
    pVomitOverlay3 = I::MaterialSystem->FindMaterial(
        "particle/screen_effect/screen_vomit", nullptr, false);

    // Clean up error materials
    if (pVomitOverlay1 && pVomitOverlay1->IsErrorMaterial())
      pVomitOverlay1 = nullptr;
    if (pVomitOverlay2 && pVomitOverlay2->IsErrorMaterial())
      pVomitOverlay2 = nullptr;
    if (pVomitOverlay3 && pVomitOverlay3->IsErrorMaterial())
      pVomitOverlay3 = nullptr;

    bInitialized = true;
  }

  bool shouldRemove = Vars::Visuals::Removals::NoBoomerVomit;

  auto ProcessMaterial = [shouldRemove](IMaterial *pMat) {
    if (!pMat)
      return;
    pMat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, shouldRemove);
    pMat->AlphaModulate(shouldRemove ? 0.0f : 1.0f);
  };

  ProcessMaterial(pVomitOverlay1);
  ProcessMaterial(pVomitOverlay2);
  ProcessMaterial(pVomitOverlay3);
}
