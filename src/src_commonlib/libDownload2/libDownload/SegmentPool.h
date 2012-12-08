#pragma once
#include "HttpAsyncRequest.h"
#include "Segment.h"

class CSegmentPool 
{
public:
	CSegmentPool(LPCTSTR szFilename);

	int64 Init(const RemoteFileInfo &remoteFileInfo, INT nCocurrent);

	BOOL hasRunning() const;
	BOOL hasQueuing() const;
	SegmentPtrs &GetRunningSegments();
	const SegmentPtrs &GetQueueSegments();
	const SegmentPtrs &GetSegments();
	
	// 
	SegmentPtr GetNextSegment(SegmentPtr pcurrent);
	void Put(SegmentPtr ptr);
	void NotifySegmentToRunning(SegmentPtr ptr);
	
protected:
	SegmentPtr _GetSegmentFromQueue( __int64 currentEnd );
	SegmentPtr _GetSegmentFromRunning( SegmentPtr pcurrent, __int64 currentEnd );
	SegmentPtr _SplitRunningSegment(  );
	SegmentPtr _RemoveFromRunning( SegmentPtr ptr );

protected:
	CString m_strFilename;
	RemoteFileInfo m_remoteFileInfo;
	int64 m_iDownloaded;
	SegmentPtrs parts, parts_running, parts_queuing, parts_done;
};

