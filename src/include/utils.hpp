class Utils {
    public:
        static ULONG AddressToLong(const PCHAR addr);
        static VOID LongToAddress(PCHAR addrStr, ULONG addrLong);
        static VOID PrintHex(PBYTE data, UINT32 size);
        static int ByteSum(PBYTE data, UINT32 size);
        static BYTE CalcClientToServerChecksum(PBYTE data, UINT32 size);
        static BYTE CalcServerToClientChecksum(PBYTE data, UINT32 size);
        static UINT32 CalcPadSize(UINT32 size, UINT32 boundary);
        static BOOL IsDir(PCHAR path);
        static BOOL IsFile(PCHAR path);
        static PVOID AllocAndCopy(PVOID pData, UINT32 size);
        static PCHAR AllocAndCopyString(PCHAR str);
        static VOID CopyToVector(vector<BYTE>* pbVec, PBYTE pbData, UINT32 cbData);
        static VOID SwapFSPHeaderEndian(PFSP_HDR pHdr);
        static VOID SwapRDIRENTHeaderEndian(PRDIRENT_HDR pHdr);
};