#pragma once

// defines
#define es16(x) x = bswap16(x)
#define es32(x) x = bswap32(x)
#define es64(x) x = bswap64(x)

// primitives
typedef void           VOID;
typedef char           CHAR;
typedef unsigned char  BYTE, uint8, u8, UINT8;
typedef unsigned short uint16, u16, ushort, USHORT, UINT16;
typedef unsigned int   uint32, u32, UINT32, BOOL;
typedef unsigned long  ulong, ULONG;

// pointers - primitives
typedef void*   PVOID;
typedef CHAR*   PCHAR;
typedef BYTE*   PBYTE;
typedef UINT16* PUINT16;
typedef UINT32* PUINT32;
typedef ULONG*  PULONG;

// enums
typedef enum {
	ERROR_NONE = 0,
	ERROR_SOCKET_CREATE_FAILED,
	ERROR_SETSOCKOPT_FAILED,
	ERROR_SOCKET_BIND_FAILED,
	ERROR_RECEIVE_FAILED,
	ERROR_SEND_FAILED,
	ERROR_SIZE_OUT_OF_BOUNDS
};

typedef enum FSP_OFFSET {
	OFFS_CMD      = 0,  // 0-1
	OFFS_CKSM     = 1,  // 1-2
	OFFS_KEY      = 2,  // 2-4
	OFFS_SEQ      = 4,  // 4-6
	OFFS_DATA_LEN = 6,  // 6-8
	OFFS_POS      = 8   // 8-12
};

typedef enum FSP_COMMAND:BYTE {
	CC_VERSION   = 0x10,
	CC_ERR       = 0x40,
	CC_GET_DIR   = 0x41,
	CC_GET_FILE  = 0x42,
	CC_UP_LOAD   = 0x43,
	CC_INSTALL   = 0x44,
	CC_DEL_FILE  = 0x45,
	CC_DEL_DIR   = 0x46,
	CC_GET_PRO   = 0x47,
	CC_SET_PRO   = 0x48,
	CC_MAKE_DIR  = 0x49,
	CC_BYE       = 0x4A,
	CC_GRAB_FILE = 0x4B,
	CC_GRAB_DONE = 0x4C,
	CC_STAT      = 0x4D,
	CC_RENAME    = 0x4E,
	CC_CH_PASSW  = 0x4F,
	CC_LIMIT     = 0x80,
	CC_TEST      = 0x81
};

typedef enum RDIRENT_TYPE:BYTE {
	RDTYPE_END  = 0x00,
	RDTYPE_FILE = 0x01,
	RDTYPE_DIR  = 0x02,
	RDTYPE_SKIP = 0x2A
};

// structs
typedef struct FSP_HDR {
	BYTE   command;
	BYTE   checksum;
	UINT16 key;
	UINT16 sequence;
	UINT16 length;
	UINT32 position;
};

typedef struct RDIRENT_HDR {
	UINT32 FileTime;
	UINT32 FileSize;
	BYTE   Type;
};

typedef struct STAT_HDR {
	UINT32 FileTime;
	UINT32 FileSize;
	BYTE   Type;
};

typedef struct FSP_ALLOC {
	PBYTE  pbData;
	UINT32 cbData;
};

// pointers - structs
typedef FSP_HDR*     PFSP_HDR;
typedef RDIRENT_HDR* PRDIRENT_HDR;
typedef STAT_HDR*    PSTAT_HDR;
typedef FSP_ALLOC*  PFSP_ALLOC;

// constants
#define SERVER_ADDR "192.168.1.19"
#define SERVER_PORT 7717

#define FSP_HSIZE 12
#define FSP_SPACE 1024
#define FSP_MAXSPACE FSP_HSIZE + FSP_SPACE
#define IP_ADDR_MAX_LEN 16
#define TRUE 1
#define FALSE 0