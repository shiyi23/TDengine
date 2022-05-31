/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "parUtil.h"
#include "cJSON.h"

static char* getSyntaxErrFormat(int32_t errCode) {
  switch (errCode) {
    case TSDB_CODE_PAR_SYNTAX_ERROR:
      return "syntax error near \"%s\"";
    case TSDB_CODE_PAR_INCOMPLETE_SQL:
      return "Incomplete SQL statement";
    case TSDB_CODE_PAR_INVALID_COLUMN:
      return "Invalid column name: %s";
    case TSDB_CODE_PAR_TABLE_NOT_EXIST:
      return "Table does not exist: %s";
    case TSDB_CODE_PAR_AMBIGUOUS_COLUMN:
      return "Column ambiguously defined: %s";
    case TSDB_CODE_PAR_WRONG_VALUE_TYPE:
      return "Invalid value type: %s";
    case TSDB_CODE_PAR_ILLEGAL_USE_AGG_FUNCTION:
      return "There mustn't be aggregation";
    case TSDB_CODE_PAR_WRONG_NUMBER_OF_SELECT:
      return "ORDER BY item must be the number of a SELECT-list expression";
    case TSDB_CODE_PAR_GROUPBY_LACK_EXPRESSION:
      return "Not a GROUP BY expression";
    case TSDB_CODE_PAR_NOT_SELECTED_EXPRESSION:
      return "Not SELECTed expression";
    case TSDB_CODE_PAR_NOT_SINGLE_GROUP:
      return "Not a single-group group function";
    case TSDB_CODE_PAR_TAGS_NOT_MATCHED:
      return "Tags number not matched";
    case TSDB_CODE_PAR_INVALID_TAG_NAME:
      return "Invalid tag name: %s";
    case TSDB_CODE_PAR_NAME_OR_PASSWD_TOO_LONG:
      return "Name or password too long";
    case TSDB_CODE_PAR_PASSWD_EMPTY:
      return "Password can not be empty";
    case TSDB_CODE_PAR_INVALID_PORT:
      return "Port should be an integer that is less than 65535 and greater than 0";
    case TSDB_CODE_PAR_INVALID_ENDPOINT:
      return "Endpoint should be in the format of 'fqdn:port'";
    case TSDB_CODE_PAR_EXPRIE_STATEMENT:
      return "This statement is no longer supported";
    case TSDB_CODE_PAR_INTER_VALUE_TOO_SMALL:
      return "Interval cannot be less than %d us";
    case TSDB_CODE_PAR_DB_NOT_SPECIFIED:
      return "Database not specified";
    case TSDB_CODE_PAR_INVALID_IDENTIFIER_NAME:
      return "Invalid identifier name: %s";
    case TSDB_CODE_PAR_CORRESPONDING_STABLE_ERR:
      return "Corresponding super table not in this db";
    case TSDB_CODE_PAR_INVALID_RANGE_OPTION:
      return "Invalid option %s: %" PRId64 " valid range: [%d, %d]";
    case TSDB_CODE_PAR_INVALID_STR_OPTION:
      return "Invalid option %s: %s";
    case TSDB_CODE_PAR_INVALID_ENUM_OPTION:
      return "Invalid option %s: %" PRId64 ", only %d, %d allowed";
    case TSDB_CODE_PAR_INVALID_KEEP_NUM:
      return "Invalid number of keep options";
    case TSDB_CODE_PAR_INVALID_KEEP_ORDER:
      return "Invalid keep value, should be keep0 <= keep1 <= keep2";
    case TSDB_CODE_PAR_INVALID_KEEP_VALUE:
      return "Invalid option keep: %d, %d, %d valid range: [%d, %d]";
    case TSDB_CODE_PAR_INVALID_COMMENT_OPTION:
      return "Invalid option comment, length cannot exceed %d";
    case TSDB_CODE_PAR_INVALID_F_RANGE_OPTION:
      return "Invalid option %s: %f valid range: [%d, %d]";
    case TSDB_CODE_PAR_INVALID_ROLLUP_OPTION:
      return "Invalid option rollup: only one function is allowed";
    case TSDB_CODE_PAR_INVALID_RETENTIONS_OPTION:
      return "Invalid option retentions";
    case TSDB_CODE_PAR_GROUPBY_WINDOW_COEXIST:
      return "GROUP BY and WINDOW-clause can't be used together";
    case TSDB_CODE_PAR_INVALID_OPTION_UNIT:
      return "Invalid option %s unit: %c, only m, h, d allowed";
    case TSDB_CODE_PAR_INVALID_KEEP_UNIT:
      return "Invalid option keep unit: %c, only m, h, d allowed";
    case TSDB_CODE_PAR_AGG_FUNC_NESTING:
      return "Aggregate functions do not support nesting";
    case TSDB_CODE_PAR_INVALID_STATE_WIN_TYPE:
      return "Only support STATE_WINDOW on integer/bool/varchar column";
    case TSDB_CODE_PAR_INVALID_STATE_WIN_COL:
      return "Not support STATE_WINDOW on tag column";
    case TSDB_CODE_PAR_INVALID_STATE_WIN_TABLE:
      return "STATE_WINDOW not support for super table query";
    case TSDB_CODE_PAR_INTER_SESSION_GAP:
      return "SESSION gap should be fixed time window, and greater than 0";
    case TSDB_CODE_PAR_INTER_SESSION_COL:
      return "Only support SESSION on primary timestamp column";
    case TSDB_CODE_PAR_INTER_OFFSET_NEGATIVE:
      return "Interval offset cannot be negative";
    case TSDB_CODE_PAR_INTER_OFFSET_UNIT:
      return "Cannot use 'year' as offset when interval is 'month'";
    case TSDB_CODE_PAR_INTER_OFFSET_TOO_BIG:
      return "Interval offset should be shorter than interval";
    case TSDB_CODE_PAR_INTER_SLIDING_UNIT:
      return "Does not support sliding when interval is natural month/year";
    case TSDB_CODE_PAR_INTER_SLIDING_TOO_BIG:
      return "sliding value no larger than the interval value";
    case TSDB_CODE_PAR_INTER_SLIDING_TOO_SMALL:
      return "sliding value can not less than 1% of interval value";
    case TSDB_CODE_PAR_ONLY_ONE_JSON_TAG:
      return "Only one tag if there is a json tag";
    case TSDB_CODE_PAR_INCORRECT_NUM_OF_COL:
      return "Query block has incorrect number of result columns";
    case TSDB_CODE_PAR_INCORRECT_TIMESTAMP_VAL:
      return "Incorrect TIMESTAMP value: %s";
    case TSDB_CODE_PAR_INVALID_DAYS_VALUE:
      return "Invalid days value, should be keep2 >= keep1 >= keep0 >= days";
    case TSDB_CODE_PAR_OFFSET_LESS_ZERO:
      return "soffset/offset can not be less than 0";
    case TSDB_CODE_PAR_SLIMIT_LEAK_PARTITION_BY:
      return "slimit/soffset only available for PARTITION BY query";
    case TSDB_CODE_PAR_INVALID_TOPIC_QUERY:
      return "Invalid topic query";
    case TSDB_CODE_PAR_INVALID_DROP_STABLE:
      return "Cannot drop super table in batch";
    case TSDB_CODE_PAR_INVALID_FILL_TIME_RANGE:
      return "Start(end) time of query range required or time range too large";
    case TSDB_CODE_PAR_DUPLICATED_COLUMN:
      return "Duplicated column names";
    case TSDB_CODE_PAR_INVALID_TAGS_LENGTH:
      return "Tags length exceeds max length %d";
    case TSDB_CODE_PAR_INVALID_ROW_LENGTH:
      return "Row length exceeds max length %d";
    case TSDB_CODE_PAR_INVALID_COLUMNS_NUM:
      return "Illegal number of columns";
    case TSDB_CODE_PAR_TOO_MANY_COLUMNS:
      return "Too many columns";
    case TSDB_CODE_PAR_INVALID_FIRST_COLUMN:
      return "First column must be timestamp";
    case TSDB_CODE_PAR_INVALID_VAR_COLUMN_LEN:
      return "Invalid binary/nchar column length";
    case TSDB_CODE_PAR_INVALID_TAGS_NUM:
      return "Invalid number of tag columns";
    case TSDB_CODE_PAR_INVALID_INTERNAL_PK:
      return "Invalid _c0 or _rowts expression";
    case TSDB_CODE_PAR_INVALID_TIMELINE_FUNC:
      return "Invalid timeline function";
    case TSDB_CODE_PAR_INVALID_PASSWD:
      return "Invalid password";
    case TSDB_CODE_PAR_INVALID_ALTER_TABLE:
      return "Invalid alter table statement";
    case TSDB_CODE_PAR_CANNOT_DROP_PRIMARY_KEY:
      return "Primary timestamp column cannot be dropped";
    case TSDB_CODE_PAR_INVALID_MODIFY_COL:
      return "Only binary/nchar column length could be modified";
    case TSDB_CODE_PAR_INVALID_TBNAME:
      return "Invalid tbname pseudo column";
    case TSDB_CODE_PAR_INVALID_FUNCTION_NAME:
      return "Invalid function name";
    case TSDB_CODE_PAR_COMMENT_TOO_LONG:
      return "Comment too long";
    case TSDB_CODE_PAR_NOT_ALLOWED_FUNC:
      return "Some functions are allowed only in the SELECT list of a query. "
             "And, cannot be mixed with other non scalar functions or columns.";
    case TSDB_CODE_PAR_NOT_ALLOWED_WIN_QUERY:
      return "Window query not supported, since the result of subquery not include valid timestamp column";
    case TSDB_CODE_PAR_INVALID_DROP_COL:
      return "No columns can be dropped";
    case TSDB_CODE_PAR_INVALID_COL_JSON:
      return "Only tag can be json type";
    case TSDB_CODE_PAR_VALUE_TOO_LONG:
      return "Value too long for column/tag: %s";
    case TSDB_CODE_OUT_OF_MEMORY:
      return "Out of memory";
    default:
      return "Unknown error";
  }
}

