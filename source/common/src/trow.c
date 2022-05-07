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

#define _DEFAULT_SOURCE
#include "trow.h"

const uint8_t tdVTypeByte[2][3] = {{
                                       // 2 bits
                                       TD_VTYPE_NORM_BYTE_II,
                                       TD_VTYPE_NONE_BYTE_II,
                                       TD_VTYPE_NULL_BYTE_II,
                                   },
                                   {
                                       // 1 bit
                                       TD_VTYPE_NORM_BYTE_I,  // normal
                                       TD_VTYPE_NULL_BYTE_I,
                                       TD_VTYPE_NULL_BYTE_I,  // padding
                                   }

};

// declaration
static uint8_t tdGetBitmapByte(uint8_t byte);

// static void dataColSetNEleNull(SDataCol *pCol, int nEle);

/**
 * @brief src2 data has more priority than src1
 *
 * @param target
 * @param src1
 * @param iter1
 * @param limit1
 * @param src2
 * @param iter2
 * @param limit2
 * @param tRows
 * @param update
 */
static void tdMergeTwoDataCols(SDataCols *target, SDataCols *src1, int *iter1, int limit1, SDataCols *src2, int *iter2,
                               int limit2, int tRows, bool update);

// implementation
/**
 * @brief Compress bitmap bytes comprised of 2-bits to counterpart of 1-bit.
 * e.g.
 * TD_VTYPE_NORM 0x00U(00000000) to 00000000 Normal
 * TD_VTYPE_NULL 0x01U(00000001) to 00000001 Null
 * TD_VTYPE_NONE 0x02U(00000010) to 00000001 Null
 *
 * 00000000 0x00 0x00
 * 01000000 0x40 0x08
 * 10000000 0x80 0x08
 *  ...
 * @param byte
 * @return uint8_t
 */
static uint8_t tdGetMergedBitmapByte(uint8_t byte) {
  switch (byte) {
    case 0x00:
      return 0x00;
    case 0x40:
      return 0x08;
    case 0x80:
      return 0x08;
    case 0x10:
      return 0x04;
    case 0x50:
      return 0x0c;
    case 0x90:
      return 0x0c;
    case 0x20:
      return 0x04;
    case 0x60:
      return 0x0c;
    case 0xa0:
      return 0x0c;
    case 0x04:
      return 0x02;
    case 0x44:
      return 0x0a;
    case 0x84:
      return 0x0a;
    case 0x14:
      return 0x06;
    case 0x54:
      return 0x0e;
    case 0x94:
      return 0x0e;
    case 0x24:
      return 0x06;
    case 0x64:
      return 0x0e;
    case 0xa4:
      return 0x0e;
    case 0x08:
      return 0x02;
    case 0x48:
      return 0x0a;
    case 0x88:
      return 0x0a;
    case 0x18:
      return 0x06;
    case 0x58:
      return 0x0e;
    case 0x98:
      return 0x0e;
    case 0x28:
      return 0x06;
    case 0x68:
      return 0x0e;
    case 0xa8:
      return 0x0e;
    case 0x01:
      return 0x01;
    case 0x41:
      return 0x09;
    case 0x81:
      return 0x09;
    case 0x11:
      return 0x05;
    case 0x51:
      return 0x0d;
    case 0x91:
      return 0x0d;
    case 0x21:
      return 0x05;
    case 0x61:
      return 0x0d;
    case 0xa1:
      return 0x0d;
    case 0x05:
      return 0x03;
    case 0x45:
      return 0x0b;
    case 0x85:
      return 0x0b;
    case 0x15:
      return 0x07;
    case 0x55:
      return 0x0f;
    case 0x95:
      return 0x0f;
    case 0x25:
      return 0x07;
    case 0x65:
      return 0x0f;
    case 0xa5:
      return 0x0f;
    case 0x09:
      return 0x03;
    case 0x49:
      return 0x0b;
    case 0x89:
      return 0x0b;
    case 0x19:
      return 0x07;
    case 0x59:
      return 0x0f;
    case 0x99:
      return 0x0f;
    case 0x29:
      return 0x07;
    case 0x69:
      return 0x0f;
    case 0xa9:
      return 0x0f;
    case 0x02:
      return 0x01;
    case 0x42:
      return 0x09;
    case 0x82:
      return 0x09;
    case 0x12:
      return 0x05;
    case 0x52:
      return 0x0d;
    case 0x92:
      return 0x0d;
    case 0x22:
      return 0x05;
    case 0x62:
      return 0x0d;
    case 0xa2:
      return 0x0d;
    case 0x06:
      return 0x03;
    case 0x46:
      return 0x0b;
    case 0x86:
      return 0x0b;
    case 0x16:
      return 0x07;
    case 0x56:
      return 0x0f;
    case 0x96:
      return 0x0f;
    case 0x26:
      return 0x07;
    case 0x66:
      return 0x0f;
    case 0xa6:
      return 0x0f;
    case 0x0a:
      return 0x03;
    case 0x4a:
      return 0x0b;
    case 0x8a:
      return 0x0b;
    case 0x1a:
      return 0x07;
    case 0x5a:
      return 0x0f;
    case 0x9a:
      return 0x0f;
    case 0x2a:
      return 0x07;
    case 0x6a:
      return 0x0f;
    case 0xaa:
      return 0x0f;
    default:
      // make sure the bitmap area is set to 0 firstly
      ASSERT(0);
      return 0x0f;  // return NULL bitmap for exception
  }
}

