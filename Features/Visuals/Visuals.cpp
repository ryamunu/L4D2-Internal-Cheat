#include "Visuals.h"
#include "../../SDK/L4D2/Interfaces/MaterialSystem.h"
#include "../Vars.h"
#include <string>
#include <vector>

void CVisuals::OnFrame() { RemoveVomit(); }

void CVisuals::RemoveVomit() {
  if (!I::MaterialSystem)
    return;

  bool shouldRemove = Vars::Visuals::Removals::NoBoomerVomit;

  // Iterate through ALL loaded materials every frame for maximum reliability
  for (MaterialHandle_t h = I::MaterialSystem->FirstMaterial();
       h != I::MaterialSystem->InvalidMaterial();
       h = I::MaterialSystem->NextMaterial(h)) {

    IMaterial *pMat = I::MaterialSystem->GetMaterial(h);
    if (!pMat || pMat->IsErrorMaterial())
      continue;

    const char *matName = pMat->GetName();
    if (!matName)
      continue;

    // Check for ANY vomit/bile/slime related materials
    bool isVomitMaterial = false;
    if (strstr(matName, "vomit") || strstr(matName, "Vomit") ||
        strstr(matName, "bile") || strstr(matName, "Bile") ||
        strstr(matName, "slime") || strstr(matName, "Slime") ||
        strstr(matName, "spit") || strstr(matName, "Spit") ||
        strstr(matName, "boomer") || strstr(matName, "Boomer") ||
        strstr(matName, "screen_effect")) {
      isVomitMaterial = true;
    }

    if (isVomitMaterial) {
      pMat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, shouldRemove);
      if (shouldRemove) {
        pMat->AlphaModulate(0.0f);
        pMat->ColorModulate(0.0f, 0.0f, 0.0f);
      } else {
        pMat->AlphaModulate(1.0f);
        pMat->ColorModulate(1.0f, 1.0f, 1.0f);
      }
    }
  }
}