int32_t generateSyntaxErrMsg(SMsgBuf* pBuf, int32_t errCode, ...) {
  va_list vArgList;
  va_start(vArgList, errCode);
  vsnprintf(pBuf->buf, pBuf->len, getSyntaxErrFormat(errCode), vArgList);
  va_end(vArgList);
  return errCode;
}

int32_t buildInvalidOperationMsg(SMsgBuf* pBuf, const char* msg) {
  strncpy(pBuf->buf, msg, pBuf->len);
  return TSDB_CODE_TSC_INVALID_OPERATION;
}

int32_t buildSyntaxErrMsg(SMsgBuf* pBuf, const char* additionalInfo, const char* sourceStr) {
  const char* msgFormat1 = "syntax error near \'%s\'";
  const char* msgFormat2 = "syntax error near \'%s\' (%s)";
  const char* msgFormat3 = "%s";

  const char* prefix = "syntax error";
  if (sourceStr == NULL) {
    assert(additionalInfo != NULL);
    snprintf(pBuf->buf, pBuf->len, msgFormat1, additionalInfo);
    return TSDB_CODE_TSC_SQL_SYNTAX_ERROR;
  }

  char buf[64] = {0};  // only extract part of sql string
  strncpy(buf, sourceStr, tListLen(buf) - 1);

  if (additionalInfo != NULL) {
    snprintf(pBuf->buf, pBuf->len, msgFormat2, buf, additionalInfo);
  } else {
    const char* msgFormat = (0 == strncmp(sourceStr, prefix, strlen(prefix))) ? msgFormat3 : msgFormat1;
    snprintf(pBuf->buf, pBuf->len, msgFormat, buf);
  }

  return TSDB_CODE_TSC_SQL_SYNTAX_ERROR;
}

