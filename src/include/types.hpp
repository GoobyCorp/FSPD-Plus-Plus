#ifndef _TYPES_HPP
#define _TYPES_HPP
typedef unsigned char  BYTE, uint8, u8, UINT8;
typedef unsigned short uint16, u16, ushort, USHORT, UINT16;
typedef unsigned int   uint32, u32, UINT32;
typedef unsigned long  ulong, ULONG;

// enums
enum {
	ERROR_NONE = 0,
	ERROR_SOCKET_CREATE_FAILED,
	ERROR_SETSOCKOPT_FAILED,
	ERROR_SOCKET_BIND_FAILED,
	ERROR_RECEIVE_FAILED,
	ERROR_SEND_FAILED,
	ERROR_THREAD_CREATE_FAILED,
	ERROR_THREAD_DETECH_FAILED,
	ERROR_THREAD_JOIN_FAILED
};

enum FSP_OFFSET {
	OFFS_CMD      = 0,  // 0-1
	OFFS_CKSM     = 1,  // 1-2
	OFFS_KEY      = 2,  // 2-4
	OFFS_SEQ      = 4,  // 4-6
	OFFS_DATA_LEN = 6,  // 6-8
	OFFS_POS      = 8   // 8-12
};

enum FSP_COMMAND:BYTE {
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

enum RDIRENT_TYPE {
	RDTYPE_END  = 0x00,
	RDTYPE_FILE = 0x01,
	RDTYPE_DIR  = 0x02,
	RDTYPE_SKIP = 0x2A
};

// structs
struct RECEIVE_CLIENT_ARGS {
	int sock;
};

struct FSP_HDR {
	FSP_COMMAND command;
	BYTE        checksum;
	USHORT      key;
	USHORT      sequence;
	USHORT      length;
	UINT32      position;
};

// pointers
typedef char*   PCHAR;
typedef BYTE*   PBYTE;
typedef uint16* PUSHORT;
typedef ulong*  PULONG;
typedef RECEIVE_CLIENT_ARGS* PRECEIVE_CLIENT_ARGS;
typedef FSP_HDR* PFSP_HDR;

// constants
#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT 7717
#define FSP_HSIZE 12
#define FSP_SPACE 1024
#define FSP_MAXSPACE FSP_HSIZE + FSP_SPACE
#define TRUE 1
#define FALSE 0
#endif