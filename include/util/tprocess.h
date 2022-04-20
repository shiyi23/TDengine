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

#ifndef _TD_UTIL_PROCESS_H_
#define _TD_UTIL_PROCESS_H_

#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { PROC_FUNC_REQ = 1, PROC_FUNC_RSP, PROC_FUNC_REGIST, PROC_FUNC_RELEASE } EProcFuncType;

typedef struct SProcObj SProcObj;
typedef void *(*ProcMallocFp)(int32_t contLen);
typedef void *(*ProcFreeFp)(void *pCont);
typedef void (*ProcConsumeFp)(void *parent, void *pHead, int16_t headLen, void *pBody, int32_t bodyLen,
                              EProcFuncType ftype);

typedef struct {
  ProcConsumeFp childConsumeFp;
  ProcMallocFp  childMallocHeadFp;
  ProcFreeFp    childFreeHeadFp;
  ProcMallocFp  childMallocBodyFp;
  ProcFreeFp    childFreeBodyFp;
  ProcConsumeFp parentConsumeFp;
  ProcMallocFp  parentMallocHeadFp;
  ProcFreeFp    parentFreeHeadFp;
  ProcMallocFp  parentMallocBodyFp;
  ProcFreeFp    parentFreeBodyFp;
  SShm          shm;
  void         *parent;
  const char   *name;
  bool          isChild;
} SProcCfg;

SProcObj *taosProcInit(const SProcCfg *pCfg);
void      taosProcCleanup(SProcObj *pProc);
int32_t   taosProcRun(SProcObj *pProc);
void      taosProcStop(SProcObj *pProc);

int32_t taosProcPutToChildQ(SProcObj *pProc, const void *pHead, int16_t headLen, const void *pBody, int32_t bodyLen,
                            void *handle, EProcFuncType ftype);
void    taosProcRemoveHandle(SProcObj *pProc, void *handle);
void    taosProcCloseHandles(SProcObj *pProc, void (*HandleFp)(void *handle));
void    taosProcPutToParentQ(SProcObj *pProc, const void *pHead, int16_t headLen, const void *pBody, int32_t bodyLen,
                             EProcFuncType ftype);

#ifdef __cplusplus
}
#endif

#endif /*_TD_UTIL_PROCESS_H_*/