SSchema* getTableColumnSchema(const STableMeta* pTableMeta) {
  assert(pTableMeta != NULL);
  return (SSchema*)pTableMeta->schema;
}

static SSchema* getOneColumnSchema(const STableMeta* pTableMeta, int32_t colIndex) {
  assert(pTableMeta != NULL && pTableMeta->schema != NULL && colIndex >= 0 &&
         colIndex < (getNumOfColumns(pTableMeta) + getNumOfTags(pTableMeta)));

  SSchema* pSchema = (SSchema*)pTableMeta->schema;
  return &pSchema[colIndex];
}

SSchema* getTableTagSchema(const STableMeta* pTableMeta) {
  assert(pTableMeta != NULL &&
         (pTableMeta->tableType == TSDB_SUPER_TABLE || pTableMeta->tableType == TSDB_CHILD_TABLE));
  return getOneColumnSchema(pTableMeta, getTableInfo(pTableMeta).numOfColumns);
}

int32_t getNumOfColumns(const STableMeta* pTableMeta) {
  assert(pTableMeta != NULL);
  // table created according to super table, use data from super table
  return getTableInfo(pTableMeta).numOfColumns;
}

int32_t getNumOfTags(const STableMeta* pTableMeta) {
  assert(pTableMeta != NULL);
  return getTableInfo(pTableMeta).numOfTags;
}

