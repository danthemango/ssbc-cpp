#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <fstream>

// try to parse the first 8 chars as binary values
// out_binaryString - the bits as a string value
// out_rest - the rest of the line
bool tryParseBinaryString(const std::string& line, std::string& out_binaryString) {
    std::string binaryString;
    int i = 0;
    while(i < 8) {
        if(i >= line.size() || (line[i] != '1' && line[i] != '0')) {
            return false;
        } else {
            binaryString.push_back(line[i]);
        }
        ++i;
    }
    out_binaryString = binaryString;
    return true;
}

// returns binary string to 4 bit parts
bool get4BitParts(const std::string& input, std::string& out_highPart, std::string& out_lowPart) {
    if(input.size() != 8) {
        return false;
    }
    out_highPart = input.substr(0, 4);
    out_lowPart = input.substr(4, 4);
}

// returns hex char from 4-bit string
char part2Hex(const std::string& input) {
    if     (input == "0000") { return '0'; }
    else if(input == "0001") { return '1'; }
    else if(input == "0010") { return '2'; }
    else if(input == "0011") { return '3'; }
    else if(input == "0100") { return '4'; }
    else if(input == "0101") { return '5'; }
    else if(input == "0110") { return '6'; }
    else if(input == "0111") { return '7'; }
    else if(input == "1000") { return '8'; }
    else if(input == "1001") { return '9'; }
    else if(input == "1010") { return 'A'; }
    else if(input == "1011") { return 'B'; }
    else if(input == "1100") { return 'C'; }
    else if(input == "1101") { return 'D'; }
    else if(input == "1110") { return 'E'; }
    else if(input == "1111") { return 'F'; }
    else { return 'x'; };
}

// returns hex char from the bottom 4-bits of input
char part2Hex(int input) {
    int part = input & 0xF;
    switch(part) {
        case 0x0: { return '0'; }
        case 0x1: { return '1'; }
        case 0x2: { return '2'; }
        case 0x3: { return '3'; }
        case 0x4: { return '4'; }
        case 0x5: { return '5'; }
        case 0x6: { return '6'; }
        case 0x7: { return '7'; }
        case 0x8: { return '8'; }
        case 0x9: { return '9'; }
        case 0xA: { return 'A'; }
        case 0xB: { return 'B'; }
        case 0xC: { return 'C'; }
        case 0xD: { return 'D'; }
        case 0xE: { return 'E'; }
        case 0xF: { return 'F'; }
    }
    return 'x';
}

// converts 8-bit binary string to 2 hex symbols
std::string bin2Hex(const std::string& binaryString) {
    std::string highPart, lowPart;
    get4BitParts(binaryString, highPart, lowPart);
    std::string result;
    result.push_back(part2Hex(highPart));
    result.push_back(part2Hex(lowPart));
    return result;
}

// returns 2 bytes into a hex string formatted like '0xFFFF'
std::string twoBytes2hex(int input) {
    std::string result;
    for(int i = 0; i < 4; i++) {
        result.insert(result.begin(), part2Hex(input));
        input >>= 4;
    }
    return result;
}

// returns true if there was an argument set, filling it's value to out_val
bool tryParseArg(const int& argc, char** argv, const std::string& flagString, std::string& out_val) {
    for(int i = 0; i < argc; i++) {
        if(flagString == argv[i] && argc > i+1) {
            out_val = argv[i+1];
            return true;
        }
    }
    return false;
}

// returns true if there was an argument set
bool tryParseArg(const int& argc, char** argv, const std::string& flagString) {
    for(int i = 0; i < argc; i++) {
        if(flagString == argv[i]) {
            return true;
        }
    }
    return false;
}

// fills infile and outfile name based on arguments
bool tryParseIOFileNames(const int& argc, char** argv, std::string& out_inFileName, std::string& out_outFileName) {
    if(!tryParseArg(argc, argv, "-i", out_inFileName)) {
        return false;
    } else if(!tryParseArg(argc, argv, "-o", out_outFileName)) {
        return false;
    } else {
        return true;
    }
}

#endif // COMMON_H
