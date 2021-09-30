class RDIRENT {
    public:
        PRDIRENT_HDR pHdr = 0;
        PCHAR pcFileName = 0;

        // functions - class
        RDIRENT();
        ~RDIRENT();
        UINT32 GetSize();
        PBYTE Pack(PUINT32 pcbOut);
        // functions - static
        static RDIRENT Create(PCHAR path);
        static RDIRENT CreateSkip();
        static RDIRENT CreateEnd();
};

class FSPRequest {
    public:
        // pointers
        PFSP_HDR pHdr = 0;
        PBYTE pbData = 0 ;
        PBYTE pbExtra = 0;

        // variables
        USHORT cbData = 0;
        USHORT cbExtra = 0;
        USHORT blkSize = 0;

        // command-specific
        PCHAR pcPassword = 0;
        PCHAR pcFilename = 0;
        PCHAR pcDirectory = 0;

        // functions - class
        FSPRequest();
        ~FSPRequest();
        UINT32 GetSize();
        PBYTE Pack(PUINT32 pcbOut);
        // functions - static
        static FSPRequest Parse(PBYTE pbData, UINT32 cbData);
        static FSPRequest Create(FSP_COMMAND cmd, PBYTE pbData, USHORT cbData, USHORT pos = 0, USHORT seq = 0);
};