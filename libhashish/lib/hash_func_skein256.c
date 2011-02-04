#include "libhashish.h"

#define  SKEIN_PORT_CODE	/* instantiate any code in skein_port.h */

#include <string.h>		/* get the memcpy/memset functions */
#include <stddef.h>		/* get size_t definition */
#include <limits.h>

#ifndef SKEIN_256_NIST_MAX_HASHBITS
#define SKEIN_256_NIST_MAX_HASHBITS (256)
#endif

#define IS_BIG_ENDIAN      4321	/* byte 0 is most significant (mc68k) */
#define IS_LITTLE_ENDIAN   1234	/* byte 0 is least significant (i386) */

/* Include files where endian defines and byteswap functions may reside */
#if defined( __FreeBSD__ ) || defined( __OpenBSD__ ) || defined( __NetBSD__ )
#  include <sys/endian.h>
#elif defined( BSD ) && ( BSD >= 199103 ) || defined( __APPLE__ ) || \
      defined( __CYGWIN32__ ) || defined( __DJGPP__ ) || defined( __osf__ )
#  include <machine/endian.h>
#elif defined( __linux__ ) || defined( __GNUC__ ) || defined( __GNU_LIBRARY__ )
#  if !defined( __MINGW32__ ) && !defined(AVR)
#    include <endian.h>
#    if !defined( __BEOS__ )
#      include <byteswap.h>
#    endif
#  endif
#endif

/* Now attempt to set the define for platform byte order using any  */
/* of the four forms SYMBOL, _SYMBOL, __SYMBOL & __SYMBOL__, which  */
/* seem to encompass most endian symbol definitions                 */

#if defined( BIG_ENDIAN ) && defined( LITTLE_ENDIAN )
#  if defined( BYTE_ORDER ) && BYTE_ORDER == BIG_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#  elif defined( BYTE_ORDER ) && BYTE_ORDER == LITTLE_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#  endif
#elif defined( BIG_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#elif defined( LITTLE_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif

#if defined( _BIG_ENDIAN ) && defined( _LITTLE_ENDIAN )
#  if defined( _BYTE_ORDER ) && _BYTE_ORDER == _BIG_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#  elif defined( _BYTE_ORDER ) && _BYTE_ORDER == _LITTLE_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#  endif
#elif defined( _BIG_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#elif defined( _LITTLE_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif

#if defined( __BIG_ENDIAN ) && defined( __LITTLE_ENDIAN )
#  if defined( __BYTE_ORDER ) && __BYTE_ORDER == __BIG_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#  elif defined( __BYTE_ORDER ) && __BYTE_ORDER == __LITTLE_ENDIAN
#    define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#  endif
#elif defined( __BIG_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#elif defined( __LITTLE_ENDIAN )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif

#if defined( __BIG_ENDIAN__ ) && defined( __LITTLE_ENDIAN__ )
#  if defined( __BYTE_ORDER__ ) && __BYTE_ORDER__ == __BIG_ENDIAN__
#    define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#  elif defined( __BYTE_ORDER__ ) && __BYTE_ORDER__ == __LITTLE_ENDIAN__
#    define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#  endif
#elif defined( __BIG_ENDIAN__ )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#elif defined( __LITTLE_ENDIAN__ )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif

/*  if the platform byte order could not be determined, then try to */
/*  set this define using common machine defines                    */
#if !defined(PLATFORM_BYTE_ORDER)

#if   defined( __alpha__ ) || defined( __alpha ) || defined( i386 )       || \
      defined( __i386__ )  || defined( _M_I86 )  || defined( _M_IX86 )    || \
      defined( __OS2__ )   || defined( sun386 )  || defined( __TURBOC__ ) || \
      defined( vax )       || defined( vms )     || defined( VMS )        || \
      defined( __VMS )     || defined( _M_X64 )  || defined( AVR )
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN

#elif defined( AMIGA )   || defined( applec )    || defined( __AS400__ )  || \
      defined( _CRAY )   || defined( __hppa )    || defined( __hp9000 )   || \
      defined( ibm370 )  || defined( mc68000 )   || defined( m68k )       || \
      defined( __MRC__ ) || defined( __MVS__ )   || defined( __MWERKS__ ) || \
      defined( sparc )   || defined( __sparc)    || defined( SYMANTEC_C ) || \
      defined( __VOS__ ) || defined( __TIGCC__ ) || defined( __TANDEM )   || \
      defined( THINK_C ) || defined( __VMCMS__ )
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN

#elif 0				/* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#elif 0				/* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER IS_BIG_ENDIAN
#else
#  error Please edit lines 126 or 128 in brg_endian.h to set the platform byte order
#endif
#endif

/* special handler for IA64, which may be either endianness (?)  */
/* here we assume little-endian, but this may need to be changed */
#if defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
#  define PLATFORM_MUST_ALIGN (1)
#ifndef PLATFORM_BYTE_ORDER
#  define PLATFORM_BYTE_ORDER IS_LITTLE_ENDIAN
#endif
#endif

#ifndef   PLATFORM_MUST_ALIGN
#  define PLATFORM_MUST_ALIGN (0)
#endif

#ifndef BRG_UI8
#  define BRG_UI8
#  if UCHAR_MAX == 255u
typedef unsigned char uint_8t;
#  else
#    error Please define uint_8t as an 8-bit unsigned integer type in brg_types.h
#  endif
#endif

#ifndef BRG_UI16
#  define BRG_UI16
#  if USHRT_MAX == 65535u
typedef unsigned short uint_16t;
#  else
#    error Please define uint_16t as a 16-bit unsigned short type in brg_types.h
#  endif
#endif

#ifndef BRG_UI32
#  define BRG_UI32
#  if UINT_MAX == 4294967295u
#    define li_32(h) 0x##h##u
typedef unsigned int uint_32t;
#  elif ULONG_MAX == 4294967295u
#    define li_32(h) 0x##h##ul
typedef unsigned long uint_32t;
#  elif defined( _CRAY )
#    error This code needs 32-bit data types, which Cray machines do not provide
#  else
#    error Please define uint_32t as a 32-bit unsigned integer type in brg_types.h
#  endif
#endif

#ifndef BRG_UI64
#  if defined( __BORLANDC__ ) && !defined( __MSDOS__ )
#    define BRG_UI64
#    define li_64(h) 0x##h##ui64
typedef unsigned __int64 uint_64t;
#  elif defined( _MSC_VER ) && ( _MSC_VER < 1300 )	/* 1300 == VC++ 7.0 */
#    define BRG_UI64
#    define li_64(h) 0x##h##ui64
typedef unsigned __int64 uint_64t;
#  elif defined( __sun ) && defined(ULONG_MAX) && ULONG_MAX == 0xfffffffful
#    define BRG_UI64
#    define li_64(h) 0x##h##ull
typedef unsigned long long uint_64t;
#  elif defined( UINT_MAX ) && UINT_MAX > 4294967295u
#    if UINT_MAX == 18446744073709551615u
#      define BRG_UI64
#      define li_64(h) 0x##h##u
typedef unsigned int uint_64t;
#    endif
#  elif defined( ULONG_MAX ) && ULONG_MAX > 4294967295u
#    if ULONG_MAX == 18446744073709551615ul
#      define BRG_UI64
#      define li_64(h) 0x##h##ul
typedef unsigned long uint_64t;
#    endif
#  elif defined( ULLONG_MAX ) && ULLONG_MAX > 4294967295u
#    if ULLONG_MAX == 18446744073709551615ull
#      define BRG_UI64
#      define li_64(h) 0x##h##ull
typedef unsigned long long uint_64t;
#    endif
#  elif defined( ULONG_LONG_MAX ) && ULONG_LONG_MAX > 4294967295u
#    if ULONG_LONG_MAX == 18446744073709551615ull
#      define BRG_UI64
#      define li_64(h) 0x##h##ull
typedef unsigned long long uint_64t;
#    endif
#  elif defined(__GNUC__)	/* DLW: avoid mingw problem with -ansi */
#      define BRG_UI64
#      define li_64(h) 0x##h##ull
typedef unsigned long long uint_64t;
#  endif
#endif

#if defined( NEED_UINT_64T ) && !defined( BRG_UI64 )
#  error Please define uint_64t as an unsigned 64 bit type in brg_types.h
#endif

