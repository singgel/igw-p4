/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef _P4_TYPES_
#define _P4_TYPES_

#if __TARGET_TOFINO__ == 1
    typedef bit<3> mirror_type_t;
#else
    typedef bit<4> mirror_type_t;
#endif

const mirror_type_t MIRROR_TYPE_I2E = 1;
const mirror_type_t MIRROR_TYPE_E2E = 2;

#endif /* _P4_TYPES_ */