/**
 * @brief Merge bitmap from 2 bits to 1 bit, and the memory buffer should be guaranteed by the invoker.
 *
 * @param srcBitmap
 * @param nBits
 * @param dstBitmap
 */
void tdMergeBitmap(uint8_t *srcBitmap, int32_t nBits, uint8_t *dstBitmap) {
  int32_t i = 0, j = 0;
  int32_t nBytes = TD_BITMAP_BYTES(nBits);
  int32_t nRoundBytes = nBits / 4;
  int32_t nRemainderBits = nBits - nRoundBytes * 4;

  switch (nRemainderBits) {
    case 0:
      // NOTHING TODO
      break;
    case 1: {
      void *lastByte = POINTER_SHIFT(srcBitmap, nRoundBytes);
      *(uint8_t *)lastByte &= 0xC0;
    } break;
    case 2: {
      void *lastByte = POINTER_SHIFT(srcBitmap, nRoundBytes);
      *(uint8_t *)lastByte &= 0xF0;
    } break;
    case 3: {
      void *lastByte = POINTER_SHIFT(srcBitmap, nRoundBytes);
      *(uint8_t *)lastByte &= 0xFC;
    } break;
    default:
      ASSERT(0);
  }

  if (nBytes > 0) {
    dstBitmap[j] = (tdGetMergedBitmapByte(srcBitmap[i]) << 4);
  }

  while ((++i) < nBytes) {
    if ((i & 1) == 0) {
      dstBitmap[j] = (tdGetMergedBitmapByte(srcBitmap[i]) << 4);
    } else {
      dstBitmap[j] |= tdGetMergedBitmapByte(srcBitmap[i]);
      ++j;
    }
  }
}

static FORCE_INLINE void dataColSetNullAt(SDataCol *pCol, int index, bool setBitmap, int8_t bitmapMode) {
  if (IS_VAR_DATA_TYPE(pCol->type)) {
    pCol->dataOff[index] = pCol->len;
    char *ptr = POINTER_SHIFT(pCol->pData, pCol->len);
    setVardataNull(ptr, pCol->type);
    pCol->len += varDataTLen(ptr);
  } else {
    setNull(POINTER_SHIFT(pCol->pData, TYPE_BYTES[pCol->type] * index), pCol->type, pCol->bytes);
    pCol->len += TYPE_BYTES[pCol->type];
  }
  if (setBitmap) {
    tdSetBitmapValType(pCol->pBitmap, index, TD_VTYPE_NONE, bitmapMode);
  }
}

// static void dataColSetNEleNull(SDataCol *pCol, int nEle) {
//   if (IS_VAR_DATA_TYPE(pCol->type)) {
//     pCol->len = 0;
//     for (int i = 0; i < nEle; i++) {
//       dataColSetNullAt(pCol, i);
//     }
//   } else {
//     setNullN(pCol->pData, pCol->type, pCol->bytes, nEle);
//     pCol->len = TYPE_BYTES[pCol->type] * nEle;
//   }
// }

/**
 * @brief Set bitmap area by byte preferentially and then by bit.
 *
 * @param pBitmap
 * @param nEle
 * @param valType
 * @param bitmapMode 0 for 2 bits, 1 for 1 bit
 * @return int32_t
 */