#ifndef RETURN_VALUES
#  define RETURN_VALUES
#  if defined( DLL_EXPORT )
#    if defined( _MSC_VER ) || defined ( __INTEL_COMPILER )
#      define VOID_RETURN    __declspec( dllexport ) void __stdcall
#      define INT_RETURN     __declspec( dllexport ) int  __stdcall
#    elif defined( __GNUC__ )
#      define VOID_RETURN    __declspec( __dllexport__ ) void
#      define INT_RETURN     __declspec( __dllexport__ ) int
#    else
#      error Use of the DLL is only available on the Microsoft, Intel and GCC compilers
#    endif
#  elif defined( DLL_IMPORT )
#    if defined( _MSC_VER ) || defined ( __INTEL_COMPILER )
#      define VOID_RETURN    __declspec( dllimport ) void __stdcall
#      define INT_RETURN     __declspec( dllimport ) int  __stdcall
#    elif defined( __GNUC__ )
#      define VOID_RETURN    __declspec( __dllimport__ ) void
#      define INT_RETURN     __declspec( __dllimport__ ) int
#    else
#      error Use of the DLL is only available on the Microsoft, Intel and GCC compilers
#    endif
#  elif defined( __WATCOMC__ )
#    define VOID_RETURN  void __cdecl
#    define INT_RETURN   int  __cdecl
#  else
#    define VOID_RETURN  void
#    define INT_RETURN   int
#  endif
#endif

/*  These defines are used to declare buffers in a way that allows
    faster operations on longer variables to be used.  In all these
    defines 'size' must be a power of 2 and >= 8

    dec_unit_type(size,x)       declares a variable 'x' of length 
                                'size' bits

    dec_bufr_type(size,bsize,x) declares a buffer 'x' of length 'bsize' 
                                bytes defined as an array of variables
                                each of 'size' bits (bsize must be a 
                                multiple of size / 8)

    ptr_cast(x,size)            casts a pointer to a pointer to a 
                                varaiable of length 'size' bits
*/

#define ui_type(size)               uint_##size##t
#define dec_unit_type(size,x)       typedef ui_type(size) x
#define dec_bufr_type(size,bsize,x) typedef ui_type(size) x[bsize / (size >> 3)]
#define ptr_cast(x,size)            ((ui_type(size)*)(x))
typedef unsigned int uint_t;	/* native unsigned integer */
typedef uint_8t u08b_t;		/*  8-bit unsigned integer */
typedef uint_64t u64b_t;	/* 64-bit unsigned integer */

#ifndef RotL_64
#define RotL_64(x,N)    (((x) << (N)) | ((x) >> (64-(N))))
#endif

/*
 * Skein is "natively" little-endian (unlike SHA-xxx), for optimal
 * performance on x86 CPUs.  The Skein code requires the following
 * definitions for dealing with endianness:
 *
 *    SKEIN_NEED_SWAP:  0 for little-endian, 1 for big-endian
 *    Skein_Put64_LSB_First
 *    Skein_Get64_LSB_First
 *    Skein_Swap64
 *
 * If SKEIN_NEED_SWAP is defined at compile time, it is used here
 * along with the portable versions of Put64/Get64/Swap64, which 
 * are slow in general.
 *
 * Otherwise, an "auto-detect" of endianness is attempted below.
 * If the default handling doesn't work well, the user may insert
 * platform-specific code instead (e.g., for big-endian CPUs).
 *
 */
#ifndef SKEIN_NEED_SWAP		/* compile-time "override" for endianness? */

#if   PLATFORM_BYTE_ORDER == IS_BIG_ENDIAN
    /* here for big-endian CPUs */
#define SKEIN_NEED_SWAP   (1)
#elif PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN
    /* here for x86 and x86-64 CPUs (and other detected little-endian CPUs) */
#define SKEIN_NEED_SWAP   (0)
#if   PLATFORM_MUST_ALIGN == 0	/* ok to use "fast" versions? */
#define Skein_Put64_LSB_First(dst08,src64,bCnt) memcpy(dst08,src64,bCnt)
#define Skein_Get64_LSB_First(dst64,src08,wCnt) memcpy(dst64,src08,8*(wCnt))
#endif
#else
#error "Skein needs endianness setting!"
#endif

#endif				/* ifndef SKEIN_NEED_SWAP */

/*
 ******************************************************************
 *      Provide any definitions still needed.
 ******************************************************************
 */
#ifndef Skein_Swap64		/* swap for big-endian, nop for little-endian */
#if     SKEIN_NEED_SWAP
#define Skein_Swap64(w64)                       \
  ( (( ((u64b_t)(w64))       & 0xFF) << 56) |   \
    (((((u64b_t)(w64)) >> 8) & 0xFF) << 48) |   \
    (((((u64b_t)(w64)) >>16) & 0xFF) << 40) |   \
    (((((u64b_t)(w64)) >>24) & 0xFF) << 32) |   \
    (((((u64b_t)(w64)) >>32) & 0xFF) << 24) |   \
    (((((u64b_t)(w64)) >>40) & 0xFF) << 16) |   \
    (((((u64b_t)(w64)) >>48) & 0xFF) <<  8) |   \
    (((((u64b_t)(w64)) >>56) & 0xFF)      ) )
#else
#define Skein_Swap64(w64)  (w64)
#endif
#endif				/* ifndef Skein_Swap64 */

#ifndef Skein_Put64_LSB_First
void Skein_Put64_LSB_First(u08b_t * dst, const u64b_t * src, size_t bCnt)
#ifdef  SKEIN_PORT_CODE		/* instantiate the function code here? */
{				/* this version is fully portable (big-endian or little-endian), but slow */
	size_t n;

	for (n = 0; n < bCnt; n++)
		dst[n] = (u08b_t) (src[n >> 3] >> (8 * (n & 7)));
}
#else
;				/* output only the function prototype */
#endif
#endif				/* ifndef Skein_Put64_LSB_First */

#ifndef Skein_Get64_LSB_First
void Skein_Get64_LSB_First(u64b_t * dst, const u08b_t * src, size_t wCnt)
#ifdef  SKEIN_PORT_CODE		/* instantiate the function code here? */
{				/* this version is fully portable (big-endian or little-endian), but slow */
	size_t n;

	for (n = 0; n < 8 * wCnt; n += 8)
		dst[n / 8] = (((u64b_t) src[n])) +
		    (((u64b_t) src[n + 1]) << 8) +
		    (((u64b_t) src[n + 2]) << 16) +
		    (((u64b_t) src[n + 3]) << 24) +
		    (((u64b_t) src[n + 4]) << 32) +
		    (((u64b_t) src[n + 5]) << 40) +
		    (((u64b_t) src[n + 6]) << 48) +
		    (((u64b_t) src[n + 7]) << 56);
}
#else
;				/* output only the function prototype */
#endif
#endif				/* ifndef Skein_Get64_LSB_First */

enum {
	SKEIN_SUCCESS = 0,	/* return codes from Skein calls */
	SKEIN_FAIL = 1,
	SKEIN_BAD_HASHLEN = 2
};

#define  SKEIN_MODIFIER_WORDS  ( 2)	/* number of modifier (tweak) words */

#define  SKEIN_256_STATE_WORDS ( 4)
#define  SKEIN_512_STATE_WORDS ( 8)
#define  SKEIN1024_STATE_WORDS (16)
#define  SKEIN_MAX_STATE_WORDS (16)

#define  SKEIN_256_STATE_BYTES ( 8*SKEIN_256_STATE_WORDS)
#define  SKEIN_512_STATE_BYTES ( 8*SKEIN_512_STATE_WORDS)
#define  SKEIN1024_STATE_BYTES ( 8*SKEIN1024_STATE_WORDS)

#define  SKEIN_256_STATE_BITS  (64*SKEIN_256_STATE_WORDS)
#define  SKEIN_512_STATE_BITS  (64*SKEIN_512_STATE_WORDS)
#define  SKEIN1024_STATE_BITS  (64*SKEIN1024_STATE_WORDS)

