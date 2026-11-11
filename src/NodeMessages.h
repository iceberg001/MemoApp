#ifndef __NODEMESSAGES_H__
#define __NODEMESSAGES_H__

// Node 组件间消息类型（CNodeUIManager 广播总线）
#define NODE_MSG_DATA_CHANGED  _T("data_changed")   // wParam=无, data: todos/memos/notes/report
#define NODE_MSG_DB_STATUS     _T("db_status")      // data: ok/error 描述

#endif