int32_t tdSetBitmapValTypeN(void *pBitmap, int16_t nEle, TDRowValT valType, int8_t bitmapMode) {
  TASSERT(valType < TD_VTYPE_MAX);
  int32_t nBytes = (bitmapMode == 0 ? nEle / TD_VTYPE_PARTS : nEle / TD_VTYPE_PARTS_I);
  uint8_t vTypeByte = tdVTypeByte[bitmapMode][valType];
  for (int i = 0; i < nBytes; ++i) {
    *(uint8_t *)pBitmap = vTypeByte;
    pBitmap = POINTER_SHIFT(pBitmap, 1);
  }

  int32_t nLeft = nEle - nBytes * (bitmapMode == 0 ? TD_VTYPE_BITS : TD_VTYPE_BITS_I);
  for (int j = 0; j < nLeft; ++j) {
    tdSetBitmapValType(pBitmap, j, valType, bitmapMode);
  }
  return TSDB_CODE_SUCCESS;
}

bool tdIsBitmapBlkNorm(const void *pBitmap, int32_t numOfBits, int8_t bitmapMode) {
  int32_t nBytes = (bitmapMode == 0 ? numOfBits / TD_VTYPE_PARTS : numOfBits / TD_VTYPE_PARTS_I);
  uint8_t vTypeByte = tdVTypeByte[bitmapMode][TD_VTYPE_NORM];
  for (int i = 0; i < nBytes; ++i) {
    if (*((uint8_t *)pBitmap) != vTypeByte) {
      return false;
    }
    pBitmap = POINTER_SHIFT(pBitmap, i);
  }

  int32_t nLeft = numOfBits - nBytes * (bitmapMode == 0 ? TD_VTYPE_BITS : TD_VTYPE_BITS_I);

  for (int j = 0; j < nLeft; ++j) {
    uint8_t vType;
    tdGetBitmapValType(pBitmap, j, &vType, bitmapMode);
    if (vType != TD_VTYPE_NORM) {
      return false;
    }
  }
  return true;
}

static FORCE_INLINE void dataColSetNoneAt(SDataCol *pCol, int index, bool setBitmap, int8_t bitmapMode) {
  if (IS_VAR_DATA_TYPE(pCol->type)) {
    pCol->dataOff[index] = pCol->len;
    char *ptr = POINTER_SHIFT(pCol->pData, pCol->len);
    setVardataNull(ptr, pCol->type);
    pCol->len += varDataTLen(ptr);
  } else {
    setNull(POINTER_SHIFT(pCol->pData, TYPE_BYTES[pCol->type] * index), pCol->type, pCol->bytes);
    pCol->len += TYPE_BYTES[pCol->type];
  }
  if (setBitmap) {
    tdSetBitmapValType(pCol->pBitmap, index, TD_VTYPE_NONE, bitmapMode);
  }
}

static void dataColSetNEleNone(SDataCol *pCol, int nEle, int8_t bitmapMode) {
  if (IS_VAR_DATA_TYPE(pCol->type)) {
    pCol->len = 0;
    for (int i = 0; i < nEle; ++i) {
      dataColSetNoneAt(pCol, i, false, bitmapMode);
    }
  } else {
    setNullN(pCol->pData, pCol->type, pCol->bytes, nEle);
    pCol->len = TYPE_BYTES[pCol->type] * nEle;
  }
#ifdef TD_SUPPORT_BITMAP
  tdSetBitmapValTypeN(pCol->pBitmap, nEle, TD_VTYPE_NONE, bitmapMode);
#endif
}

#if 0
void trbSetRowInfo(SRowBuilder *pRB, bool del, uint16_t sver) {
  // TODO
}

void trbSetRowVersion(SRowBuilder *pRB, uint64_t ver) {
  // TODO
}

void trbSetRowTS(SRowBuilder *pRB, TSKEY ts) {
  // TODO
}

int trbWriteCol(SRowBuilder *pRB, void *pData, col_id_t cid) {
  // TODO
  return 0;
}

#endif

STSRow *tdRowDup(STSRow *row) {
  STSRow *trow = taosMemoryMalloc(TD_ROW_LEN(row));
  if (trow == NULL) return NULL;

  tdRowCpy(trow, row);
  return trow;
}