STableComInfo getTableInfo(const STableMeta* pTableMeta) {
  assert(pTableMeta != NULL);
  return pTableMeta->tableInfo;
}

static uint32_t getTableMetaSize(const STableMeta* pTableMeta) {
  int32_t totalCols = 0;
  if (pTableMeta->tableInfo.numOfColumns >= 0) {
    totalCols = pTableMeta->tableInfo.numOfColumns + pTableMeta->tableInfo.numOfTags;
  }

  return sizeof(STableMeta) + totalCols * sizeof(SSchema);
}

STableMeta* tableMetaDup(const STableMeta* pTableMeta) {
  size_t size = getTableMetaSize(pTableMeta);

  STableMeta* p = taosMemoryMalloc(size);
  memcpy(p, pTableMeta, size);
  return p;
}

int32_t trimString(const char* src, int32_t len, char* dst, int32_t dlen) {
  if (len <= 0 || dlen <= 0) return 0;

  char    delim = src[0];
  int32_t j = 0;
  for (uint32_t k = 1; k < len - 1; ++k) {
    if (j >= dlen) {
      dst[j - 1] = '\0';
      return j;
    }
    if (src[k] == delim && src[k + 1] == delim) {  // deal with "", ''
      dst[j] = src[k + 1];
      j++;
      k++;
      continue;
    }

    if (src[k] == '\\') {  // deal with escape character
      if (src[k + 1] == 'n') {
        dst[j] = '\n';
      } else if (src[k + 1] == 'r') {
        dst[j] = '\r';
      } else if (src[k + 1] == 't') {
        dst[j] = '\t';
      } else if (src[k + 1] == '\\') {
        dst[j] = '\\';
      } else if (src[k + 1] == '\'') {
        dst[j] = '\'';
      } else if (src[k + 1] == '"') {
        dst[j] = '"';
      } else if (src[k + 1] == '%' || src[k + 1] == '_') {
        dst[j++] = src[k];
        dst[j] = src[k + 1];
      } else {
        dst[j] = src[k + 1];
      }
      j++;
      k++;
      continue;
    }

    dst[j] = src[k];
    j++;
  }
  dst[j] = '\0';
  return j;
}

static bool isValidateTag(char* input) {
  if (!input) return false;
  for (size_t i = 0; i < strlen(input); ++i) {
    if (isprint(input[i]) == 0) return false;
  }
  return true;
}

