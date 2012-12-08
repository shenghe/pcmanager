///////////////////////////////////////////////////////////////
//
//	Filename: 	KTraversableTreeCtrl.h
//	Creator:	lichenglin  <lichenglin@kingsoft.net>
//	Date:		2007-12-5  19:23
//	Comment:	支持对树控件进行单步遍历.
//              需要TTreeCtrl实现GetRootItem和GetNextItem函数
//              WTL和MFC中的树控件都可以支持
//
///////////////////////////////////////////////////////////////

#ifndef _KTraversableTreeCtrl_h_
#define _KTraversableTreeCtrl_h_


template <class TTreeCtrl>
class KUITraversableTreeCtrl : public TTreeCtrl
{
public:
	HTREEITEM TravGetFirstItem()
	{
		TTreeCtrl* pT = static_cast<TTreeCtrl*>(this);
		return pT->GetRootItem();
	}


	HTREEITEM TravGetNextItem( HTREEITEM hItem )
	{
		TTreeCtrl* pT = static_cast<TTreeCtrl*>(this);

		HTREEITEM hNextItem = NULL;

		//--------------------------------------
		// 如果有子结点 则返回第一个子结点
		hNextItem = pT->GetNextItem( hItem, TVGN_CHILD );
		if( hNextItem != NULL )
			return hNextItem;

		//--------------------------------------
		// 否则, 如果有兄弟结点, 则返回下一个兄弟结点
		hNextItem = pT->GetNextItem( hItem, TVGN_NEXT );
		if( hNextItem != NULL )
			return hNextItem;

		//--------------------------------------
		// 否则, 返回祖先结点的下一个兄弟结点.
		while( (hItem = pT->GetNextItem( hItem, TVGN_PARENT )) != NULL )
		{
			hNextItem = pT->GetNextItem( hItem, TVGN_NEXT );
			if( hNextItem != NULL )
				break;
		}

		return hNextItem;
	}
};



#endif // _KTraversableTreeCtrl_h_