/**
 * @brief
 *
 * @param pCol
 * @param valType
 * @param val
 * @param numOfRows
 * @param maxPoints
 * @param bitmapMode default is 0(2 bits), otherwise 1(1 bit)
 * @param isMerge merge to current row
 * @return int
 */
int tdAppendValToDataCol(SDataCol *pCol, TDRowValT valType, const void *val, int numOfRows, int maxPoints,
                         int8_t bitmapMode, bool isMerge) {
  TASSERT(pCol != NULL);

  // Assume that the columns not specified during insert/upsert mean None.
  if (isAllRowsNone(pCol)) {
    if (tdValIsNone(valType)) {
      // all None value yet, just return
      return 0;
    }

    if (tdAllocMemForCol(pCol, maxPoints) < 0) return -1;
    if (numOfRows > 0) {
      // Find the first not None value, fill all previous values as None
      dataColSetNEleNone(pCol, numOfRows, bitmapMode);
    }
  }
  const void *value = val;
  if (!tdValTypeIsNorm(valType) || !val) {
    // TODO:
    // 1. back compatibility and easy to debug with codes of 2.0 to save NULL values.
    // 2. later on, considering further optimization, don't save Null/None for VarType.
    value = getNullValue(pCol->type);
  }
  if (!isMerge) {
    if (IS_VAR_DATA_TYPE(pCol->type)) {
      // set offset
      pCol->dataOff[numOfRows] = pCol->len;
      // Copy data
      memcpy(POINTER_SHIFT(pCol->pData, pCol->len), value, varDataTLen(value));
      // Update the length
      pCol->len += varDataTLen(value);
    } else {
      ASSERT(pCol->len == TYPE_BYTES[pCol->type] * numOfRows);
      memcpy(POINTER_SHIFT(pCol->pData, pCol->len), value, pCol->bytes);
      pCol->len += pCol->bytes;
    }
  } else if (!tdValTypeIsNone(valType)) {
    if (IS_VAR_DATA_TYPE(pCol->type)) {
      // keep the last offset
      // discard the last var data
      int32_t lastVarLen = varDataTLen(POINTER_SHIFT(pCol->pData, pCol->dataOff[numOfRows]));
      pCol->len -= lastVarLen;
      // Copy data
      memcpy(POINTER_SHIFT(pCol->pData, pCol->len), value, varDataTLen(value));
      // Update the length
      pCol->len += varDataTLen(value);
    } else {
      ASSERT(pCol->len - TYPE_BYTES[pCol->type] == TYPE_BYTES[pCol->type] * numOfRows);
      memcpy(POINTER_SHIFT(pCol->pData, pCol->len - TYPE_BYTES[pCol->type]), value, pCol->bytes);
    }
  }

#ifdef TD_SUPPORT_BITMAP
  if (!isMerge || !tdValTypeIsNone(valType)) {
    tdSetBitmapValType(pCol->pBitmap, numOfRows, valType, bitmapMode);
  }
#endif
  return 0;
}