int32_t parseJsontoTagData(const char* json, SArray* pTagVals, STag **ppTag, SMsgBuf* pMsgBuf) {
  int32_t   retCode = TSDB_CODE_SUCCESS;
  // set json NULL data
  if (!json || strtrim((char*)json) == 0 || strcasecmp(json, TSDB_DATA_NULL_STR_L) == 0) {
    retCode = TSDB_CODE_SUCCESS;
    goto end;
  }

  // set json real data
  cJSON* root = cJSON_Parse(json);
  if (root == NULL) {
    retCode = buildSyntaxErrMsg(pMsgBuf, "json parse error", json);
    goto end;
  }

  int32_t size = cJSON_GetArraySize(root);
  if (!cJSON_IsObject(root)) {
    retCode = buildSyntaxErrMsg(pMsgBuf, "json error invalide value", json);
    goto end;
  }

  SHashObj* keyHash = taosHashInit(8, taosGetDefaultHashFunction(TSDB_DATA_TYPE_BINARY), false, false);
  for (int32_t i = 0; i < size; i++) {
    cJSON* item = cJSON_GetArrayItem(root, i);
    if (!item) {
      uError("json inner error:%d", i);
      retCode = buildSyntaxErrMsg(pMsgBuf, "json inner error", json);
      goto end;
    }

    char* jsonKey = item->string;
    if (!isValidateTag(jsonKey)) {
      retCode = buildSyntaxErrMsg(pMsgBuf, "json key not validate", jsonKey);
      goto end;
    }
    size_t keyLen = strlen(jsonKey);
    if (keyLen > TSDB_MAX_JSON_KEY_LEN) {
      uError("json key too long error");
      retCode = buildSyntaxErrMsg(pMsgBuf, "json key too long, more than 256", jsonKey);
      goto end;
    }
    if (keyLen == 0 || taosHashGet(keyHash, jsonKey, keyLen) != NULL) {
      continue;
    }
    STagVal val = {0};
    val.pKey = jsonKey;
    taosHashPut(keyHash, jsonKey, keyLen, &keyLen, CHAR_BYTES);  // add key to hash to remove dumplicate, value is useless

    if (item->type == cJSON_String) {  // add json value  format: type|data
      char*   jsonValue = item->valuestring;
      int32_t valLen = (int32_t)strlen(jsonValue);
      char*   tmp = taosMemoryCalloc(1, valLen * TSDB_NCHAR_SIZE);
      if (!tmp) {
        retCode = TSDB_CODE_TSC_OUT_OF_MEMORY;
        goto end;
      }
      val.type = TSDB_DATA_TYPE_NCHAR;
      if (valLen > 0 && !taosMbsToUcs4(jsonValue, valLen, (TdUcs4*)tmp,
                                       (int32_t)(valLen * TSDB_NCHAR_SIZE), &valLen)) {
        uError("charset:%s to %s. val:%s, errno:%s, convert failed.", DEFAULT_UNICODE_ENCODEC, tsCharset, jsonValue,
               strerror(errno));
        retCode = buildSyntaxErrMsg(pMsgBuf, "charset convert json error", jsonValue);
        goto end;
      }
      val.nData = valLen;
      val.pData = tmp;
    } else if (item->type == cJSON_Number) {
      if (!isfinite(item->valuedouble)) {
        uError("json value is invalidate");
        retCode = buildSyntaxErrMsg(pMsgBuf, "json value number is illegal", json);
        goto end;
      }
      val.type = TSDB_DATA_TYPE_DOUBLE;
      *((double*)&(val.i64)) = item->valuedouble;
    } else if (item->type == cJSON_True || item->type == cJSON_False) {
      val.type = TSDB_DATA_TYPE_BOOL;
      *((char*)&(val.i64)) = (char)(item->valueint);
    } else if (item->type == cJSON_NULL) {
      val.type = TSDB_DATA_TYPE_NULL;
    } else {
      retCode = buildSyntaxErrMsg(pMsgBuf, "invalidate json value", json);
      goto end;
    }
    taosArrayPush(pTagVals, &val);
  }

end:
  taosHashCleanup(keyHash);
  if(retCode == TSDB_CODE_SUCCESS){
    tTagNew(pTagVals, 1, true, ppTag);
  }
  cJSON_Delete(root);
  return retCode;
}

static int32_t buildTableReq(SHashObj* pTablesHash, SArray** pTables) {
  if (NULL != pTablesHash) {
    *pTables = taosArrayInit(taosHashGetSize(pTablesHash), sizeof(SName));
    if (NULL == *pTables) {
      return TSDB_CODE_OUT_OF_MEMORY;
    }
    void* p = taosHashIterate(pTablesHash, NULL);
    while (NULL != p) {
      size_t len = 0;
      char*  pKey = taosHashGetKey(p, &len);
      char   fullName[TSDB_TABLE_FNAME_LEN] = {0};
      strncpy(fullName, pKey, len);
      SName name = {0};
      tNameFromString(&name, fullName, T_NAME_ACCT | T_NAME_DB | T_NAME_TABLE);
      taosArrayPush(*pTables, &name);
      p = taosHashIterate(pTablesHash, p);
    }
  }
  return TSDB_CODE_SUCCESS;
}

static int32_t buildDbReq(SHashObj* pDbsHash, SArray** pDbs) {
  if (NULL != pDbsHash) {
    *pDbs = taosArrayInit(taosHashGetSize(pDbsHash), TSDB_DB_FNAME_LEN);
    if (NULL == *pDbs) {
      return TSDB_CODE_OUT_OF_MEMORY;
    }
    void* p = taosHashIterate(pDbsHash, NULL);
    while (NULL != p) {
      size_t len = 0;
      char*  pKey = taosHashGetKey(p, &len);
      char   fullName[TSDB_DB_FNAME_LEN] = {0};
      strncpy(fullName, pKey, len);
      taosArrayPush(*pDbs, fullName);
      p = taosHashIterate(pDbsHash, p);
    }
  }
  return TSDB_CODE_SUCCESS;
}

