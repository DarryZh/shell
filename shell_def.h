#ifndef __SHELL_DEF_H__
#define __SHELL_DEF_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int                             sh_bool_t;      /**< boolean type */
typedef signed long                     sh_base_t;      /**< Nbit CPU related date type */
typedef unsigned long                   sh_ubase_t;     /**< Nbit unsigned CPU related data type */

typedef int8_t                          sh_int8_t;      /**<  8bit integer type */
typedef int16_t                         sh_int16_t;     /**< 16bit integer type */
typedef int32_t                         sh_int32_t;     /**< 32bit integer type */
typedef uint8_t                         sh_uint8_t;     /**<  8bit unsigned integer type */
typedef uint16_t                        sh_uint16_t;    /**< 16bit unsigned integer type */
typedef uint32_t                        sh_uint32_t;    /**< 32bit unsigned integer type */
typedef int64_t                         sh_int64_t;     /**< 64bit integer type */
typedef uint64_t                        sh_uint64_t;    /**< 64bit unsigned integer type */
typedef size_t                          sh_size_t;      /**< Type for size number */
// typedef ssize_t                         sh_ssize_t;     /**< Used for a count of bytes or an error indication */

typedef sh_base_t                       sh_err_t;       /**< Type for error number */

#define SH_EOK                          0               /**< There is no error */
#define SH_ERROR                        1               /**< A generic error happens */

/* boolean type definitions */
#define SH_TRUE                         1               /**< boolean true  */
#define SH_FALSE                        0               /**< boolean fails */

/* null pointer definition */
#define SH_NULL                         0

#define SH_UINT8_MAX                    UINT8_MAX       /**< Maximum number of UINT8 */
#define SH_UINT16_MAX                   UINT16_MAX      /**< Maximum number of UINT16 */
#define SH_UINT32_MAX                   UINT32_MAX      /**< Maximum number of UINT32 */


/* Common Utilities */

#define SH_UNUSED(x)                   ((void)x)

/* compile time assertion */
#define SH_CTASSERT(name, expn) typedef char _ct_assesh_##name[(expn)?1:-1]

/* Compiler Related Definitions */
#if defined(__ARMCC_VERSION)           /* ARM Compiler */
#define sh_section(x)               __attribute__((section(x)))
#define sh_used                     __attribute__((used))
#define sh_align(n)                 __attribute__((aligned(n)))
#define sh_weak                     __attribute__((weak))
#define sh_inline                   static __inline
#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
#define sh_section(x)               @ x
#define sh_used                     __root
#define PRAGMA(x)                   _Pragma(#x)
#define sh_align(n)                    PRAGMA(data_alignment=n)
#define sh_weak                     __weak
#define sh_inline                   static inline
#elif defined (__GNUC__)                /* GNU GCC Compiler */
#define __SH_STRINGIFY(x...)        #x
#define SH_STRINGIFY(x...)          __SH_STRINGIFY(x)
#define sh_section(x)               __attribute__((section(x)))
#define sh_used                     __attribute__((used))
#define sh_align(n)                 __attribute__((aligned(n)))
#define sh_weak                     __attribute__((weak))
#define sh_inline                   static __inline
#elif defined (_MSC_VER)
#define sh_section(x)
#define sh_used
#define sh_align(n)                 __declspec(align(n))
#define sh_weak
#define sh_inline                   static __inline
#else
    #error not supported tool chain
#endif /* __ARMCC_VERSION */

#ifdef __cplusplus
}
#endif

#endif
