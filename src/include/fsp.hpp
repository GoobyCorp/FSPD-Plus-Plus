#pragma once

class RDIRENT {
    public:
        PRDIRENT_HDR pHdr = 0;
        PCHAR pcFileName = 0;

        // functions - class
        RDIRENT();
        ~RDIRENT();
        UINT32 GetSize();
        Alloc* Pack();
        VOID Free();
        // functions - static
        static RDIRENT* Create(PCHAR path);
        static RDIRENT* CreateSkip();
        static RDIRENT* CreateEnd();
};

class STAT {
    public:
        PSTAT_HDR pHdr = 0;

        // functions - class
        STAT();
        ~STAT();
        UINT32 GetSize();
        Alloc* Pack();
        VOID Free();
        // functions - static
        static STAT* Create(PCHAR path);
};

class FSPRequest {
    public:
        // pointers
        PFSP_HDR pHdr = 0;
        PBYTE pbData = 0 ;
        PBYTE pbExtra = 0;

        // variables
        UINT16 cbData = 0;
        UINT16 cbExtra = 0;
        UINT16 blkSize = 0;

        // command-specific
        PCHAR pcPassword = 0;
        PCHAR pcFilename = 0;
        PCHAR pcDirectory = 0;

        // functions - class
        FSPRequest();
        ~FSPRequest();
        UINT32 GetSize();
        Alloc* Pack();
        VOID PackAndSend(int srvSock, sockaddr* cliAddr, socklen_t cliAddrLen);
        VOID Free();
        // functions - static
        static FSPRequest* Parse(PBYTE pbData, UINT32 cbData);
        static FSPRequest* Create(BYTE cmd, PBYTE pbData, UINT16 cbData, PBYTE pbExtra, UINT16 cbExtra, UINT16 pos = 0, UINT16 seq = 0);
};