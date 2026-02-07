#include "Visuals.h"
#include "../../SDK/L4D2/Interfaces/MaterialSystem.h"
#include "../Vars.h"

void CVisuals::OnFrame() { RemoveVomit(); }

// Production-ready optimized vomit removal with static material caching
void CVisuals::RemoveVomit() {
  // Static material pointer - FindMaterial called only once
  static IMaterial *pVomitOverlay = nullptr;
  static bool bInitialized = false;

  if (!bInitialized) {
    if (I::MaterialSystem) {
      pVomitOverlay = I::MaterialSystem->FindMaterial(
          "particle/screen_effect/screen_overlay_vomit", nullptr, false);
      if (pVomitOverlay && pVomitOverlay->IsErrorMaterial()) {
        pVomitOverlay = nullptr;
      }
    }
    bInitialized = true;
  }

  if (!pVomitOverlay)
    return;

  bool shouldRemove = Vars::Visuals::Removals::NoBoomerVomit;

  // Set NoDraw flag
  pVomitOverlay->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, shouldRemove);

  // Force alpha modulation for overlay effects
  if (shouldRemove) {
    pVomitOverlay->AlphaModulate(0.0f);
  } else {
    pVomitOverlay->AlphaModulate(1.0f);
  }
}
