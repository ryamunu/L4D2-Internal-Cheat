#pragma once
#include "../../SDK/SDK.h"
#include "../Vars.h"

class CMisc {
public:
  void run(C_TerrorPlayer *pLocal, CUserCmd *pCmd);
  void Bunnyhop(CUserCmd *pCmd, C_TerrorPlayer *pLocal);
  void AutoStrafe(CUserCmd *pCmd, C_TerrorPlayer *pLocal);
  void Thirdperson();

private:
  void movement(C_TerrorPlayer *pLocal, CUserCmd *pCmd);
};

namespace f {
inline CMisc *misc;
}