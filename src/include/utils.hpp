class Utils {
    public:
        static ulong AddressToLong(const PCHAR addr);
        static PCHAR LongToAddress(ulong addr);
        static BYTE CalcClientToServerChecksum(PBYTE data, UINT32 size);
        static BYTE CalcServerToClientChecksum(PBYTE data, UINT32 size);
};