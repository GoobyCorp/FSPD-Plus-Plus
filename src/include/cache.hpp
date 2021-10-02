#ifndef _CACHE_HPP
#define _CACHE_HPP
class Cache {
    public:
        static PCHAR FspLastGetDir;
        static PCHAR FspLastGetFile;
        static vector<vector<BYTE>> PktQueue;
};
#endif