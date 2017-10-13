#ifndef HistoryAdapter_H_
#define HistoryAdapter_H_
#include "../stdafx.h"

#include <control/SCmnCtrl.h>
#include <control/SSliderBar.h>
#include <control/SListView.h>
#include <helper/SAdapterBase.h>
#include <map>

#include "../SqlOp/HistoryInfoMgr.h"
namespace SOUI
{
	class CHistoryAdapterFix : public SAdapterBase
	{
		public:
				CHistoryAdapterFix();
				~CHistoryAdapterFix();
				int          getCount();
				void         SortOrder(int i=0);
				///ÓÒ¼üÊÂ¼þ
				bool         OnRClick(EventArgs *pEvt);
				void         getView(int position, SWindow * pItem,pugi::xml_node xmlTemplate);
				bool         OnCbxSelChange(EventArgs *pEvt);
	public:
		        void UpdateData();
				void ClearData();
		private:	
			    std::map<int,IBitmap*>       m_BitMap;
				std::vector<AV_Hos_Info *>   m_slQue;
				int                          m_nSortOrder;
	};
}
#endif