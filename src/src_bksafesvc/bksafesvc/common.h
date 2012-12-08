#pragma once

#include "common\utility.h"
#include "comproxy\comdata_scan_def.h"
#include "skylark2\bkeng.h"
#include "winmod\winosver.h"
#include "winmod\winfilefind.h"
#include "winmod\winfilefinddepthfirst.h"
#include "winmod\winpath.h"
#include "winmod\winprocessenumerator.h"
#include "skylark2\skylarkpath.h"

enum
{
    BkScanRelationDirStepMemory = BkScanStepMemory + 0x1000,
    BkScanRelationDirStepAutorun = BkScanStepAutorun + 0x1000
};