#define  SKEIN_256_BLOCK_BYTES ( 8*SKEIN_256_STATE_WORDS)
#define  SKEIN_512_BLOCK_BYTES ( 8*SKEIN_512_STATE_WORDS)
#define  SKEIN1024_BLOCK_BYTES ( 8*SKEIN1024_STATE_WORDS)

typedef struct {
	size_t hashBitLen;	/* size of hash result, in bits */
	size_t bCnt;		/* current byte count in buffer b[] */
	u64b_t T[SKEIN_MODIFIER_WORDS];	/* tweak words: T[0]=byte cnt, T[1]=flags */
} Skein_Ctxt_Hdr_t;

typedef struct {		/*  256-bit Skein hash context structure */
	Skein_Ctxt_Hdr_t h;	/* common header context variables */
	u64b_t X[SKEIN_256_STATE_WORDS];	/* chaining variables */
	u08b_t b[SKEIN_256_BLOCK_BYTES];	/* partial block buffer (8-byte aligned) */
} Skein_256_Ctxt_t;

/*   Skein APIs for (incremental) "straight hashing" */
int Skein_256_Init(Skein_256_Ctxt_t * ctx, size_t hashBitLen);

int Skein_256_Update(Skein_256_Ctxt_t * ctx, const u08b_t * msg,
		     size_t msgByteCnt);

int Skein_256_Final(Skein_256_Ctxt_t * ctx, u08b_t * hashVal);
typedef struct {
	uint_t statebits;	/* 256, 512, or 1024 */
	union {
		Skein_Ctxt_Hdr_t h;	/* common header "overlay" */
		Skein_256_Ctxt_t ctx_256;
	} u;
} hashState;

/*
**   Skein APIs for "extended" initialization: MAC keys, tree hashing.
**   After an InitExt() call, just use Update/Final calls as with Init().
**
**   Notes: Same parameters as _Init() calls, plus treeInfo/key/keyBytes.
**          When keyBytes == 0 and treeInfo == SKEIN_SEQUENTIAL, 
**              the results of InitExt() are identical to calling Init().
**          The function Init() may be called once to "precompute" the IV for
**              a given hashBitLen value, then by saving a copy of the context
**              the IV computation may be avoided in later calls.
**          Similarly, the function InitExt() may be called once per MAC key 
**              to precompute the MAC IV, then a copy of the context saved and
**              reused for each new MAC computation.
**/
int Skein_256_InitExt(Skein_256_Ctxt_t * ctx, size_t hashBitLen,
		      u64b_t treeInfo, const u08b_t * key, size_t keyBytes);

/*
**   Skein APIs for tree hash:
**		Final_Pad:  pad, do final block, but no OUTPUT type
**		Output:     do just the output stage
*/
#ifndef SKEIN_TREE_HASH
#define SKEIN_TREE_HASH (1)
#endif
#if  SKEIN_TREE_HASH
int Skein_256_Final_Pad(Skein_256_Ctxt_t * ctx, u08b_t * hashVal);

int Skein_256_Output(Skein_256_Ctxt_t * ctx, u08b_t * hashVal);
#endif

/*****************************************************************
** "Internal" Skein definitions
**    -- not needed for sequential hashing API, but will be 
**           helpful for other uses of Skein (e.g., tree hash mode).
**    -- included here so that they can be shared between
**           reference and optimized code.
******************************************************************/

/* tweak word T[1]: bit field starting positions */
#define SKEIN_T1_BIT(BIT)       ((BIT) - 64)	/* offset 64 because it's the second word  */

#define SKEIN_T1_POS_TREE_LVL   SKEIN_T1_BIT(112)	/* bits 112..118: level in hash tree       */
#define SKEIN_T1_POS_BIT_PAD    SKEIN_T1_BIT(119)	/* bit  119     : partial final input byte */
#define SKEIN_T1_POS_BLK_TYPE   SKEIN_T1_BIT(120)	/* bits 120..125: type field               */
#define SKEIN_T1_POS_FIRST      SKEIN_T1_BIT(126)	/* bits 126     : first block flag         */
#define SKEIN_T1_POS_FINAL      SKEIN_T1_BIT(127)	/* bit  127     : final block flag         */

/* tweak word T[1]: flag bit definition(s) */
#define SKEIN_T1_FLAG_FIRST     (((u64b_t)  1 ) << SKEIN_T1_POS_FIRST)
#define SKEIN_T1_FLAG_FINAL     (((u64b_t)  1 ) << SKEIN_T1_POS_FINAL)
#define SKEIN_T1_FLAG_BIT_PAD   (((u64b_t)  1 ) << SKEIN_T1_POS_BIT_PAD)

/* tweak word T[1]: tree level bit field mask */
#define SKEIN_T1_TREE_LVL_MASK  (((u64b_t)0x7F) << SKEIN_T1_POS_TREE_LVL)
#define	SKEIN_T1_TREE_LEVEL(n)  (((u64b_t) (n)) << SKEIN_T1_POS_TREE_LVL)

/* tweak word T[1]: block type field */
#define SKEIN_BLK_TYPE_KEY      ( 0)	/* key, for MAC and KDF */
#define SKEIN_BLK_TYPE_CFG      ( 4)	/* configuration block */
#define SKEIN_BLK_TYPE_PERS     ( 8)	/* personalization string */
#define SKEIN_BLK_TYPE_PK       (12)	/* public key (for digital signature hashing) */
#define SKEIN_BLK_TYPE_KDF      (16)	/* key identifier for KDF */
#define SKEIN_BLK_TYPE_NONCE    (20)	/* nonce for PRNG */
#define SKEIN_BLK_TYPE_MSG      (48)	/* message processing */
#define SKEIN_BLK_TYPE_OUT      (63)	/* output stage */
#define SKEIN_BLK_TYPE_MASK     (63)	/* bit field mask */

#define SKEIN_T1_BLK_TYPE(T)   (((u64b_t) (SKEIN_BLK_TYPE_##T)) << SKEIN_T1_POS_BLK_TYPE)
#define SKEIN_T1_BLK_TYPE_KEY   SKEIN_T1_BLK_TYPE(KEY)	/* key, for MAC and KDF */
#define SKEIN_T1_BLK_TYPE_CFG   SKEIN_T1_BLK_TYPE(CFG)	/* configuration block */
#define SKEIN_T1_BLK_TYPE_PERS  SKEIN_T1_BLK_TYPE(PERS)	/* personalization string */
#define SKEIN_T1_BLK_TYPE_PK    SKEIN_T1_BLK_TYPE(PK)	/* public key (for digital signature hashing) */
#define SKEIN_T1_BLK_TYPE_KDF   SKEIN_T1_BLK_TYPE(KDF)	/* key identifier for KDF */
#define SKEIN_T1_BLK_TYPE_NONCE SKEIN_T1_BLK_TYPE(NONCE)	/* nonce for PRNG */
#define SKEIN_T1_BLK_TYPE_MSG   SKEIN_T1_BLK_TYPE(MSG)	/* message processing */
#define SKEIN_T1_BLK_TYPE_OUT   SKEIN_T1_BLK_TYPE(OUT)	/* output stage */
#define SKEIN_T1_BLK_TYPE_MASK  SKEIN_T1_BLK_TYPE(MASK)	/* field bit mask */

#define SKEIN_T1_BLK_TYPE_CFG_FINAL       (SKEIN_T1_BLK_TYPE_CFG | SKEIN_T1_FLAG_FINAL)
#define SKEIN_T1_BLK_TYPE_OUT_FINAL       (SKEIN_T1_BLK_TYPE_OUT | SKEIN_T1_FLAG_FINAL)

#define SKEIN_VERSION           (1)

#ifndef SKEIN_ID_STRING_LE	/* allow compile-time personalization */
#define SKEIN_ID_STRING_LE      (0x33414853)	/* "SHA3" (little-endian) */
#endif

#define SKEIN_MK_64(hi32,lo32)  ((lo32) + (((u64b_t) (hi32)) << 32))
#define SKEIN_SCHEMA_VER        SKEIN_MK_64(SKEIN_VERSION,SKEIN_ID_STRING_LE)
#define SKEIN_KS_PARITY         SKEIN_MK_64(0x55555555,0x55555555)

