class RDIRENT {
    public:
        UINT32 FileTime = 0;
        UINT32 FileSize = 0;
        PCHAR pcFileName = 0;
        RDIRENT_TYPE Type = RDTYPE_END;

        // functions - class
        RDIRENT();
        ~RDIRENT();
        UINT32 GetSize();
        UINT32 Pack(PBYTE pbOut);
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
        UINT32 Pack(PBYTE pbOut);
        // functions - static
        static FSPRequest Parse(PBYTE pbData, UINT32 cbData);
        static FSPRequest Create(FSP_COMMAND cmd, PBYTE pbData, USHORT cbData, USHORT pos = 0, USHORT seq = 0);
};