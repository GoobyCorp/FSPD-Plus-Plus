#pragma once

class Cache {
    public:
        static PCHAR FspLastGetDir;
        static PCHAR FspLastGetFile;
        static vector<vector<BYTE>> PktQueue;
};