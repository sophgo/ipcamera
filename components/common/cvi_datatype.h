/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_datatype.h
 * Description:
 */

#ifndef __CVI_DATATYPE_H__
#define __CVI_DATATYPE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdint.h>

/*----------------------------------------------
 * The common data type
 *----------------------------------------------
 */
typedef unsigned char           CVI_UCHAR;
typedef unsigned char           CVI_U8;
typedef unsigned short          CVI_U16;
typedef unsigned int            CVI_U32;
typedef unsigned int            CVI_HANDLE;

typedef signed char             CVI_S8;
typedef char                    CVI_CHAR;
typedef short                   CVI_S16;
typedef int                     CVI_S32;

typedef unsigned long           CVI_UL;
typedef signed long             CVI_SL;

typedef float                   CVI_FLOAT;
typedef double                  CVI_DOUBLE;

typedef void                    CVI_VOID;
typedef unsigned char           CVI_BOOL;

typedef unsigned long long int  CVI_U64;
typedef long long int           CVI_S64;

typedef unsigned int            CVI_SIZE_T;

/*----------------------------------------------
 * const defination
 *----------------------------------------------
 */
#define CVI_NULL                0L
#define CVI_SUCCESS             0
#define CVI_FAILURE             (-1)
#define CVI_FAILURE_ILLEGAL_PARAM (-2)
#define CVI_TRUE                1
#define CVI_FALSE               0

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_DATATYPE_H__ */
