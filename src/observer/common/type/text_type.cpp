#include "common/type/text_type.h"
#include "common/value.h"
#include "common/log/log.h"
#include "common/lang/comparator.h"

int TextType::compare(const Value &left, const Value &right) const
{
  // 比较两个 text 值
  const string &left_str = left.get_string();
  const string &right_str = right.get_string();
  return left_str.compare(right_str);
}

RC TextType::set_value_from_str(Value &val, const string &data) const
{
  // 从字符串设置 text 值
  val.set_string(data.c_str(), data.length());
  val.set_type(AttrType::TEXTS);
  return RC::SUCCESS;
}