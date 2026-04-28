#include "sql/executor/drop_table_executor.h"
#include "sql/stmt/drop_table_stmt.h"
#include "storage/db/db.h"
#include "session/session.h"
#include "event/sql_event.h"  // <--- 新增
#include "event/session_event.h"    // <--- 新增（针对 sql_event->session_event()）
RC DropTableExecutor::execute(SQLStageEvent *sql_event)
{
  // 1. 获取 Stmt 对象
  Stmt *stmt = sql_event->stmt();
  DropTableStmt *drop_stmt = static_cast<DropTableStmt *>(stmt);

  // 2. 获取当前数据库
  Session *session = sql_event->session_event()->session();
  Db *db = session->get_current_db();

  // 3. 调用存储层删除表
  return db->drop_table(drop_stmt->table_name().c_str());
}