// internal
static int32_t tdAppendTpRowToDataCol(STSRow *pRow, STSchema *pSchema, SDataCols *pCols, bool isMerge) {
#if 0
  ASSERT(pCols->numOfRows == 0 || dataColsKeyLast(pCols) < TD_ROW_KEY(pRow));
#endif

  // Multi-Version rows with the same key and different versions supported
  ASSERT(pCols->numOfRows == 0 || dataColsKeyLast(pCols) <= TD_ROW_KEY(pRow));

  int   rcol = 1;
  int   dcol = 1;
  void *pBitmap = tdGetBitmapAddrTp(pRow, pSchema->flen);

  SDataCol *pDataCol = &(pCols->cols[0]);
  ASSERT(pDataCol->colId == PRIMARYKEY_TIMESTAMP_COL_ID);
  tdAppendValToDataCol(pDataCol, TD_VTYPE_NORM, &pRow->ts, pCols->numOfRows, pCols->maxPoints, pCols->bitmapMode,
                       isMerge);

  while (dcol < pCols->numOfCols) {
    pDataCol = &(pCols->cols[dcol]);
    if (rcol >= schemaNCols(pSchema)) {
      tdAppendValToDataCol(pDataCol, TD_VTYPE_NULL, NULL, pCols->numOfRows, pCols->maxPoints, pCols->bitmapMode,
                           isMerge);
      ++dcol;
      continue;
    }

    STColumn *pRowCol = schemaColAt(pSchema, rcol);
    SCellVal  sVal = {0};
    if (pRowCol->colId == pDataCol->colId) {
      if (tdGetTpRowValOfCol(&sVal, pRow, pBitmap, pRowCol->type, pRowCol->offset - sizeof(TSKEY), rcol - 1) < 0) {
        return terrno;
      }
      tdAppendValToDataCol(pDataCol, sVal.valType, sVal.val, pCols->numOfRows, pCols->maxPoints, pCols->bitmapMode,
                           isMerge);
      ++dcol;
      ++rcol;
    } else if (pRowCol->colId < pDataCol->colId) {
      ++rcol;
    } else {
      tdAppendValToDataCol(pDataCol, TD_VTYPE_NULL, NULL, pCols->numOfRows, pCols->maxPoints, pCols->bitmapMode,
                           isMerge);
      ++dcol;
    }
  }
#if 0  
  ++pCols->numOfRows;
#endif

  return TSDB_CODE_SUCCESS;
}
// internal
static int32_t tdAppendKvRowToDataCol(STSRow *pRow, STSchema *pSchema, SDataCols *pCols, bool isMerge) {
  ASSERT(pCols->numOfRows == 0 || dataColsKeyLast(pCols) < TD_ROW_KEY(pRow));

  int   rcol = 0;
  int   dcol = 1;
  int   tRowCols = tdRowGetNCols(pRow) - 1;  // the primary TS key not included in kvRowColIdx part
  int   tSchemaCols = schemaNCols(pSchema) - 1;
  void *pBitmap = tdGetBitmapAddrKv(pRow, tdRowGetNCols(pRow));

  SDataCol *pDataCol = &(pCols->cols[0]);
  ASSERT(pDataCol->colId == PRIMARYKEY_TIMESTAMP_COL_ID);
  tdAppendValToDataCol(pDataCol, TD_VTYPE_NORM, &pRow->ts, pCols->numOfRows, pCols->maxPoints, pCols->bitmapMode,
                       isMerge);

  while (dcol < pCols->numOfCols) {
    pDataCol = &(pCols->cols[dcol]);
    if (rcol >= tRowCols || rcol >= tSchemaCols) {
      tdAppendValToDataCol(pDataCol, TD_VTYPE_NULL, NULL, pCols->numOfRows, pCols->maxPoints, pCols->bitmapMode,
                           isMerge);
      ++dcol;
      continue;
    }

    SKvRowIdx *pIdx = tdKvRowColIdxAt(pRow, rcol);
    int16_t    colIdx = -1;
    if (pIdx) {
      colIdx = POINTER_DISTANCE(pIdx, TD_ROW_COL_IDX(pRow)) / sizeof(SKvRowIdx);
    }
    TASSERT(colIdx >= 0);
    SCellVal sVal = {0};
    if (pIdx->colId == pDataCol->colId) {
      if (tdGetKvRowValOfCol(&sVal, pRow, pBitmap, pIdx->offset, colIdx) < 0) {
        return terrno;
      }
      tdAppendValToDataCol(pDataCol, sVal.valType, sVal.val, pCols->numOfRows, pCols->maxPoints, pCols->bitmapMode,
                           isMerge);
      ++dcol;
      ++rcol;
    } else if (pIdx->colId < pDataCol->colId) {
      ++rcol;
    } else {
      tdAppendValToDataCol(pDataCol, TD_VTYPE_NULL, NULL, pCols->numOfRows, pCols->maxPoints, pCols->bitmapMode,
                           isMerge);
      ++dcol;
    }
  }
#if 0
  ++pCols->numOfRows;
#endif

  return TSDB_CODE_SUCCESS;
}

/**
 * @brief exposed
 *
 * @param pRow
 * @param pSchema
 * @param pCols
 */
int32_t tdAppendSTSRowToDataCol(STSRow *pRow, STSchema *pSchema, SDataCols *pCols, bool isMerge) {
  if (TD_IS_TP_ROW(pRow)) {
    return tdAppendTpRowToDataCol(pRow, pSchema, pCols, isMerge);
  } else if (TD_IS_KV_ROW(pRow)) {
    return tdAppendKvRowToDataCol(pRow, pSchema, pCols, isMerge);
  } else {
    ASSERT(0);
  }
  return TSDB_CODE_SUCCESS;
}

