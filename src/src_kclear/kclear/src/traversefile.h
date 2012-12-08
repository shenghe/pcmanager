/********************************************************************
* CreatedOn: 2007-1-16   18:31
* FileName: TraverseFile.h
* CreatedBy: lidengwang <lidengwang@kingsoft.net>
* $LastChangedDate$
* $LastChangedRevision$
* $LastChangedBy$
* $HeadURL:  $
* Purpose:
*********************************************************************/

#ifndef __TRAVERSEFILE_H__
#define __TRAVERSEFILE_H__

typedef struct tagFIND_FILE_DATA   FINDFILEDATA, *LPFINDFILEDATA;


class ITraverseFile
{
public:
    virtual void TraverseFile(LPFINDFILEDATA pFileData) = 0;

    virtual void TraverseProcess(unsigned long uCurPos) = 0;

    virtual void TraverseProcessEnd(unsigned long uCurPos,CString videoPath) = 0;

    virtual void TraverseFinished() = 0;
};


#endif //__TRAVERSEFILE_H__