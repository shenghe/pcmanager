#include "stdafx.h"
#include "common/netfluxdef.h"
#include<vector>
using namespace std;
#include "netflowmonlist.h"
#include "netflowmonlistitemdata.h"

KNetFlowMonList::~KNetFlowMonList()
{
    Clear();
}

KNetFlowMonListItemData* KNetFlowMonList::FindMatch(vector<KNetFlowMonListItemData*>& items, KNetFlowMonListItemData* pItem)
{
    for(UINT i = 0; i < items.size(); i++)
    {
        if(items[i]->Match(pItem))
        {
            return items[i];
        }
    }

    return NULL;
}

KNetFlowMonList::KNetFlowMonList(CTreeListCtrlEx& listCtrl)
    : m_listCtrl(listCtrl)
{
}

HRESULT KNetFlowMonList::UpdateNetFlow(vector<KProcFluxItem>* pFlowInfo, bool& bIsModified)
{
	if (NULL == pFlowInfo)
	{
		return E_INVALIDARG;
	}

    // 生成列表
    vector<KNetFlowMonListItemData*> newProcessFlowVector;

    for(int i = 0; i < pFlowInfo->size(); i++)
    {
		KProcFluxItem netflowItem = (*pFlowInfo)[i];
		KNetFlowMonListItemData* pItem = new KNetFlowMonListItemData(netflowItem);
		newProcessFlowVector.push_back(pItem);
    }

    if(!m_ItemsVector.empty())
    {
        vector<KNetFlowMonListItemData*>::iterator it = m_ItemsVector.end();

        do
        {
            it--;
            BOOL bBegin = (it == m_ItemsVector.begin());

            KNetFlowMonListItemData* pItem = *it;
            KNetFlowMonListItemData* pItemCur = FindMatch(newProcessFlowVector, pItem);
			
			// 删除不存在于新的列表中的项
            if(pItemCur == NULL)
            {
                if(pItem->GetListItem())
                {
                    m_listCtrl.DeleteItem(pItem->GetListItem());
					pItem->SetListItem(NULL);
                }

                if(pItem)
                {
                    delete pItem;
                }

                vector<KNetFlowMonListItemData*>::iterator itErase = it;

                if(!bBegin)
                {
                    it--;
                    m_ItemsVector.erase(itErase);
                    it++;
                }
                else
                {
                    m_ItemsVector.erase(itErase);
                }

                bIsModified = true;
            }

            if(bBegin)
            {
                break;
            }
        }
        while(TRUE);
    }

    // 创建原先列表中没有的项
    for(UINT i = 0; i < newProcessFlowVector.size(); i++)
    {
        KNetFlowMonListItemData* pItem = newProcessFlowVector[i];
        KNetFlowMonListItemData* pItemCur = FindMatch(m_ItemsVector, pItem);

        if(pItemCur)
        {
			 if (pItemCur->Update(*pItem))
			 {
				 bIsModified = true;
			 }
        }
        else
        {
            pItemCur = new KNetFlowMonListItemData();
            *pItemCur = *pItem;
            m_ItemsVector.push_back(pItemCur);

			HTREELISTITEMEX item = m_listCtrl.InsertItem(pItemCur, NULL);
			pItemCur->SetListItem(item);
			
            //pItemCur->CreateItem(m_listCtrl);
            bIsModified = true;
        }

        if(pItem)
        {
            delete pItem;
        }
    }

    newProcessFlowVector.clear();

    return S_OK;
}

void KNetFlowMonList::Clear()
{
    for(UINT i = 0; i < m_ItemsVector.size(); i++)
    {
        KNetFlowMonListItemData* pItem = m_ItemsVector[i];

        if(pItem->GetListItem())
        {
            m_listCtrl.DeleteItem(pItem->GetListItem());
        }

        if(pItem)
        {
            delete pItem;
        }
    }

    m_ItemsVector.clear();
}