#include "StdAfx.h"
#include "CNodeUIBase.h"
#include "CNodeUIManager.h"

namespace DuiLib {

IMPLEMENT_DUICONTROL(CNodeUIBase)

CNodeUIBase::CNodeUIBase()
    : bRegistered_(false)
{
    SetMouseChildEnabled(true);
}

CNodeUIBase::~CNodeUIBase()
{
    if (bRegistered_)
    {
        CNodeUIManager::GetInstance().UnregisterNode(this);
        bRegistered_ = false;
    }
}

LPCTSTR CNodeUIBase::GetClass() const
{
    return _T("CNodeUIBase");
}

LPVOID CNodeUIBase::GetInterface(LPCTSTR pstrName)
{
    if (_tcscmp(pstrName, _T("CNodeUIBase")) == 0) return static_cast<CNodeUIBase*>(this);
    return CContainerUI::GetInterface(pstrName);
}

void CNodeUIBase::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    CPaintManagerUI* oldManager = GetManager();
    CContainerUI::SetManager(pManager, pParent, bInit);

    if (pManager != NULL && oldManager == NULL && !bRegistered_)
    {
        CNodeUIManager::GetInstance().RegisterNode(this);
        bRegistered_ = true;
    }
    else if (pManager == NULL && bRegistered_)
    {
        CNodeUIManager::GetInstance().UnregisterNode(this);
        bRegistered_ = false;
    }
}

void CNodeUIBase::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("nodename")) == 0)
    {
        SetNodeName(CDuiString(pstrValue));
    }
    else
    {
        CContainerUI::SetAttribute(pstrName, pstrValue);
    }
}

void CNodeUIBase::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_TIMER)
    {
        UINT activeId = CNodeUIManager::GetInstance().GetActiveTimerId();
        if (activeId != 0 && event.wParam == (WPARAM)activeId)
        {
            CNodeUIManager::GetInstance().ProcessMessageQueue();
            return;
        }
    }
    CContainerUI::DoEvent(event);
}

void CNodeUIBase::PostNodeMessage(const CDuiString& msgType,
                                  const CDuiString& msgData,
                                  WPARAM             wParam,
                                  LPARAM             lParam,
                                  bool               bAsync)
{
    CNodeMessage msg(msgType, msgData, this, wParam, lParam);
    CNodeUIManager::GetInstance().BroadcastMessage(msg, bAsync);
}

void CNodeUIBase::PostNodeMessage(const CDuiString& msgType,
                                  void*             data,
                                  size_t            size,
                                  WPARAM            wParam,
                                  LPARAM            lParam,
                                  bool              bAsync)
{
    CNodeMessage msg(msgType, data, size, this, wParam, lParam);
    CNodeUIManager::GetInstance().BroadcastMessage(msg, bAsync);
}

} // namespace DuiLib