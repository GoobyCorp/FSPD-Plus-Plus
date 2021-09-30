class Utils {
    public:
        static ULONG AddressToLong(const PCHAR addr);
        static void LongToAddress(PCHAR addrStr, ULONG addrLong);
        static void PrintHex(PBYTE data, UINT32 size);
        static int ByteSum(PBYTE data, UINT32 size);
        static BYTE ComputeChecksum(PBYTE data, UINT32 size, UINT32 sum);
        static BYTE CalcClientToServerChecksum(PBYTE data, UINT32 size);
        static BYTE CalcServerToClientChecksum(PBYTE data, UINT32 size);
        static UINT32 CalcPadSize(UINT32 size, UINT32 boundary);
        static BOOL IsDir(PCHAR path);
        static BOOL IsFile(PCHAR path);
        static void SwapFSPHeaderEndian(PFSP_HDR pHdr);
        static void SwapRDIRENTHeaderEndian(PRDIRENT_HDR pHdr);
};