/* bit field definitions in config block treeInfo word */
#define SKEIN_CFG_TREE_LEAF_SIZE_POS  ( 0)
#define SKEIN_CFG_TREE_NODE_SIZE_POS  ( 8)
#define SKEIN_CFG_TREE_MAX_LEVEL_POS  (16)

#define SKEIN_CFG_TREE_LEAF_SIZE_MSK  ((u64b_t) 0xFF) << SKEIN_CFG_TREE_LEAF_SIZE_POS)
#define SKEIN_CFG_TREE_NODE_SIZE_MSK  ((u64b_t) 0xFF) << SKEIN_CFG_TREE_NODE_SIZE_POS)
#define SKEIN_CFG_TREE_MAX_LEVEL_MSK  ((u64b_t) 0xFF) << SKEIN_CFG_TREE_MAX_LEVEL_POS)

#define SKEIN_CFG_TREE_INFO_SEQUENTIAL (0)	/* use as treeInfo in InitExt() call for sequential processing */
#define SKEIN_CFG_TREE_INFO(leaf,node,maxLevel) ((u64b_t) ((leaf) | ((node) << 8) | ((maxLevel) << 16)))

/*
**   Skein macros for getting/setting tweak words, etc.
**   These are useful for partial input bytes, hash tree init/update, etc.
**/
#define Skein_Get_Tweak(ctxPtr,TWK_NUM)         ((ctxPtr)->h.T[TWK_NUM])
#define Skein_Set_Tweak(ctxPtr,TWK_NUM,tVal)    {(ctxPtr)->h.T[TWK_NUM] = (tVal);}

#define Skein_Get_T0(ctxPtr)    Skein_Get_Tweak(ctxPtr,0)
#define Skein_Get_T1(ctxPtr)    Skein_Get_Tweak(ctxPtr,1)
#define Skein_Set_T0(ctxPtr,T0) Skein_Set_Tweak(ctxPtr,0,T0)
#define Skein_Set_T1(ctxPtr,T1) Skein_Set_Tweak(ctxPtr,1,T1)

/* set both tweak words at once */
#define Skein_Set_T0_T1(ctxPtr,T0,T1)           \
    {                                           \
    Skein_Set_T0(ctxPtr,(T0));                  \
    Skein_Set_T1(ctxPtr,(T1));                  \
    }

