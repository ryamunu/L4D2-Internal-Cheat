#include "NetVarManager.h"

#include "../../SDK/L4D2/Interfaces/BaseClientDll.h"

inline int GetOffsetInternal(RecvTable *pTable, const char *const szVar) {
  for (int n = 0; n < pTable->GetNumProps(); n++) {
    RecvProp *pProp = pTable->GetProp(n);

    if (!pProp)
      continue;

    if (strcmp(szVar, pProp->GetName()) == 0)
      return pProp->GetOffset();

    RecvTable *pDataTable = pProp->GetDataTable();

    if (!pDataTable)
      continue;

    const int nOffset = GetOffsetInternal(pDataTable, szVar);

    if (nOffset)
      return (nOffset + pProp->GetOffset());
  }

  return 0;
}

int CUtil_NetVarManager::Get(const char *const szClass,
                             const char *const szVar) {
  ClientClass *pCC = I::BaseClient->GetAllClasses();

  // Stage 1: Search for specific class
  while (pCC) {
    const char *networkName = pCC->m_pNetworkName;
    const char *tableName = pCC->m_pRecvTable->GetName();

    bool match =
        (strcmp(szClass, networkName) == 0 || strcmp(szClass, tableName) == 0 ||
         (szClass[0] == 'C' && strcmp(szClass + 1, tableName + 3) == 0) ||
         strcmp(szClass, tableName + 3) == 0);

    if (match) {
      int nOffset = GetOffsetInternal(pCC->m_pRecvTable, szVar);
      if (nOffset)
        return nOffset;
    }

    pCC = pCC->m_pNext;
  }

  // Stage 2: Fallback - Search everything
  pCC = I::BaseClient->GetAllClasses();
  while (pCC) {
    int nOffset = GetOffsetInternal(pCC->m_pRecvTable, szVar);
    if (nOffset)
      return nOffset;

    pCC = pCC->m_pNext;
  }

  return 0;
}