
#ifndef WF_INT_H
#define WF_INT_H

#ifdef _WIN32
#define WINDOWS_PLATFORM
#endif

#ifdef WIN64
#define WINDOWS_PLATFORM
#endif

#ifdef WINDOWS_PLATFORM //ÊÇWINDOWSÆ½Ì¨
    typedef __int8            int8_t;
    typedef __int16           int16_t;
    typedef __int32           int32_t;
    typedef __int64           int64_t;
    typedef unsigned __int8   uint8_t;
    typedef unsigned __int16  uint16_t;
    typedef unsigned __int32  uint32_t;
    typedef unsigned __int64  uint64_t;
#else
    #include <sys/types.h>
    typedef u_int8_t    uint8_t;
    typedef u_int16_t   uint16_t;
    typedef u_int32_t   uint32_t;
    typedef u_int64_t   uint64_t;
#endif

#endif  // end of #ifndef

