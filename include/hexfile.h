#ifndef HEXFILE_H
#define HEXFILE_H

#include "schedule.h"

class HexFile {
    public:
        HexFile();
        ~HexFile();

        Schedule *read(String hexFilename);

    private:
        int processHex(String line, int &index, byte *contents);
};

#endif