/**
 * @brief source data has more priority than target
 *
 * @param target
 * @param source
 * @param rowsToMerge
 * @param pOffset
 * @param update
 * @param maxVer
 * @return int
 */
int tdMergeDataCols(SDataCols *target, SDataCols *source, int rowsToMerge, int *pOffset, bool update,
                    TDRowVerT maxVer) {
  ASSERT(rowsToMerge > 0 && rowsToMerge <= source->numOfRows);
  ASSERT(target->numOfCols == source->numOfCols);
  int offset = 0;

  if (pOffset == NULL) {
    pOffset = &offset;
  }

  SDataCols *pTarget = NULL;

  if ((target->numOfRows == 0) || (dataColsKeyLast(target) < dataColsKeyAtRow(source, *pOffset))) {  // No overlap
    ASSERT(target->numOfRows + rowsToMerge <= target->maxPoints);
    // TODO: filter the maxVer
    TSKEY lastKey = TSKEY_INITIAL_VAL;
    for (int i = 0; i < rowsToMerge; ++i) {
      bool merge = false;
      for (int j = 0; j < source->numOfCols; j++) {
        if (source->cols[j].len > 0 || target->cols[j].len > 0) {
          SCellVal sVal = {0};
          if (tdGetColDataOfRow(&sVal, source->cols + j, i + (*pOffset), source->bitmapMode) < 0) {
            TASSERT(0);
          }

          if (j == 0) {
            if (lastKey == *(TSKEY *)sVal.val) {
              if (!update) {
                break;
              }
              merge = true;
            } else if (lastKey != TSKEY_INITIAL_VAL) {
              ++target->numOfRows;
            }

            lastKey = *(TSKEY *)sVal.val;
          }
          if (i == 0) {
            (target->cols + j)->bitmap = (source->cols + j)->bitmap;
          }

          tdAppendValToDataCol(target->cols + j, sVal.valType, sVal.val, target->numOfRows, target->maxPoints,
                               target->bitmapMode, merge);
        }
      }
    }
    if (lastKey != TSKEY_INITIAL_VAL) {
      ++target->numOfRows;
    }
    (*pOffset) += rowsToMerge;
  } else {
    pTarget = tdDupDataCols(target, true);
    if (pTarget == NULL) goto _err;

    int iter1 = 0;
    tdMergeTwoDataCols(target, pTarget, &iter1, pTarget->numOfRows, source, pOffset, source->numOfRows,
                       pTarget->numOfRows + rowsToMerge, update);
  }

  tdFreeDataCols(pTarget);
  return 0;

_err:
  tdFreeDataCols(pTarget);
  return -1;
}

static void tdAppendValToDataCols(SDataCols *target, SDataCols *src, int iter, bool isMerge) {
  for (int i = 0; i < src->numOfCols; ++i) {
    ASSERT(target->cols[i].type == src->cols[i].type);
    if (src->cols[i].len > 0 || target->cols[i].len > 0) {
      SCellVal sVal = {0};
      if (tdGetColDataOfRow(&sVal, src->cols + i, iter, src->bitmapMode) < 0) {
        TASSERT(0);
      }
      if (isMerge) {
        if (!tdValTypeIsNone(sVal.valType)) {
          tdAppendValToDataCol(&(target->cols[i]), sVal.valType, sVal.val, target->numOfRows, target->maxPoints,
                               target->bitmapMode, isMerge);
        } else {
          // Keep the origin value for None
        }
      } else {
        tdAppendValToDataCol(&(target->cols[i]), sVal.valType, sVal.val, target->numOfRows, target->maxPoints,
                             target->bitmapMode, isMerge);
      }
    }
  }
}
/**
 * @brief src2 data has more priority than src1
 *
 * @param target
 * @param src1
 * @param iter1
 * @param limit1
 * @param src2
 * @param iter2
 * @param limit2
 * @param tRows
 * @param update
 */
