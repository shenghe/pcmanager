////////////////////////////////////////////////////////////////////////////////
//
//      Comment   : 网镖流量监控对话框中list控件的数据行
//
//      Create at : 2010-08-25
//      Create by : 林杰
//
//////////////////////////////////////////////////////////////////////////////
// 网络活动列表
#pragma  once
#include "treelistctrlex.h"
#include <vector>
#include "common/netfluxdef.h"
using namespace std;


class KNetFlowMonListItemData;

class KNetFlowMonList
{
private:
    vector<KNetFlowMonListItemData*>		m_ItemsVector;
    CTreeListCtrlEx&					m_listCtrl;

public:
	KNetFlowMonList(CTreeListCtrlEx& listCtrl);
    ~KNetFlowMonList();

    HRESULT UpdateNetFlow(vector<KProcFluxItem>* pFlowInfo, bool& bIsModified);

private:
	KNetFlowMonListItemData* FindMatch(
        vector<KNetFlowMonListItemData*>& items,
        KNetFlowMonListItemData* pItem);

    void Clear();
};