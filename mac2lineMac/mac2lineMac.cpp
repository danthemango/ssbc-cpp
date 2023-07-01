/*
    accepts .mac file,
    adds hex line number and hex bytecode to beginning
    of each line with binary byte code
    (all other lines are assumed to be comments and will be preserved)
*/ 

#include <iostream>
#include <fstream>
#include <string>
#include "../common.h"

int main(int argc, char** argv) {
    std::string line, binaryString, inFileName, outFileName;
    if(!tryParseIOFileNames(argc, argv, inFileName, outFileName)) {
        std::cerr << "Usage: " << argv[0] << " -i infile -o outfile" << std::endl;
        return 1;
    }

    std::ifstream inFile;
    inFile.open(inFileName);
    if(!inFile.is_open()) {
        std::cerr << "Error: could not open " << inFileName << std::endl;
        return 1;
    }
    std::ofstream outFile;
    outFile.open(outFileName);
    if(!outFile.is_open()) {
        std::cerr << "Error: could not open " << outFileName << std::endl;
        return 1;
    }

    int lineNum = 0;
    while(std::getline(inFile, line)) {
        std::string hexCode = "";
        if(tryParseBinaryString(line, binaryString)) {
            hexCode = bin2Hex(binaryString);
            outFile << twoBytes2hex(lineNum) << ' ' << hexCode << ' '<< line << std::endl;
            ++lineNum;
        } else {
            outFile << line << std::endl;
        }
    }

    inFile.close();
    outFile.close();
    return 0;
}
