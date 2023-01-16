#include "caudiocompression.h"

CAudioCompression::CAudioCompression(CRadioController* r):
    _radioController(r)
{
    _recordToFile = false;
}

CAudioCompression::CAudioCompression(CRadioController* r, string filename):
    _radioController(r),
    _Filename(filename)
{
    _recordToFile = true;
}