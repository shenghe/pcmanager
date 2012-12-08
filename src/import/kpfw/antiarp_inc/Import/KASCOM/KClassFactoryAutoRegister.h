///////////////////////////////////////////////////////////////
//
//	Filename: 	KClassFactoryAutoRegister.h
//	Creator:	lichenglin  <lichenglin@kingsoft.net>
//	Date:		2007-8-26   13:00
//	Comment:	
//
///////////////////////////////////////////////////////////////

#ifndef _KComAutoRegister_h_
#define _KComAutoRegister_h_
#include "KClassFactoryManagement.h"
#include "KClassFactory.h"


//.-----------------------------------------------------------------------------
// KClassFactoryAutoRegister, Auto register the class factory to mgt.
//.-----------------------------------------------------------------------------

template <class TCOM, const CLSID& rclsid>
class KClassFactoryAutoRegister
{
public:
	KClassFactoryAutoRegister()
	{
		_Register();
	}

private:
	void _Register()
	{
		KComObject<KClassFactory<TCOM> >* pCf = new KComObject<KClassFactory<TCOM> >;
		pCf->AddRef();

		KClassFactoryManagement* pCfmgt = KClassFactoryManagement::GetInstance();
		pCfmgt->Add( rclsid, pCf );
	}
};




//.-----------------------------------------------------------------------------
// OBJECT_ENTRY_AUTO
//.-----------------------------------------------------------------------------

/*
    通过该宏自动为组件生成类厂并注册到类厂管理器.
*/
#define KAS_OBJECT_ENTRY_AUTO( clsid, comImpl )  \
	/*__declspec(selectany)*/ KClassFactoryAutoRegister<KComObject<comImpl>, clsid>  _cfar_##comImpl;


#endif // _KComAutoRegister_h_