static void tdMergeTwoDataCols(SDataCols *target, SDataCols *src1, int *iter1, int limit1, SDataCols *src2, int *iter2,
                               int limit2, int tRows, bool update) {
  tdResetDataCols(target);
  target->bitmapMode = src1->bitmapMode;
  ASSERT(limit1 <= src1->numOfRows && limit2 <= src2->numOfRows);
  int32_t nRows = 0;

  // TODO: filter the maxVer
  // TODO: handle the delete function
  TSKEY lastKey = TSKEY_INITIAL_VAL;
  while (nRows < tRows) {
    if (*iter1 >= limit1 && *iter2 >= limit2) break;

    TSKEY key1 = (*iter1 >= limit1) ? INT64_MAX : dataColsKeyAt(src1, *iter1);
    // TKEY  tkey1 = (*iter1 >= limit1) ? TKEY_NULL : dataColsTKeyAt(src1, *iter1);
    TSKEY key2 = (*iter2 >= limit2) ? INT64_MAX : dataColsKeyAt(src2, *iter2);
    // TKEY  tkey2 = (*iter2 >= limit2) ? TKEY_NULL : dataColsTKeyAt(src2, *iter2);

    // ASSERT(tkey1 == TKEY_NULL || (!TKEY_IS_DELETED(tkey1)));

    if (key1 <= key2) {
      // select key1 if not delete
      if (update && (lastKey == key1)) {
        tdAppendValToDataCols(target, src1, *iter1, true);
      } else if (lastKey != key1) {
        if (lastKey != TSKEY_INITIAL_VAL) {
          ++target->numOfRows;
        }
        tdAppendValToDataCols(target, src1, *iter1, false);
      }
      ++nRows;
      ++(*iter1);
      lastKey = key1;
    } else {
      // use key2 if not deleted
      // TODO: handle the delete function
      if (update && (lastKey == key2)) {
        tdAppendValToDataCols(target, src2, *iter2, true);
      } else if (lastKey != key2) {
        if (lastKey != TSKEY_INITIAL_VAL) {
          ++target->numOfRows;
        }
        tdAppendValToDataCols(target, src2, *iter2, false);
      }

      ++nRows;
      ++(*iter2);
      lastKey = key2;
    }

    ASSERT(target->numOfRows <= target->maxPoints - 1);
  }
  if (lastKey != TSKEY_INITIAL_VAL) {
    ++target->numOfRows;
  }
}

