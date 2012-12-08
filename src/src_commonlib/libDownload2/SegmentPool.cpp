#include "StdAfx.h"
#include "SegmentPool.h"
#include <algorithm>

CSegmentPool::CSegmentPool( LPCTSTR szFilename ) : m_strFilename(szFilename)
{
	m_iDownloaded = 0;
}

int64 CSegmentPool::Init( const RemoteFileInfo &remoteFileInfo, INT nCocurrent )
{
	m_remoteFileInfo = remoteFileInfo;
	MYTRACE(_T("_AllocSegments : RandomAccess:%d  ContentLength:%I64d\r\n"), m_remoteFileInfo.bRandomAccess, m_remoteFileInfo.fileSize);
	
	release_array(parts);
	m_iDownloaded = 0;
	if (m_remoteFileInfo.bRandomAccess && m_remoteFileInfo.fileSize>0)
	{		
		int64 iContentLength = 0;
		CSegmentInfoFile segfile(m_strFilename);
		if(segfile.Load(iContentLength, m_iDownloaded, parts) && iContentLength==m_remoteFileInfo.fileSize)
		{
			;
		}
		else
		{
			release_array(parts);
			// If load failed, and filesize not match the content-length 				
			iContentLength = m_remoteFileInfo.fileSize;
			m_iDownloaded = 0;

			int nblock = 0;
			int64 block_size = 0;
			if (iContentLength > (MIN_BLOCK * nCocurrent))
			{
				nblock = nCocurrent;
				block_size = iContentLength/nCocurrent;
			}
			else
			{
				nblock = (iContentLength + MIN_BLOCK - 1) / MIN_BLOCK;
				block_size = iContentLength/nblock;
			}
			for (int i=0; i<nblock; ++i)
			{
				parts.push_back( new Segment(i, i*block_size, i*block_size+block_size) );
			}
			parts[parts.size()-1]->endposition = iContentLength;
		}
	}
	else
	{
		parts.push_back( new Segment(0, 0, m_remoteFileInfo.fileSize > 0 ? m_remoteFileInfo.fileSize : -1) );
	}

	for(size_t i=0; i<parts.size(); ++i)
	{
		Segment *s = parts[i];
		s->isDone() ? parts_done.push_back( s ) : parts_queuing.push_back( s );
	}
	return m_iDownloaded;
}

BOOL CSegmentPool::hasRunning() const
{
	return !parts_running.empty();
}

BOOL CSegmentPool::hasQueuing() const
{
	return !parts_queuing.empty();
}

SegmentPtrs & CSegmentPool::GetRunningSegments()
{
	return parts_running;
}

const SegmentPtrs & CSegmentPool::GetQueueSegments()
{
	return parts_queuing;
}

const SegmentPtrs & CSegmentPool::GetSegments()
{
	return parts;
}

SegmentPtr CSegmentPool::GetNextSegment( SegmentPtr pcurrent )
{
	// 检查是不是可以接着下一段, 否则得话, 需要开始调度了 
	__int64 currentEnd = -1;
	if(pcurrent)
		currentEnd = pcurrent->endposition;
	
	SegmentPtr pnext=NULL;
	if( pnext = _GetSegmentFromQueue(currentEnd) )
		return pnext;
	
	if(m_remoteFileInfo.fileSize<=0)
		return NULL;
	
	// 查找未邻接的未开始的块 : 这里返回的instream 应该不为空, 需要重新放到request pool 
	if( currentEnd>0 && (pnext=_GetSegmentFromRunning(pcurrent, currentEnd)) )
		return pnext;
	
	// 下一个是空的, 看是否可以从其中切割一个块出来 
	if(pnext=_SplitRunningSegment())
		return pnext;

	return NULL;
}

void CSegmentPool::Put( SegmentPtr ptr )
{
	ATLASSERT(std::find(parts_running.begin(), parts_running.end(), ptr)!=parts_running.end());
	_RemoveFromRunning(ptr);
	ATLASSERT(ptr->instream==NULL);
	//ptr->startposition = ptr->currentposition;	
	if(ptr->endposition<=0 || ptr->isDone())
		parts_done.push_back(ptr);
	else
		parts_queuing.push_back( ptr );
}

void CSegmentPool::NotifySegmentToRunning( SegmentPtr ptr )
{
	ATLASSERT(std::find(parts_running.begin(), parts_running.end(), ptr)==parts_running.end());
	parts_running.push_back( ptr );
}

SegmentPtr CSegmentPool::_GetSegmentFromQueue( __int64 currentEnd )
{
	Segment *pnext = NULL, *pnext_neighbor=NULL;
	for(size_t i=0; i<parts_queuing.size(); ++i)
	{
		Segment *p = parts_queuing[i];
		if(!pnext)
			pnext = p;
		else
		{
			if(pnext->currentposition>p->currentposition)
				pnext = p;
		}

		ATLASSERT(p->endposition>p->currentposition);
		if(currentEnd>0 && p->currentposition==currentEnd && p->endposition>p->currentposition)
		{
			pnext_neighbor = p;
			break;
		}
	}
	if(pnext_neighbor)
		pnext = pnext_neighbor;
	if(pnext)
		parts_queuing.erase( std::find(parts_queuing.begin(), parts_queuing.end(), pnext) );
	return pnext;
}

SegmentPtr CSegmentPool::_GetSegmentFromRunning( SegmentPtr pcurrent, __int64 currentEnd )
{
	SegmentPtr pnext = NULL;
	for(size_t i=0; i<parts_running.size(); ++i)
	{
		Segment *p = parts_running[i];
		if(p==pcurrent)
			continue;

		ATLASSERT(p->endposition>p->currentposition);
		if( p->currentposition==currentEnd && p->endposition>p->currentposition )
		{
			pnext = p;
			break;
		}
	}
	if(pnext)
	{
		// Remove from running, and disable !! 
		parts_running.erase( std::find(parts_running.begin(), parts_running.end(), pnext) );
	}	
	return pnext;
}

SegmentPtr CSegmentPool::_SplitRunningSegment()
{
	if(m_remoteFileInfo.fileSize<=0)
		return NULL;

	// TODO 考虑速度, 不能只考虑大小 
	SegmentPtr pnext = NULL;
	int64 maxblocksize = 0;
	Segment *ps_to_split = NULL;
	for(size_t i=0; i<parts_running.size(); ++i)
	{
		Segment *ps = parts_running[i];
		if(maxblocksize < ps->Remain())
		{
			maxblocksize = ps->Remain();
			ps_to_split = ps;
		}
	}
	if(maxblocksize>MIN_BLOCK_TOSPLIT)
	{
		int64 middle = (ps_to_split->endposition + ps_to_split->currentposition)/2;
		pnext = new Segment(parts.size(), middle, ps_to_split->endposition);
		parts.push_back( pnext );
		ps_to_split->endposition = middle;
		ps_to_split->instream->ModifyRange(ps_to_split->currentposition, ps_to_split->endposition);
	}
	return pnext;
}

SegmentPtr CSegmentPool::_RemoveFromRunning( SegmentPtr ptr )
{
	SegmentPtrs::iterator it = std::find(parts_running.begin(), parts_running.end(), ptr);
	if(it!=parts_running.end())
		parts_running.erase(it);
	return ptr;
}