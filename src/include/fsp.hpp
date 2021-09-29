class FSPRequest {
    public:
        // pointers
        PFSP_HDR pHdr = 0;
        PBYTE pbData = 0 ;
        PBYTE pbExtra = 0;

        // variables
        USHORT cbData = 0;
        USHORT cbExtra = 0;

        // command-specific
        PCHAR pcUsername = 0;
        PCHAR pcPassword = 0;
        PCHAR pcFilename = 0;
        PCHAR pcDirectory = 0;

        // functions
        FSPRequest();
        ~FSPRequest();
        static FSPRequest Parse(PBYTE pbData, UINT32 cbData);
        static FSPRequest Create(FSP_COMMAND cmd, PBYTE pbData, USHORT cbData, USHORT pos = 0, USHORT seq = 0);
        USHORT GetSize();
        USHORT Serialize(PBYTE pbOut);
};