static int32_t buildTableMetaReq(SHashObj* pTableMetaHash, SArray** pTableMeta) {
  return buildTableReq(pTableMetaHash, pTableMeta);
}

static int32_t buildDbVgroupReq(SHashObj* pDbVgroupHash, SArray** pDbVgroup) {
  return buildDbReq(pDbVgroupHash, pDbVgroup);
}

static int32_t buildTableVgroupReq(SHashObj* pTableVgroupHash, SArray** pTableVgroup) {
  return buildTableReq(pTableVgroupHash, pTableVgroup);
}

static int32_t buildDbCfgReq(SHashObj* pDbCfgHash, SArray** pDbCfg) { return buildDbReq(pDbCfgHash, pDbCfg); }

int32_t buildCatalogReq(const SParseMetaCache* pMetaCache, SCatalogReq* pCatalogReq) {
  int32_t code = buildTableMetaReq(pMetaCache->pTableMeta, &pCatalogReq->pTableMeta);
  if (TSDB_CODE_SUCCESS == code) {
    code = buildDbVgroupReq(pMetaCache->pDbVgroup, &pCatalogReq->pDbVgroup);
  }
  if (TSDB_CODE_SUCCESS == code) {
    code = buildTableVgroupReq(pMetaCache->pTableVgroup, &pCatalogReq->pTableHash);
  }
  if (TSDB_CODE_SUCCESS == code) {
    code = buildDbCfgReq(pMetaCache->pDbVgroup, &pCatalogReq->pDbCfg);
  }
  return code;
}

static int32_t putTableMetaToCache(const SArray* pTableMetaReq, const SArray* pTableMetaData, SHashObj* pTableMeta) {
  int32_t ntables = taosArrayGetSize(pTableMetaReq);
  for (int32_t i = 0; i < ntables; ++i) {
    char fullName[TSDB_TABLE_FNAME_LEN];
    tNameExtractFullName(taosArrayGet(pTableMetaReq, i), fullName);
    if (TSDB_CODE_SUCCESS !=
        taosHashPut(pTableMeta, fullName, strlen(fullName), taosArrayGet(pTableMetaData, i), POINTER_BYTES)) {
      return TSDB_CODE_OUT_OF_MEMORY;
    }
  }
  return TSDB_CODE_SUCCESS;
}

static int32_t putDbVgroupToCache(const SArray* pDbVgroupReq, const SArray* pDbVgroupData, SHashObj* pDbVgroup) {
  int32_t nvgs = taosArrayGetSize(pDbVgroupReq);
  for (int32_t i = 0; i < nvgs; ++i) {
    char* pDbFName = taosArrayGet(pDbVgroupReq, i);
    if (TSDB_CODE_SUCCESS !=
        taosHashPut(pDbVgroup, pDbFName, strlen(pDbFName), taosArrayGet(pDbVgroupData, i), POINTER_BYTES)) {
      return TSDB_CODE_OUT_OF_MEMORY;
    }
  }
  return TSDB_CODE_SUCCESS;
}

static int32_t putTableVgroupToCache(const SArray* pTableVgroupReq, const SArray* pTableVgroupData,
                                     SHashObj* pTableVgroup) {
  int32_t ntables = taosArrayGetSize(pTableVgroupReq);
  for (int32_t i = 0; i < ntables; ++i) {
    char fullName[TSDB_TABLE_FNAME_LEN];
    tNameExtractFullName(taosArrayGet(pTableVgroupReq, i), fullName);
    SVgroupInfo* pInfo = taosArrayGet(pTableVgroupData, i);
    if (TSDB_CODE_SUCCESS != taosHashPut(pTableVgroup, fullName, strlen(fullName), &pInfo, POINTER_BYTES)) {
      return TSDB_CODE_OUT_OF_MEMORY;
    }
  }
  return TSDB_CODE_SUCCESS;
}

static int32_t putDbCfgToCache(const SArray* pDbCfgReq, const SArray* pDbCfgData, SHashObj* pDbCfg) {
  int32_t nvgs = taosArrayGetSize(pDbCfgReq);
  for (int32_t i = 0; i < nvgs; ++i) {
    char*       pDbFName = taosArrayGet(pDbCfgReq, i);
    SDbCfgInfo* pInfo = taosArrayGet(pDbCfgData, i);
    if (TSDB_CODE_SUCCESS != taosHashPut(pDbCfg, pDbFName, strlen(pDbFName), &pInfo, POINTER_BYTES)) {
      return TSDB_CODE_OUT_OF_MEMORY;
    }
  }
  return TSDB_CODE_SUCCESS;
}

