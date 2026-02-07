#pragma once
#include "../../SDK/SDK.h"

class CVisuals {
public:
  void OnFrame();

private:
  void RemoveVomit();
};

namespace F {
inline CVisuals Visuals;
}
