//////////////////////////////////////////////////////////////////////////
// Creator: Leo Jiang<JiangFengbing@gmail.com>
// Purpose: 为防止消息冲突, 集中定义kclear模块的所有自定义消息

#ifndef KCLEAR_KCLEARMSG_H_
#define KCLEAR_KCLEARMSG_H_

//////////////////////////////////////////////////////////////////////////
// 大文件管理
#define WM_BIGFILE_SCAN_END                 (WM_USER+100)

//////////////////////////////////////////////////////////////////////////
// 大文件管理List控件
#define WM_BIGFILE_LIST_LINK                (WM_USER+101)
#define WM_BIGFILE_LIST_DBCLICK             (WM_USER+102)
#define WM_BIGFILE_LIST_CONTEXT             (WM_USER+103)
#define WM_BIGFILE_LIST_MORE                (WM_USER+104)

//////////////////////////////////////////////////////////////////////////
#define WM_CLEAN_STATE                      (WM_USER+200)

//////////////////////////////////////////////////////////////////////////
#define KIS_WM_TREEDOWNBTN_SCAN             (WM_USER+250)

//////////////////////////////////////////////////////////////////////////
// 垃圾清理
#define DEF_TRASH_FLASH_EVENT               (WM_USER+700)
#define DEF_TRASH_PROGRESS_EVENT            (WM_USER+701)
#define DEF_TRASH_LIST_UPDATE_EVENT         (WM_USER+702)
#define DEF_TRASH_SKIP_RESCAN_EVENT         (WM_USER+703)
#define WM_CLEAR_SETTIING                   (WM_USER+6000)
#define WM_FINISH_INITILIZE                 (WM_USER+6001)
#define WM_TRASH_CLOSE_TASK                 (WM_USER+6002)
#define WM_TRASH_RESCAN                     (WM_USER+6003)

//////////////////////////////////////////////////////////////////////////
// 系统盘瘦身
#define SLIM_WM_LOADCOMPLETE                (WM_USER+800)   // dat加载完毕
#define SLIM_WM_ITEM_SCAN_END               (WM_USER+801)   // 子项目扫描完毕
#define SLIM_WM_ITEM_TREAT                  (WM_USER+802)
#define SLIM_WM_ITEM_TREATED                (WM_USER+803)
#define SLIM_WM_ITEM_LINK                   (WM_USER+804)

//////////////////////////////////////////////////////////////////////////
// 系统深度扫描扩展名List
#define WM_NONE_CHECKED                     (WM_USER+678)

#endif // KCLEAR_KCLEARMSG_H_