#define Skein_Set_Type(ctxPtr,BLK_TYPE)         \
    Skein_Set_T1(ctxPtr,SKEIN_T1_BLK_TYPE_##BLK_TYPE)

/* set up for starting with a new type: h.T[0]=0; h.T[1] = NEW_TYPE; h.bCnt=0; */
#define Skein_Start_New_Type(ctxPtr,BLK_TYPE)   \
    { Skein_Set_T0_T1(ctxPtr,0,SKEIN_T1_FLAG_FIRST | SKEIN_T1_BLK_TYPE_##BLK_TYPE); (ctxPtr)->h.bCnt=0; }

#define Skein_Clear_First_Flag(hdr)	     { (hdr).T[1] &= ~SKEIN_T1_FLAG_FIRST;       }
#define Skein_Set_Bit_Pad_Flag(hdr)      { (hdr).T[1] |=  SKEIN_T1_FLAG_BIT_PAD;     }

#define Skein_Set_Tree_Level(hdr,height) { (hdr).T[1] |= SKEIN_T1_TREE_LEVEL(height);}

/*****************************************************************
** "Internal" Skein definitions for debugging and error checking
******************************************************************/
#ifdef  SKEIN_DEBUG		/* examine/display intermediate values? */
#include "skein_debug.h"
#else				/* default is no callouts */
#define Skein_Show_Block(bits,ctx,X,blkPtr,wPtr,ksEvenPtr,ksOddPtr)
#define Skein_Show_Round(bits,ctx,r,X)
#define Skein_Show_R_Ptr(bits,ctx,r,X_ptr)
#define Skein_Show_Final(bits,ctx,cnt,outPtr)
#define Skein_Show_Key(bits,ctx,key,keyBytes)
#endif

#ifndef SKEIN_ERR_CHECK		/* run-time checks (e.g., bad params, uninitialized context)? */
#define Skein_Assert(x,retCode)	/* default: ignore all Asserts, for performance */
#define Skein_assert(x)
#elif   defined(SKEIN_ASSERT)
#include <assert.h>
#define Skein_Assert(x,retCode) assert(x)
#define Skein_assert(x)         assert(x)
#else
#include <assert.h>
#define Skein_Assert(x,retCode) { if (!(x)) return retCode; }	/*  caller  error */
#define Skein_assert(x)         assert(x)	/* internal error */
#endif

/*****************************************************************
** Skein block function constants (shared across Ref and Opt code)
******************************************************************/
enum {
	/* Skein_256 round rotation constants */
	R_256_0_0 = 5, R_256_0_1 = 56,
	R_256_1_0 = 36, R_256_1_1 = 28,
	R_256_2_0 = 13, R_256_2_1 = 46,
	R_256_3_0 = 58, R_256_3_1 = 44,
	R_256_4_0 = 26, R_256_4_1 = 20,
	R_256_5_0 = 53, R_256_5_1 = 35,
	R_256_6_0 = 11, R_256_6_1 = 42,
	R_256_7_0 = 59, R_256_7_1 = 50,

	/* Skein_512 round rotation constants */
	R_512_0_0 = 38, R_512_0_1 = 30, R_512_0_2 = 50, R_512_0_3 = 53,
	R_512_1_0 = 48, R_512_1_1 = 20, R_512_1_2 = 43, R_512_1_3 = 31,
	R_512_2_0 = 34, R_512_2_1 = 14, R_512_2_2 = 15, R_512_2_3 = 27,
	R_512_3_0 = 26, R_512_3_1 = 12, R_512_3_2 = 58, R_512_3_3 = 7,
	R_512_4_0 = 33, R_512_4_1 = 49, R_512_4_2 = 8, R_512_4_3 = 42,
	R_512_5_0 = 39, R_512_5_1 = 27, R_512_5_2 = 41, R_512_5_3 = 14,
	R_512_6_0 = 29, R_512_6_1 = 26, R_512_6_2 = 11, R_512_6_3 = 9,
	R_512_7_0 = 33, R_512_7_1 = 51, R_512_7_2 = 39, R_512_7_3 = 35,

	/* Skein1024 round rotation constants */
	R1024_0_0 = 55, R1024_0_1 = 43, R1024_0_2 = 37, R1024_0_3 =
	    40, R1024_0_4 = 16, R1024_0_5 = 22, R1024_0_6 = 38, R1024_0_7 = 12,
	R1024_1_0 = 25, R1024_1_1 = 25, R1024_1_2 = 46, R1024_1_3 =
	    13, R1024_1_4 = 14, R1024_1_5 = 13, R1024_1_6 = 52, R1024_1_7 = 57,
	R1024_2_0 = 33, R1024_2_1 = 8, R1024_2_2 = 18, R1024_2_3 =
	    57, R1024_2_4 = 21, R1024_2_5 = 12, R1024_2_6 = 32, R1024_2_7 = 54,
	R1024_3_0 = 34, R1024_3_1 = 43, R1024_3_2 = 25, R1024_3_3 =
	    60, R1024_3_4 = 44, R1024_3_5 = 9, R1024_3_6 = 59, R1024_3_7 = 34,
	R1024_4_0 = 28, R1024_4_1 = 7, R1024_4_2 = 47, R1024_4_3 =
	    48, R1024_4_4 = 51, R1024_4_5 = 9, R1024_4_6 = 35, R1024_4_7 = 41,
	R1024_5_0 = 17, R1024_5_1 = 6, R1024_5_2 = 18, R1024_5_3 =
	    25, R1024_5_4 = 43, R1024_5_5 = 42, R1024_5_6 = 40, R1024_5_7 = 15,
	R1024_6_0 = 58, R1024_6_1 = 7, R1024_6_2 = 32, R1024_6_3 =
	    45, R1024_6_4 = 19, R1024_6_5 = 18, R1024_6_6 = 2, R1024_6_7 = 56,
	R1024_7_0 = 47, R1024_7_1 = 49, R1024_7_2 = 27, R1024_7_3 =
	    58, R1024_7_4 = 37, R1024_7_5 = 48, R1024_7_6 = 53, R1024_7_7 = 56
};

#ifndef SKEIN_ROUNDS
#define SKEIN_256_ROUNDS_TOTAL (72)	/* number of rounds for the different block sizes */
#define SKEIN_512_ROUNDS_TOTAL (72)
#define SKEIN1024_ROUNDS_TOTAL (80)
#else				/* allow command-line define in range 8*(5..14)   */
#define SKEIN_256_ROUNDS_TOTAL (8*((((SKEIN_ROUNDS/100) + 5) % 10) + 5))
#define SKEIN_512_ROUNDS_TOTAL (8*((((SKEIN_ROUNDS/ 10) + 5) % 10) + 5))
#define SKEIN1024_ROUNDS_TOTAL (8*((((SKEIN_ROUNDS    ) + 5) % 10) + 5))
#endif
/*
***************** Pre-computed Skein IVs *******************
**
** NOTE: these values are not "magic" constants, but
** are generated using the Threefish block function.
** They are pre-computed here only for speed; i.e., to
** avoid the need for a Threefish call during Init().
**
** The IV for any fixed hash length may be pre-computed.
** Only the most common values are included here.
**
************************************************************
**/

#define MK_64 SKEIN_MK_64

/* blkSize =  256 bits. hashSize =  128 bits */
const u64b_t SKEIN_256_IV_128[] = {
	MK_64(0x302F7EA2, 0x3D7FE2E1),
	MK_64(0xADE4683A, 0x6913752B),
	MK_64(0x975CFABE, 0xF208AB0A),
	MK_64(0x2AF4BA95, 0xF831F55B)
};

/* blkSize =  256 bits. hashSize =  160 bits */
const u64b_t SKEIN_256_IV_160[] = {
	MK_64(0xA38A0D80, 0xA3687723),
	MK_64(0xB73CDB6A, 0x5963FFC9),
	MK_64(0x9633E8EA, 0x07A1B447),
	MK_64(0xCA0ED09E, 0xC9529C22)
};

/* blkSize =  256 bits. hashSize =  224 bits */
const u64b_t SKEIN_256_IV_224[] = {
	MK_64(0xB8092969, 0x9AE0F431),
	MK_64(0xD340DC14, 0xA06929DC),
	MK_64(0xAE866594, 0xBDE4DC5A),
	MK_64(0x339767C2, 0x5A60EA1D)
};

/* blkSize =  256 bits. hashSize =  256 bits */
const u64b_t SKEIN_256_IV_256[] = {
	MK_64(0x38851268, 0x0E660046),
	MK_64(0x4B72D5DE, 0xC5A8FF01),
	MK_64(0x281A9298, 0xCA5EB3A5),
	MK_64(0x54CA5249, 0xF46070C4)
};

/* blkSize =  512 bits. hashSize =  128 bits */
const u64b_t SKEIN_512_IV_128[] = {
	MK_64(0x00CE52E8, 0x677FE944),
	MK_64(0x57BA6C22, 0x68473BB5),
	MK_64(0xF083280E, 0x738FA141),
	MK_64(0xDF6DFC06, 0x17C956D7),
	MK_64(0x00332C15, 0xB46046F1),
	MK_64(0x1AE41A9A, 0x2B63DC55),
	MK_64(0xC812A302, 0x4B909692),
	MK_64(0x4C7B98DF, 0x51914760)
};

/* blkSize =  512 bits. hashSize =  160 bits */
const u64b_t SKEIN_512_IV_160[] = {
	MK_64(0x53BE57B8, 0x4A128B82),
	MK_64(0xDE248BCE, 0x8A7DF878),
	MK_64(0xBB05FB93, 0x0BE80D77),
	MK_64(0x750B3BDF, 0x8B056751),
	MK_64(0x827F0B05, 0x0B0EE024),
	MK_64(0x4D597EE9, 0xAE3F2774),
	MK_64(0x0F706962, 0x3A958D5B),
	MK_64(0xEB247D22, 0xF0B63D34)
};

/* blkSize =  512 bits. hashSize =  224 bits */
const u64b_t SKEIN_512_IV_224[] = {
	MK_64(0x11AE9072, 0xA87174E4),
	MK_64(0xF26D313F, 0xE0DA4261),
	MK_64(0xC686CC9A, 0x40FBBED9),
	MK_64(0xDC8BECEB, 0xA813B217),
	MK_64(0xBF420F3A, 0x03181324),
	MK_64(0x05700E28, 0x9ED73F27),
	MK_64(0x24B7ED7A, 0x8806891E),
	MK_64(0xE6555798, 0xB3A5A6D1)
};

/* blkSize =  512 bits. hashSize =  256 bits */
const u64b_t SKEIN_512_IV_256[] = {
	MK_64(0xB28464F1, 0xC2832686),
	MK_64(0xB78CB2E6, 0x6662F7E0),
	MK_64(0x3EDFE63C, 0x9ABE6E00),
	MK_64(0xD74EA633, 0x3F3C51DE),
	MK_64(0x3E591E83, 0x0D2A4647),
	MK_64(0xF76F4942, 0xB65B2E3F),
	MK_64(0x1DF2D635, 0x89027150),
	MK_64(0x8BAC70D7, 0x8D7D70F6)
};

/* blkSize =  512 bits. hashSize =  384 bits */
const u64b_t SKEIN_512_IV_384[] = {
	MK_64(0xE34B2AD7, 0xBC712975),
	MK_64(0x7808E500, 0x49E75965),
	MK_64(0x33529B8A, 0x121A306C),
	MK_64(0xEF9283AF, 0x1C1D392B),
	MK_64(0xD2EABFDE, 0xDB670B29),
	MK_64(0x4302B353, 0xD3FD1EF3),
	MK_64(0xCDA26096, 0x33B940D1),
	MK_64(0x20717333, 0x3B7C73E1)
};

/* blkSize =  512 bits. hashSize =  512 bits */
const u64b_t SKEIN_512_IV_512[] = {
	MK_64(0x6941D6EA, 0x3247F947),
	MK_64(0x181D627E, 0x9AD667FE),
	MK_64(0x0D44C453, 0x719EF322),
	MK_64(0xFA7B1E15, 0x447A7567),
	MK_64(0x90BFA06F, 0xEEC4C873),
	MK_64(0x35326748, 0xE26162B0),
	MK_64(0x5DB2DE78, 0x8D2839A6),
	MK_64(0xA9784A13, 0x143FD2EC)
};

/* blkSize = 1024 bits. hashSize =  384 bits */
const u64b_t SKEIN1024_IV_384[] = {
	MK_64(0xD5A49D15, 0x693CBF16),
	MK_64(0xD4ADA437, 0xABB0CF5B),
	MK_64(0x1EF34E38, 0x69EADDD0),
	MK_64(0x371CD4A3, 0xE5636211),
	MK_64(0x6CF32384, 0x9ACA1AD1),
	MK_64(0x8A9F46F3, 0xE2FAB037),
	MK_64(0x81A93DDA, 0xD6644234),
	MK_64(0x3F70DC2D, 0x627FB49C),
	MK_64(0x656B221D, 0xBF08239C),
	MK_64(0xCE783FD2, 0x9C1F9CE0),
	MK_64(0xBB858FB9, 0xE544DE66),
	MK_64(0x1CB13E52, 0xDFF040F2),
	MK_64(0x545B4070, 0xDDF9D479),
	MK_64(0xE0EAB0DE, 0x91CB6F55),
	MK_64(0x90559C8A, 0x2A156052),
	MK_64(0x337B58B9, 0x26302CDD)
};

/* blkSize = 1024 bits. hashSize =  512 bits */
const u64b_t SKEIN1024_IV_512[] = {
	MK_64(0xDE14C055, 0x29D4FE16),
	MK_64(0x26B03D82, 0x09DD7258),
	MK_64(0x0A9110E4, 0x70D5CF62),
	MK_64(0xB55AFCB0, 0x17F4D158),
	MK_64(0x489743AA, 0xD4B1A19B),
	MK_64(0x2D4C86DC, 0x75F7701C),
	MK_64(0xD7CF34E9, 0x2A57F805),
	MK_64(0x7B73ACD8, 0x75C46BEC),
	MK_64(0xBE089B37, 0x3942959E),
	MK_64(0x4BD412E8, 0xB0889F42),
	MK_64(0x3D9775F2, 0xE8E4A933),
	MK_64(0x2F510422, 0x3CF96A79),
	MK_64(0xAB3CFE9B, 0x06E5BCC9),
	MK_64(0x58B86378, 0x5D883590),
	MK_64(0x71954E0F, 0xF33D5ABF),
	MK_64(0x1355211F, 0x6D1FF4AC)
};

/* blkSize = 1024 bits. hashSize = 1024 bits */
const u64b_t SKEIN1024_IV_1024[] = {
	MK_64(0xB57C075C, 0x2274D71A),
	MK_64(0x49450570, 0xE753364D),
	MK_64(0xB02AF3B3, 0xB59DE329),
	MK_64(0xA16F7DD0, 0x498B1230),
	MK_64(0x3420E7B2, 0xFF686AD6),
	MK_64(0x6AF2877B, 0xF97739DF),
	MK_64(0x54AFC749, 0x5DB69891),
	MK_64(0x8FFB81FD, 0xD6A77CBF),
	MK_64(0xED481C34, 0x9CFD8F34),
	MK_64(0xC0930D63, 0x926E185E),
	MK_64(0x5EFD94B3, 0xC4A96A1B),
	MK_64(0xCE8BDB01, 0x82F8B4B0),
	MK_64(0xD32DBC15, 0x53245F64),
	MK_64(0x024AA6E5, 0x3E35A5B3),
	MK_64(0x58627674, 0x1F034DEC),
	MK_64(0xA4565435, 0xFF0C0315)
};

#ifndef SKEIN_USE_ASM
#define SKEIN_USE_ASM   (0)	/* default is all C code (no ASM) */
#endif

#ifndef SKEIN_LOOP
#define SKEIN_LOOP 001		/* default: unroll 256 and 512, but not 1024 */
#endif

#define BLK_BITS        (WCNT*64)	/* some useful definitions for code here */
#define KW_TWK_BASE     (0)
#define KW_KEY_BASE     (3)
#define ks              (kw + KW_KEY_BASE)
#define ts              (kw + KW_TWK_BASE)

#ifdef SKEIN_DEBUG
#define DebugSaveTweak(ctx) { ctx->h.T[0] = ts[0]; ctx->h.T[1] = ts[1]; }
#else
#define DebugSaveTweak(ctx)
#endif

typedef enum {
	SUCCESS = SKEIN_SUCCESS,
	FAIL = SKEIN_FAIL,
	BAD_HASHLEN = SKEIN_BAD_HASHLEN
} HashReturn;

/*****************************  Skein_256 ******************************/
#if !(SKEIN_USE_ASM & 256)
static void Skein_256_Process_Block(Skein_256_Ctxt_t * ctx,
				    const u08b_t * blkPtr, size_t blkCnt,
				    size_t byteCntAdd)
{				/* do it in C */
	enum {
		WCNT = SKEIN_256_STATE_WORDS
	};
#undef  RCNT
#define RCNT  (SKEIN_256_ROUNDS_TOTAL/8)

#ifdef  SKEIN_LOOP		/* configure how much to unroll the loop */
#define SKEIN_UNROLL_256 (((SKEIN_LOOP)/100)%10)
#else
#define SKEIN_UNROLL_256 (0)
#endif

#if SKEIN_UNROLL_256
#if (RCNT % SKEIN_UNROLL_256)
#error "Invalid SKEIN_UNROLL_256"	/* sanity check on unroll count */
#endif
	size_t r;
	u64b_t kw[WCNT + 4 + RCNT * 2];	/* key schedule words : chaining vars + tweak + "rotation" */
#else
	u64b_t kw[WCNT + 4];	/* key schedule words : chaining vars + tweak */
#endif
	u64b_t X0, X1, X2, X3;	/* local copy of context vars, for speed */
	u64b_t w[WCNT];		/* local copy of input block */
#ifdef SKEIN_DEBUG
	const u64b_t *Xptr[4];	/* use for debugging (help compiler put Xn in registers) */
	Xptr[0] = &X0;
	Xptr[1] = &X1;
	Xptr[2] = &X2;
	Xptr[3] = &X3;
#endif
	Skein_assert(blkCnt != 0);	/* never call with blkCnt == 0! */
	ts[0] = ctx->h.T[0];
	ts[1] = ctx->h.T[1];
	do {
		/* this implementation only supports 2**64 input bytes (no carry out here) */
		ts[0] += byteCntAdd;	/* update processed length */

		/* precompute the key schedule for this block */
		ks[0] = ctx->X[0];
		ks[1] = ctx->X[1];
		ks[2] = ctx->X[2];
		ks[3] = ctx->X[3];
		ks[4] = ks[0] ^ ks[1] ^ ks[2] ^ ks[3] ^ SKEIN_KS_PARITY;

		ts[2] = ts[0] ^ ts[1];

		Skein_Get64_LSB_First(w, blkPtr, WCNT);	/* get input block in little-endian format */
		DebugSaveTweak(ctx);
		Skein_Show_Block(BLK_BITS, &ctx->h, ctx->X, blkPtr, w, ks, ts);

		X0 = w[0] + ks[0];	/* do the first full key injection */
		X1 = w[1] + ks[1] + ts[0];
		X2 = w[2] + ks[2] + ts[1];
		X3 = w[3] + ks[3];

		Skein_Show_R_Ptr(BLK_BITS, &ctx->h, SKEIN_RND_KEY_INITIAL, Xptr);	/* show starting state values */

		blkPtr += SKEIN_256_BLOCK_BYTES;

		/* run the rounds */

#define Round256(p0,p1,p2,p3,ROT,rNum)                              \
    X##p0 += X##p1; X##p1 = RotL_64(X##p1,ROT##_0); X##p1 ^= X##p0; \
    X##p2 += X##p3; X##p3 = RotL_64(X##p3,ROT##_1); X##p3 ^= X##p2; \

#if SKEIN_UNROLL_256 == 0
#define R256(p0,p1,p2,p3,ROT,rNum)           /* fully unrolled */   \
    Round256(p0,p1,p2,p3,ROT,rNum)                                  \
    Skein_Show_R_Ptr(BLK_BITS,&ctx->h,rNum,Xptr);

#define I256(R)                                                     \
    X0   += ks[((R)+1) % 5];    /* inject the key schedule value */ \
    X1   += ks[((R)+2) % 5] + ts[((R)+1) % 3];                      \
    X2   += ks[((R)+3) % 5] + ts[((R)+2) % 3];                      \
    X3   += ks[((R)+4) % 5] +     (R)+1;                            \
    Skein_Show_R_Ptr(BLK_BITS,&ctx->h,SKEIN_RND_KEY_INJECT,Xptr);
#else				/* looping version */
#define R256(p0,p1,p2,p3,ROT,rNum)                                  \
    Round256(p0,p1,p2,p3,ROT,rNum)                                  \
    Skein_Show_R_Ptr(BLK_BITS,&ctx->h,4*(r-1)+rNum,Xptr);

#define I256(R)                                                     \
    X0   += ks[r+(R)+0];        /* inject the key schedule value */ \
    X1   += ks[r+(R)+1] + ts[r+(R)+0];                              \
    X2   += ks[r+(R)+2] + ts[r+(R)+1];                              \
    X3   += ks[r+(R)+3] +    r+(R)   ;                              \
    ks[r + (R)+4    ]   = ks[r+(R)-1];     /* rotate key schedule */\
    ts[r + (R)+2    ]   = ts[r+(R)-1];                              \
    Skein_Show_R_Ptr(BLK_BITS,&ctx->h,SKEIN_RND_KEY_INJECT,Xptr);

		for (r = 1; r < 2 * RCNT; r += 2 * SKEIN_UNROLL_256)	/* loop thru it */
#endif
		{
#define R256_8_rounds(R)                  \
        R256(0,1,2,3,R_256_0,8*(R) + 1);  \
        R256(0,3,2,1,R_256_1,8*(R) + 2);  \
        R256(0,1,2,3,R_256_2,8*(R) + 3);  \
        R256(0,3,2,1,R_256_3,8*(R) + 4);  \
        I256(2*(R));                      \
        R256(0,1,2,3,R_256_4,8*(R) + 5);  \
        R256(0,3,2,1,R_256_5,8*(R) + 6);  \
        R256(0,1,2,3,R_256_6,8*(R) + 7);  \
        R256(0,3,2,1,R_256_7,8*(R) + 8);  \
        I256(2*(R)+1);

			R256_8_rounds(0);

#define R256_Unroll_R(NN) ((SKEIN_UNROLL_256 == 0 && SKEIN_256_ROUNDS_TOTAL/8 > (NN)) || (SKEIN_UNROLL_256 > (NN)))

#if   R256_Unroll_R( 1)
			R256_8_rounds(1);
#endif
#if   R256_Unroll_R( 2)
			R256_8_rounds(2);
#endif
#if   R256_Unroll_R( 3)
			R256_8_rounds(3);
#endif
#if   R256_Unroll_R( 4)
			R256_8_rounds(4);
#endif
#if   R256_Unroll_R( 5)
			R256_8_rounds(5);
#endif
#if   R256_Unroll_R( 6)
			R256_8_rounds(6);
#endif
#if   R256_Unroll_R( 7)
			R256_8_rounds(7);
#endif
#if   R256_Unroll_R( 8)
			R256_8_rounds(8);
#endif
#if   R256_Unroll_R( 9)
			R256_8_rounds(9);
#endif
#if   R256_Unroll_R(10)
			R256_8_rounds(10);
#endif
#if   R256_Unroll_R(11)
			R256_8_rounds(11);
#endif
#if   R256_Unroll_R(12)
			R256_8_rounds(12);
#endif
#if   R256_Unroll_R(13)
			R256_8_rounds(13);
#endif
#if   R256_Unroll_R(14)
			R256_8_rounds(14);
#endif
#if  (SKEIN_UNROLL_256 > 14)
#error  "need more unrolling in Skein_256_Process_Block"
#endif
		}
		/* do the final "feedforward" xor, update context chaining vars */
		ctx->X[0] = X0 ^ w[0];
		ctx->X[1] = X1 ^ w[1];
		ctx->X[2] = X2 ^ w[2];
		ctx->X[3] = X3 ^ w[3];

		Skein_Show_Round(BLK_BITS, &ctx->h, SKEIN_RND_FEED_FWD, ctx->X);

		ts[1] &= ~SKEIN_T1_FLAG_FIRST;
	}
	while (--blkCnt);
	ctx->h.T[0] = ts[0];
	ctx->h.T[1] = ts[1];
}

#if defined(SKEIN_CODE_SIZE) || defined(SKEIN_PERF)
size_t Skein_256_Process_Block_CodeSize(void)
{
	return ((u08b_t *) Skein_256_Process_Block_CodeSize) -
	    ((u08b_t *) Skein_256_Process_Block);
}

uint_t Skein_256_Unroll_Cnt(void)
{
	return SKEIN_UNROLL_256;
}
#endif
#endif

typedef size_t DataLength;	/* bit count  type */
typedef u08b_t BitSequence;	/* bit stream type */

/*****************************************************************/
/*     256-bit Skein                                             */
/*****************************************************************/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* init the context for a straight hashing operation  */
int Skein_256_Init(Skein_256_Ctxt_t * ctx, size_t hashBitLen)
{
	union {
		u08b_t b[SKEIN_256_STATE_BYTES];
		u64b_t w[SKEIN_256_STATE_WORDS];
	} cfg;			/* config block */

	Skein_Assert(hashBitLen > 0, SKEIN_BAD_HASHLEN);
	ctx->h.hashBitLen = hashBitLen;	/* output hash bit count */

	switch (hashBitLen) {	/* use pre-computed values, where available */
#ifndef SKEIN_NO_PRECOMP
	case 256:
		memcpy(ctx->X, SKEIN_256_IV_256, sizeof(ctx->X));
		break;
	case 224:
		memcpy(ctx->X, SKEIN_256_IV_224, sizeof(ctx->X));
		break;
	case 160:
		memcpy(ctx->X, SKEIN_256_IV_160, sizeof(ctx->X));
		break;
	case 128:
		memcpy(ctx->X, SKEIN_256_IV_128, sizeof(ctx->X));
		break;
#endif
	default:
		/* here if there is no precomputed IV value available */
		/* build/process the config block, type == CONFIG (could be precomputed) */
		Skein_Start_New_Type(ctx, CFG_FINAL);	/* set tweaks: T0=0; T1=CFG | FINAL */

		cfg.w[0] = Skein_Swap64(SKEIN_SCHEMA_VER);	/* set the schema, version */
		cfg.w[1] = Skein_Swap64(hashBitLen);	/* hash result length in bits */
		cfg.w[2] = Skein_Swap64(SKEIN_CFG_TREE_INFO_SEQUENTIAL);
		memset(&cfg.w[3], 0, sizeof(cfg) - 3 * sizeof(cfg.w[0]));	/* zero pad config block */

		/* compute the initial chaining values from config block */
		memset(ctx->X, 0, sizeof(ctx->X));	/* zero the chaining variables */
		Skein_256_Process_Block(ctx, cfg.b, 1, sizeof(cfg));
		break;
	}
	/* The chaining vars ctx->X are now initialized for the given hashBitLen. */
	/* Set up to process the data message portion of the hash (default) */
	Skein_Start_New_Type(ctx, MSG);	/* T0=0, T1= MSG type */

	return SKEIN_SUCCESS;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* init the context for a MAC and/or tree hash operation */
/* [identical to Skein_256_Init() when keyBytes == 0 && treeInfo == SKEIN_CFG_TREE_INFO_SEQUENTIAL] */
int Skein_256_InitExt(Skein_256_Ctxt_t * ctx, size_t hashBitLen,
		      u64b_t treeInfo, const u08b_t * key, size_t keyBytes)
{
	union {
		u08b_t b[SKEIN_256_STATE_BYTES];
		u64b_t w[SKEIN_256_STATE_WORDS];
	} cfg;			/* config block */

	Skein_Assert(hashBitLen > 0, SKEIN_BAD_HASHLEN);
	Skein_Assert(keyBytes == 0 || key != NULL, SKEIN_FAIL);

	/* compute the initial chaining values ctx->X[], based on key */
	if (keyBytes == 0) {	/* is there a key? */
		memset(ctx->X, 0, sizeof(ctx->X));	/* no key: use all zeroes as key for config block */
	} else {		/* here to pre-process a key */

		Skein_assert(sizeof(cfg.b) >= sizeof(ctx->X));
		/* do a mini-Init right here */
		ctx->h.hashBitLen = 8 * sizeof(ctx->X);	/* set output hash bit count = state size */
		Skein_Start_New_Type(ctx, KEY);	/* set tweaks: T0 = 0; T1 = KEY type */
		memset(ctx->X, 0, sizeof(ctx->X));	/* zero the initial chaining variables */
		Skein_256_Update(ctx, key, keyBytes);	/* hash the key */
		Skein_256_Final(ctx, cfg.b);	/* put result into cfg.b[] */
		memcpy(ctx->X, cfg.b, sizeof(cfg.b));	/* copy over into ctx->X[] */
#if SKEIN_NEED_SWAP
		{
			uint_t i;
			for (i = 0; i < SKEIN_256_STATE_WORDS; i++)	/* convert key bytes to context words */
				ctx->X[i] = Skein_Swap64(ctx->X[i]);
		}
#endif
	}
	/* build/process the config block, type == CONFIG (could be precomputed for each key) */
	ctx->h.hashBitLen = hashBitLen;	/* output hash bit count */
	Skein_Start_New_Type(ctx, CFG_FINAL);

	memset(&cfg.w, 0, sizeof(cfg.w));	/* pre-pad cfg.w[] with zeroes */
	cfg.w[0] = Skein_Swap64(SKEIN_SCHEMA_VER);
	cfg.w[1] = Skein_Swap64(hashBitLen);	/* hash result length in bits */
	cfg.w[2] = Skein_Swap64(treeInfo);	/* tree hash config info (or SKEIN_CFG_TREE_INFO_SEQUENTIAL) */

	Skein_Show_Key(256, &ctx->h, key, keyBytes);

	/* compute the initial chaining values from config block */
	Skein_256_Process_Block(ctx, cfg.b, 1, sizeof(cfg));

	/* The chaining vars ctx->X are now initialized */
	/* Set up to process the data message portion of the hash (default) */
	ctx->h.bCnt = 0;	/* buffer b[] starts out empty */
	Skein_Start_New_Type(ctx, MSG);

	return SKEIN_SUCCESS;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* process the input bytes */
int Skein_256_Update(Skein_256_Ctxt_t * ctx, const u08b_t * msg,
		     size_t msgByteCnt)
{
	size_t n;

	Skein_Assert(ctx->h.bCnt <= SKEIN_256_BLOCK_BYTES, SKEIN_FAIL);	/* catch uninitialized context */

	/* process full blocks, if any */
	if (msgByteCnt + ctx->h.bCnt > SKEIN_256_BLOCK_BYTES) {
		if (ctx->h.bCnt) {	/* finish up any buffered message data */
			n = SKEIN_256_BLOCK_BYTES - ctx->h.bCnt;	/* # bytes free in buffer b[] */
			if (n) {
				Skein_assert(n < msgByteCnt);	/* check on our logic here */
				memcpy(&ctx->b[ctx->h.bCnt], msg, n);
				msgByteCnt -= n;
				msg += n;
				ctx->h.bCnt += n;
			}
			Skein_assert(ctx->h.bCnt == SKEIN_256_BLOCK_BYTES);
			Skein_256_Process_Block(ctx, ctx->b, 1,
						SKEIN_256_BLOCK_BYTES);
			ctx->h.bCnt = 0;
		}
		/* now process any remaining full blocks, directly from input message data */
		if (msgByteCnt > SKEIN_256_BLOCK_BYTES) {
			n = (msgByteCnt - 1) / SKEIN_256_BLOCK_BYTES;	/* number of full blocks to process */
			Skein_256_Process_Block(ctx, msg, n,
						SKEIN_256_BLOCK_BYTES);
			msgByteCnt -= n * SKEIN_256_BLOCK_BYTES;
			msg += n * SKEIN_256_BLOCK_BYTES;
		}
		Skein_assert(ctx->h.bCnt == 0);
	}

	/* copy any remaining source message data bytes into b[] */
	if (msgByteCnt) {
		Skein_assert(msgByteCnt + ctx->h.bCnt <= SKEIN_256_BLOCK_BYTES);
		memcpy(&ctx->b[ctx->h.bCnt], msg, msgByteCnt);
		ctx->h.bCnt += msgByteCnt;
	}

	return SKEIN_SUCCESS;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* finalize the hash computation and output the result */
int Skein_256_Final(Skein_256_Ctxt_t * ctx, u08b_t * hashVal)
{
	size_t i, n, byteCnt;
	u64b_t X[SKEIN_256_STATE_WORDS];
	Skein_Assert(ctx->h.bCnt <= SKEIN_256_BLOCK_BYTES, SKEIN_FAIL);	/* catch uninitialized context */

	ctx->h.T[1] |= SKEIN_T1_FLAG_FINAL;	/* tag as the final block */
	if (ctx->h.bCnt < SKEIN_256_BLOCK_BYTES)	/* zero pad b[] if necessary */
		memset(&ctx->b[ctx->h.bCnt], 0,
		       SKEIN_256_BLOCK_BYTES - ctx->h.bCnt);

	Skein_256_Process_Block(ctx, ctx->b, 1, ctx->h.bCnt);	/* process the final block */

	/* now output the result */
	byteCnt = (ctx->h.hashBitLen + 7) >> 3;	/* total number of output bytes */

	/* run Threefish in "counter mode" to generate output */
	memset(ctx->b, 0, sizeof(ctx->b));	/* zero out b[], so it can hold the counter */
	memcpy(X, ctx->X, sizeof(X));	/* keep a local copy of counter mode "key" */
	for (i = 0; i * SKEIN_256_BLOCK_BYTES < byteCnt; i++) {
		((u64b_t *) ctx->b)[0] = Skein_Swap64((u64b_t) i);	/* build the counter block */
		Skein_Start_New_Type(ctx, OUT_FINAL);
		Skein_256_Process_Block(ctx, ctx->b, 1, sizeof(u64b_t));	/* run "counter mode" */
		n = byteCnt - i * SKEIN_256_BLOCK_BYTES;	/* number of output bytes left to go */
		if (n >= SKEIN_256_BLOCK_BYTES)
			n = SKEIN_256_BLOCK_BYTES;
		Skein_Put64_LSB_First(hashVal + i * SKEIN_256_BLOCK_BYTES, ctx->X, n);	/* "output" the ctr mode bytes */
		Skein_Show_Final(256, &ctx->h, n,
				 hashVal + i * SKEIN_256_BLOCK_BYTES);
		memcpy(ctx->X, X, sizeof(X));	/* restore the counter mode key for next time */
	}
	return SKEIN_SUCCESS;
}

/******************************************************************/
/*     AHS API code                                               */
/******************************************************************/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* select the context size and init the context */
static int Init(hashState * state, int hashbitlen)
{
	if (hashbitlen <= SKEIN_256_NIST_MAX_HASHBITS) {
		Skein_Assert(hashbitlen > 0, BAD_HASHLEN);
		state->statebits = 64 * SKEIN_256_STATE_WORDS;
		return Skein_256_Init(&state->u.ctx_256, (size_t) hashbitlen);
	} else {
		/* error */
		return FAIL;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* process data to be hashed */
static int Update(hashState * state, const BitSequence * data,
		  DataLength databitlen)
{
	/* only the final Update() call is allowed do partial bytes, else assert an error */
	Skein_Assert((state->u.h.T[1] & SKEIN_T1_FLAG_BIT_PAD) == 0
		     || databitlen == 0, FAIL);

	Skein_Assert(state->statebits % 256 == 0
		     && (state->statebits - 256) < 1024, FAIL);
	if ((databitlen & 7) == 0) {	/* partial bytes? */
		switch ((state->statebits >> 8) & 3) {
		case 1:
			return Skein_256_Update(&state->u.ctx_256, data,
						databitlen >> 3);
		default:
			return FAIL;
		}
	} else {		/* handle partial final byte */
		size_t bCnt = (databitlen >> 3) + 1;	/* number of bytes to handle (nonzero here!) */
		u08b_t b, mask;

		mask = (u08b_t) (1u << (7 - (databitlen & 7)));	/* partial byte bit mask */
		b = (u08b_t) ((data[bCnt - 1] & (0 - mask)) | mask);	/* apply bit padding on final byte */

		switch ((state->statebits >> 8) & 3) {
		case 1:
			Skein_256_Update(&state->u.ctx_256, data, bCnt - 1);	/* process all but the final byte    */
			Skein_256_Update(&state->u.ctx_256, &b, 1);	/* process the (masked) partial byte */
			break;
		default:
			return FAIL;
		}
		Skein_Set_Bit_Pad_Flag(state->u.h);	/* set tweak flag for the final call */

		return SUCCESS;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* finalize hash computation and output the result (hashbitlen bits) */
static int Final(hashState * state, BitSequence * hashval)
{
	Skein_Assert(state->statebits % 256 == 0
		     && (state->statebits - 256) < 1024, FAIL);
	switch ((state->statebits >> 8) & 3) {
	case 1:
		return Skein_256_Final(&state->u.ctx_256, hashval);
	default:
		return FAIL;
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* all-in-one hash function */
static int Hash(int hashbitlen, const BitSequence * data,	/* all-in-one call */
		DataLength databitlen, BitSequence * hashval)
{
	hashState state;
	HashReturn r = Init(&state, hashbitlen);
	if (r == SUCCESS) {	/* these calls do not fail when called properly */
		r = Update(&state, data, databitlen);
		Final(&state, hashval);
	}
	return r;
}

#define	SKEIN_BLOCK_SIZE 32

uint32_t lhi_hash_skein256(const uint8_t * data, uint32_t len)
{
	uint32_t ret;
	uint8_t hash[SKEIN_BLOCK_SIZE];

	Hash(SKEIN_BLOCK_SIZE * 8, data, len * 8, hash);

	/* truncate hash - consider the 32 higher order bits */
	memcpy(&ret, hash, sizeof(ret));

	return ret;
}
