//////////////////////////////////////////////////////////////////////////////////////
//
//  FileName    :   ITimerCallBack.h
//  Creator     :   YangXD(yxdh)
//  Date        :   2008-8-26 16:14:50
//  Comment     :   Interface for the ITimerCallBack class.
//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef	_I_TIMER_CALL_BACK_H_ 
#define	_I_TIMER_CALL_BACK_H_ 


class ITimerCallBack
{
public:
	// return true:		continue timing
	// return false:	cancel timing
	virtual int		OnTimer() = 0;
};


#endif		// #ifndef _I_TIMER_CALL_BACK_H_ 
