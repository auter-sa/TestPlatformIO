#include <SPIFFS.h>
#include "hexfile.h"
#include "constants.h"

//-----------------------------------------------------------------------------

HexFile::HexFile() {
}

HexFile::~HexFile() {
}

Schedule *HexFile::read(String hexFilename) {
    byte hexContents[MAX_BYTES];

	try {
        // Open HEX file
        if (! SPIFFS.begin(true)) {
            Serial.println("No se pudo montar SPIFFS.");
            Serial.println("Fin del programa.");
            return NULL;
        } 

        File file = SPIFFS.open(hexFilename.c_str(), "r");
        if (! file) {
            Serial.println("No se pudo abrir el archivo " + hexFilename);
            Serial.println("Fin del programa.");
            return NULL;
        }

        // Process HEX file
        String fileContents = "", line = "";
        int    i, lineBegin = 0, lineEnd, index = 0, bytesRead = 0, bytesReadTotal = 0;

        while (file.available()) {
            fileContents += (char) file.read();
        }
        file.close();

        while (bytesRead >= 0) {
            lineEnd = fileContents.indexOf('\n', lineBegin);
            if (lineEnd >= 0) {
                line = fileContents.substring(lineBegin, lineEnd);
            } else {
                line = fileContents.substring(lineBegin);
            }
            line.trim();
            line.toUpperCase();
            bytesRead = processHex(line, index, hexContents);
            if (bytesRead >= 0) {
                bytesReadTotal += bytesRead;
                if (lineEnd >= 0) {
                    lineBegin = lineEnd + 1;  // Move past the newline character
                } else {
                    break;
                }
            }
        }
        for (i = bytesReadTotal; i < MAX_BYTES; i ++) {
            hexContents[i] = 0;
        }

        Schedule *schedule = new Schedule(hexContents);
        return schedule;
    
	} catch (...) {
		return NULL;
	}
    return NULL;
}

int HexFile::processHex(String line, int &index, byte *contents) {
    String value;
    int    numBytes, pos, i;

    if (line.equals(":00000001FF")) {
        return -1;
    }

    try {
        numBytes = (int) strtol(line.substring(1, 3).c_str(), NULL, 16);
        for (i = 0; i < numBytes; i ++) {
            if (index < MAX_BYTES) {
                pos = (i * 2) + 9;
                contents[index ++] = (byte) strtol(line.substring(pos, pos + 2).c_str(), NULL, 16);
            } else {
                return i;
            }
        }
    } catch (...) {
    }
    return numBytes;
}

//-----------------------------------------------------------------------------
