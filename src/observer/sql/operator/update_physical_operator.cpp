/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/operator/update_physical_operator.h"

#include <string.h>

#include "common/log/log.h"
#include "sql/expr/tuple.h"
#include "storage/table/table.h"
#include "storage/trx/trx.h"

RC UpdatePhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    return RC::SUCCESS;
  }

  unique_ptr<PhysicalOperator> &child = children_[0];

  RC rc = child->open(trx);
  if (rc != RC::SUCCESS) {
    LOG_WARN("failed to open child operator: %s", strrc(rc));
    return rc;
  }

  trx_ = trx;

  while (OB_SUCC(rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (tuple == nullptr) {
      LOG_WARN("failed to get current record: %s", strrc(rc));
      return rc;
    }

    RowTuple *row_tuple = static_cast<RowTuple *>(tuple);
    Record   &record    = row_tuple->record();
    records_.emplace_back(record);
  }

  child->close();

  if (rc != RC::RECORD_EOF) {
    return rc;
  }

  rc = RC::SUCCESS;
  for (Record &old_record : records_) {
    Record new_record;
    rc = new_record.copy_data(old_record.data(), old_record.len());
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to copy record data. rc=%s", strrc(rc));
      return rc;
    }
    new_record.set_rid(old_record.rid());

    Value real_value;
    const Value *value = &value_;
    if (value_.attr_type() != field_meta_->type()) {
      rc = Value::cast_to(value_, field_meta_->type(), real_value);
      if (rc != RC::SUCCESS) {
        LOG_WARN("failed to cast value. field=%s, from=%s, to=%s, rc=%s",
            field_meta_->name(), attr_type_to_string(value_.attr_type()), attr_type_to_string(field_meta_->type()), strrc(rc));
        return rc;
      }
      value = &real_value;
    }

    size_t copy_len = field_meta_->len();
    const size_t data_len = value->length();
    if (field_meta_->type() == AttrType::CHARS || field_meta_->type() == AttrType::TEXTS) {
      memset(new_record.data() + field_meta_->offset(), 0, field_meta_->len());
      if (copy_len > data_len) {
        copy_len = data_len + 1;
      }
    }
    memcpy(new_record.data() + field_meta_->offset(), value->data(), copy_len);

    rc = trx_->update_record(table_, old_record, new_record);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to update record: %s", strrc(rc));
      return rc;
    }
  }

  return RC::SUCCESS;
}

RC UpdatePhysicalOperator::next()
{
  return RC::RECORD_EOF;
}

RC UpdatePhysicalOperator::close()
{
  records_.clear();
  return RC::SUCCESS;
}
