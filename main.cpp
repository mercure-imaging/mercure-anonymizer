#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#include "dcmtk/dcmdata/dcpath.h"
#include "dcmtk/dcmdata/dcerror.h"
#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/ofstd/ofstd.h"
#include "dcmtk/dcmdata/dcspchrs.h"
#include "dcmtk/dcmdata/dctypes.h"

#define VERSION "0.1"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << std::endl;
        std::cout << "mercure-anonymizer ver " << VERSION << std::endl;
        std::cout << "--------------------------" << std::endl
                  << std::endl;
        std::cout << "Usage: [input folder] [output folder]" << std::endl
                  << std::endl;
        return 1;
    }
}