int32_t putMetaDataToCache(const SCatalogReq* pCatalogReq, const SMetaData* pMetaData, SParseMetaCache* pMetaCache) {
  int32_t code = putTableMetaToCache(pCatalogReq->pTableMeta, pMetaData->pTableMeta, pMetaCache->pTableMeta);
  if (TSDB_CODE_SUCCESS == code) {
    code = putDbVgroupToCache(pCatalogReq->pDbVgroup, pMetaData->pDbVgroup, pMetaCache->pDbVgroup);
  }
  if (TSDB_CODE_SUCCESS == code) {
    code = putTableVgroupToCache(pCatalogReq->pTableHash, pMetaData->pTableHash, pMetaCache->pTableVgroup);
  }
  if (TSDB_CODE_SUCCESS == code) {
    code = putDbCfgToCache(pCatalogReq->pDbCfg, pMetaData->pDbCfg, pMetaCache->pDbCfg);
  }
  return code;
}

int32_t reserveTableMetaInCache(int32_t acctId, const char* pDb, const char* pTable, SParseMetaCache* pMetaCache) {
  if (NULL == pMetaCache->pTableMeta) {
    pMetaCache->pTableMeta = taosHashInit(4, taosGetDefaultHashFunction(TSDB_DATA_TYPE_BINARY), true, HASH_NO_LOCK);
    if (NULL == pMetaCache->pTableMeta) {
      return TSDB_CODE_OUT_OF_MEMORY;
    }
  }
  char    fullName[TSDB_TABLE_FNAME_LEN];
  int32_t len = snprintf(fullName, sizeof(fullName), "%d.%s.%s", acctId, pDb, pTable);
  return taosHashPut(pMetaCache->pTableMeta, fullName, len, &len, POINTER_BYTES);
}

int32_t getTableMetaFromCache(SParseMetaCache* pMetaCache, const SName* pName, STableMeta** pMeta) {
  char fullName[TSDB_TABLE_FNAME_LEN];
  tNameExtractFullName(pName, fullName);
  *pMeta = taosHashGet(pMetaCache->pTableMeta, fullName, strlen(fullName));
  return NULL == *pMeta ? TSDB_CODE_PAR_INTERNAL_ERROR : TSDB_CODE_SUCCESS;
}

int32_t getDBVgInfoFromCache(SParseMetaCache* pMetaCache, const char* pDbFName, SArray** pVgInfo) {
  *pVgInfo = taosHashGet(pMetaCache->pDbVgroup, pDbFName, strlen(pDbFName));
  return NULL == *pVgInfo ? TSDB_CODE_PAR_INTERNAL_ERROR : TSDB_CODE_SUCCESS;
}

int32_t getTableHashVgroupFromCache(SParseMetaCache* pMetaCache, const SName* pName, SVgroupInfo* pVgroup) {
  char fullName[TSDB_TABLE_FNAME_LEN];
  tNameExtractFullName(pName, fullName);
  SVgroupInfo* pInfo = taosHashGet(pMetaCache->pTableVgroup, fullName, strlen(fullName));
  if (NULL == pInfo) {
    return TSDB_CODE_PAR_INTERNAL_ERROR;
  }
  memcpy(pVgroup, pInfo, sizeof(SVgroupInfo));
  return TSDB_CODE_SUCCESS;
}

int32_t getDBVgVersionFromCache(SParseMetaCache* pMetaCache, const char* pDbFName, int32_t* pVersion, int64_t* pDbId,
                                int32_t* pTableNum) {
  return TSDB_CODE_PAR_INTERNAL_ERROR;
}

int32_t getDBCfgFromCache(SParseMetaCache* pMetaCache, const char* pDbFName, SDbCfgInfo* pInfo) {
  SDbCfgInfo* pDbCfg = taosHashGet(pMetaCache->pDbCfg, pDbFName, strlen(pDbFName));
  if (NULL == pDbCfg) {
    return TSDB_CODE_PAR_INTERNAL_ERROR;
  }
  memcpy(pInfo, pDbCfg, sizeof(SDbCfgInfo));
  return TSDB_CODE_SUCCESS;
}
