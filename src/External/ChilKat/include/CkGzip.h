#pragma once

#include "WarIncCurlCompat.h"

extern "C"
{
#include "zlib.h"
}

class CkGzip
{
public:
    CkGzip()
    {
    }

    bool UnlockComponent(const char*)
    {
        return true;
    }

    bool CompressMemory(const CkByteData& in, CkByteData& out)
    {
        out.clear();

        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        int r = deflateInit2(
            &zs,
            Z_BEST_COMPRESSION,
            Z_DEFLATED,
            MAX_WBITS + 16,
            8,
            Z_DEFAULT_STRATEGY
        );

        if(r != Z_OK)
            return false;

        zs.next_in = (Bytef*)in.getData();
        zs.avail_in = (uInt)in.getSize();

        unsigned char buffer[32768];

        do
        {
            zs.next_out = buffer;
            zs.avail_out = sizeof(buffer);

            r = deflate(&zs, Z_FINISH);

            unsigned int have = sizeof(buffer) - zs.avail_out;
            if(have > 0)
                out.append(buffer, have);
        }
        while(r == Z_OK);

        deflateEnd(&zs);
        return r == Z_STREAM_END;
    }

    bool UncompressMemory(const CkByteData& in, CkByteData& out)
    {
        out.clear();

        z_stream zs;
        memset(&zs, 0, sizeof(zs));

        int r = inflateInit2(&zs, MAX_WBITS + 32);
        if(r != Z_OK)
            return false;

        zs.next_in = (Bytef*)in.getData();
        zs.avail_in = (uInt)in.getSize();

        unsigned char buffer[32768];

        do
        {
            zs.next_out = buffer;
            zs.avail_out = sizeof(buffer);

            r = inflate(&zs, 0);

            unsigned int have = sizeof(buffer) - zs.avail_out;
            if(have > 0)
                out.append(buffer, have);
        }
        while(r == Z_OK);

        inflateEnd(&zs);
        return r == Z_STREAM_END;
    }
};