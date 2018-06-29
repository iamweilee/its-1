﻿#ifndef EYEGLE_COMMON_COMPRESS_LIB_H_
#define EYEGLE_COMMON_COMPRESS_LIB_H_

#include "common/Global.h"
#include "zlib.h"


namespace itstation {
namespace common {

bool COMMON_API ZlibCompress(void * pIn,int nInLen,void * pOut,int * nOutLen);

bool COMMON_API ZlibUnCompress(void * pIn,int nInLen,void * pOut,int * nOutLen);

int COMMON_API ZlibLimitLen(int len);

}
}


#endif