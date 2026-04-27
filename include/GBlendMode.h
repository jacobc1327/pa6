/*
 *  Copyright 2018 Mike Reed
 */

#ifndef GBlendMode_DEFINED
#define GBlendMode_DEFINED

#include "GTypes.h"

enum class GBlendMode {
    kClear,    //!<     0
    kSrc,      //!<     S
    kDst,      //!<     D
    kSrcOver,  //!<     S + (1 - Sa)*D
    kDstOver,  //!<     D + (1 - Da)*S
    kSrcIn,    //!<     Da * S
    kDstIn,    //!<     Sa * D
    kSrcOut,   //!<     (1 - Da)*S
    kDstOut,   //!<     (1 - Sa)*D
    kSrcATop,  //!<     Da*S + (1 - Sa)*D
    kDstATop,  //!<     Sa*D + (1 - Da)*S
    kXor,      //!<     (1 - Sa)*D + (1 - Da)*S

    kModulate,      //!< S*D
    kScreen,        //!< S + D - S*D
    kDarken,        //!< Rc = S + D - max(S*Da, D*Sa), Ra = kSrcOver
    kLighten,       //!< Rc = S + D - min(S*Da, D*Sa), Ra = kSrcOver

    kLast_GBlendMode = kLighten,
};

#endif
