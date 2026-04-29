#include "common/type/text_type.h"
#include "common/lang/sstream.h"
#include "common/log/log.h"
#include "common/value.h"
#include "common/lang/comparator.h"

int TextType::compare(const Value &left, const Value &right) const
{
  // 比较两个 text 值
  // const string &left_str = left.get_string();
  // const string &right_str = right.get_string();
  // return left_str.compare(right_str);
  ASSERT(left.attr_type() == AttrType::TEXTS && right.attr_type() == AttrType::TEXTS, "invalid type");
  return common::compare_string(
      (void *)left.value_.pointer_value_, left.length_, (void *)right.value_.pointer_value_, right.length_);
}
RC TextType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    case AttrType::TEXTS: {
      result.set_string(val.value_.pointer_value_, val.length_);
      result.set_type(AttrType::TEXTS);
      return RC::SUCCESS;
    }
    case AttrType::CHARS: {
      result.set_string(val.value_.pointer_value_, val.length_);
      return RC::SUCCESS;
    }
    default: {
      return RC::UNIMPLEMENTED;
    }
  }
}
RC TextType::set_value_from_str(Value &val, const string &data) const
{
  // 从字符串设置 text 值
  //val.set_string(data.c_str(), data.length());
  val.set_string(data.c_str(), static_cast<int>(data.length()));
  val.set_type(AttrType::TEXTS);
  return RC::SUCCESS;
}
int TextType::cast_cost(AttrType type)
{
  if (type == AttrType::TEXTS || type == AttrType::CHARS) {
    return 0;
  }
  return INT32_MAX;
}

RC TextType::to_string(const Value &val, string &result) const
{
  stringstream ss;
  ss << val.value_.pointer_value_;
  result = ss.str();
  return RC::SUCCESS;
}