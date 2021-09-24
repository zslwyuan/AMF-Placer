#ifndef _READZIP
#define _READZIP

#include <cstdio>
#include <iostream>

// create a FILEBUF to read the unzip file pipe

struct FILEbuf : std::streambuf
{
    FILEbuf(FILE *fp) : fp_(fp)
    {
    }
    int underflow()
    {
        if (this->gptr() == this->egptr())
        {
            int size = fread(this->buffer_, 1, int(s_size), this->fp_);
            if (0 < size)
            {
                this->setg(this->buffer_, this->buffer_, this->buffer_ + size);
            }
        }
        return this->gptr() == this->egptr() ? traits_type::eof() : traits_type::to_int_type(*gptr());
    }
    FILE *fp_;
    enum
    {
        s_size = 1024
    };
    char buffer_[s_size];
};

#endif
