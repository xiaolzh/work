#ifndef SWSTD_H
#define SWSTD_H
/**
 *  @brief This is the basic file which shoudl be included firstly, it contains some global control triggers and basic defines
 */

/** @todo - basic defines
 *        - All English
 */

#define IN
#define OUT
#define INOUT

/// define the debug macros
#if DEBUG
#define INLINE
#else
#define INLINE inline
#endif

/// define the test macros
#if DEBUG
#define UNIT_TEST 1
#endif // DEBUG


//typedef unsigned long long u64;
//typedef unsigned int u32;
//typedef long long s64;
//typedef int s32;
//typedef bool Boolean;

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#endif // ~>.!.<~
