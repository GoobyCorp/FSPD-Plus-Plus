#ifndef _TYPES_HPP
#define _TYPES_HPP
typedef unsigned char  BYTE, uint8, u8;
typedef unsigned short uint16, u16, ushort, USHORT;
typedef unsigned int   uint32, u32;
typedef unsigned long  ulong, ULONG;

// pointers
typedef char*   PCHAR;
typedef BYTE*   PBYTE;
typedef uint16* PUSHORT;
typedef ulong*  PULONG;

// constants
#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT 7717
#define FSP_HSIZE 12
#define FSP_SPACE 1024
#define FSP_MAXSPACE FSP_HSIZE + FSP_SPACE
#define TRUE 1
#define FALSE 0

enum {
	ERROR_NONE = 0,
	ERROR_SOCKET_CREATE_FAILED,
	ERROR_SETSOCKOPT_FAILED,
	ERROR_SOCKET_BIND_FAILED,
};
#endif