STSRow *mergeTwoRows(void *buffer, STSRow *row1, STSRow *row2, STSchema *pSchema1, STSchema *pSchema2) {
#if 0
  ASSERT(TD_ROW_KEY(row1) == TD_ROW_KEY(row2));
  ASSERT(schemaVersion(pSchema1) == TD_ROW_SVER(row1));
  ASSERT(schemaVersion(pSchema2) == TD_ROW_SVER(row2));
  ASSERT(schemaVersion(pSchema1) >= schemaVersion(pSchema2));
#endif

#if 0
  SArray *stashRow = taosArrayInit(pSchema1->numOfCols, sizeof(SColInfo));
  if (stashRow == NULL) {
    return NULL;
  }

  STSRow  pRow = buffer;
  STpRow dataRow = memRowDataBody(pRow);
  memRowSetType(pRow, SMEM_ROW_DATA);
  dataRowSetVersion(dataRow, schemaVersion(pSchema1));  // use latest schema version
  dataRowSetLen(dataRow, (TDRowLenT)(TD_DATA_ROW_HEAD_SIZE + pSchema1->flen));

  TDRowLenT dataLen = 0, kvLen = TD_MEM_ROW_KV_HEAD_SIZE;

  int32_t  i = 0;  // row1
  int32_t  j = 0;  // row2
  int32_t  nCols1 = schemaNCols(pSchema1);
  int32_t  nCols2 = schemaNCols(pSchema2);
  SColInfo colInfo = {0};
  int32_t  kvIdx1 = 0, kvIdx2 = 0;

  while (i < nCols1) {
    STColumn *pCol = schemaColAt(pSchema1, i);
    void *    val1 = tdGetMemRowDataOfColEx(row1, pCol->colId, pCol->type, TD_DATA_ROW_HEAD_SIZE + pCol->offset, &kvIdx1);
    // if val1 != NULL, use val1;
    if (val1 != NULL && !isNull(val1, pCol->type)) {
      tdAppendColVal(dataRow, val1, pCol->type, pCol->offset);
      kvLen += tdGetColAppendLen(SMEM_ROW_KV, val1, pCol->type);
      setSColInfo(&colInfo, pCol->colId, pCol->type, val1);
      taosArrayPush(stashRow, &colInfo);
      ++i;  // next col
      continue;
    }

    void *val2 = NULL;
    while (j < nCols2) {
      STColumn *tCol = schemaColAt(pSchema2, j);
      if (tCol->colId < pCol->colId) {
        ++j;
        continue;
      }
      if (tCol->colId == pCol->colId) {
        val2 = tdGetMemRowDataOfColEx(row2, tCol->colId, tCol->type, TD_DATA_ROW_HEAD_SIZE + tCol->offset, &kvIdx2);
      } else if (tCol->colId > pCol->colId) {
        // set NULL
      }
      break;
    }  // end of while(j<nCols2)
    if (val2 == NULL) {
      val2 = (void *)getNullValue(pCol->type);
    }
    tdAppendColVal(dataRow, val2, pCol->type, pCol->offset);
    if (!isNull(val2, pCol->type)) {
      kvLen += tdGetColAppendLen(SMEM_ROW_KV, val2, pCol->type);
      setSColInfo(&colInfo, pCol->colId, pCol->type, val2);
      taosArrayPush(stashRow, &colInfo);
    }

    ++i;  // next col
  }

  dataLen = TD_ROW_LEN(pRow);

  if (kvLen < dataLen) {
    // scan stashRow and generate SKVRow
    memset(buffer, 0, sizeof(dataLen));
    STSRow tRow = buffer;
    memRowSetType(tRow, SMEM_ROW_KV);
    SKVRow kvRow = (SKVRow)memRowKvBody(tRow);
    int16_t nKvNCols = (int16_t) taosArrayGetSize(stashRow);
    kvRowSetLen(kvRow, (TDRowLenT)(TD_KV_ROW_HEAD_SIZE + sizeof(SColIdx) * nKvNCols));
    kvRowSetNCols(kvRow, nKvNCols);
    memRowSetKvVersion(tRow, pSchema1->version);

    int32_t toffset = 0;
    int16_t k;
    for (k = 0; k < nKvNCols; ++k) {
      SColInfo *pColInfo = taosArrayGet(stashRow, k);
      tdAppendKvColVal(kvRow, pColInfo->colVal, true, pColInfo->colId, pColInfo->colType, toffset);
      toffset += sizeof(SColIdx);
    }
    ASSERT(kvLen == TD_ROW_LEN(tRow));
  }
  taosArrayDestroy(stashRow);
  return buffer;
#endif
  return NULL;
}

SDataCols *tdDupDataCols(SDataCols *pDataCols, bool keepData) {
  SDataCols *pRet = tdNewDataCols(pDataCols->maxCols, pDataCols->maxPoints);
  if (pRet == NULL) return NULL;

  pRet->numOfCols = pDataCols->numOfCols;
  pRet->bitmapMode = pDataCols->bitmapMode;
  pRet->sversion = pDataCols->sversion;
  if (keepData) pRet->numOfRows = pDataCols->numOfRows;

  for (int i = 0; i < pDataCols->numOfCols; ++i) {
    pRet->cols[i].type = pDataCols->cols[i].type;
    pRet->cols[i].bitmap = pDataCols->cols[i].bitmap;
    pRet->cols[i].colId = pDataCols->cols[i].colId;
    pRet->cols[i].bytes = pDataCols->cols[i].bytes;
    pRet->cols[i].offset = pDataCols->cols[i].offset;

    if (keepData) {
      if (pDataCols->cols[i].len > 0) {
        if (tdAllocMemForCol(&pRet->cols[i], pRet->maxPoints) < 0) {
          tdFreeDataCols(pRet);
          return NULL;
        }
        pRet->cols[i].len = pDataCols->cols[i].len;
        memcpy(pRet->cols[i].pData, pDataCols->cols[i].pData, pDataCols->cols[i].len);
        if (IS_VAR_DATA_TYPE(pRet->cols[i].type)) {
          int dataOffSize = sizeof(VarDataOffsetT) * pDataCols->maxPoints;
          memcpy(pRet->cols[i].dataOff, pDataCols->cols[i].dataOff, dataOffSize);
        }
        if (!TD_COL_ROWS_NORM(pRet->cols + i)) {
          memcpy(pRet->cols[i].pBitmap, pDataCols->cols[i].pBitmap, TD_BITMAP_BYTES(pDataCols->numOfRows));
        }
      }
    }
  }

  return pRet;
}