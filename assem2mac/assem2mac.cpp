/*
    accepts an ssbc assembly file and transforms it into machine code
*/

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "../common.h"

/*
    - every line of assembly code can have 0 or 1 instruction.
    - whitespace is ignored
    - comments are prepended by a semicolon (;) or a double-slash (//)
*/

/* 
    const std::string op_noop = "00000000",      // no operation
    const std::string op_halt = "00000001",      // halt
    const std::string op_pushimm = "00000010",   // push immediate
    const std::string op_pushext = "00000011",   // push external
    const std::string op_popinh = "00000100",    // pop inherent
    const std::string op_popext = "00000101",    // pop external
    const std::string op_jnz = "00000110",       // jump not zero
    const std::string op_jnn = "00000111",       // jump not negative
    const std::string op_add = "00001000",       // add
    const std::string op_sub = "00001001",       // subtract
    const std::string op_nor = "00001010",       // nor
*/

// opcodes
enum {
    op_noop = 0,      // no operation
    op_halt = 1,      // halt
    op_pushimm = 2,   // push immediate
    op_pushext = 3,   // push external
    op_popinh = 4,    // pop inherent
    op_popext = 5,    // pop external
    op_jnz = 6,       // jump not zero
    op_jnn = 7,       // jump not negative
    op_add = 8,       // add
    op_sub = 9,       // subtract
    op_nor = 10,      // nor
};

// converts number to 8-bit digit string
// e.g. n = 9 returns "00001001"
std::string num2macString(char n) {
    int mask = 0x80;
    std::string result;
    while(mask > 0) {
        int check = n & mask;
        if((n & mask) == 0) {
            result.push_back('0');
        } else {
            result.push_back('1');
        }
        mask >>= 1;
    }
    return result;
}

// returns the high 8 bits of the value
char getHighByte(int n) {
    n >>= 8;
    return n & 0xFF;
}

char getLowByte(int n) {
    return n & 0xFF;
}

// increments i for as long as there is whitespace in the line
bool skipSpace(const std::string& input, int& i) {
    bool spaceFound = false;
    while(i < input.size() && std::isspace(input[i])) {
        spaceFound = true;
        ++i;
    }
    return spaceFound;
}

// returns true if the next available character is c
// incrementing i to the char after if true
bool tryParseNextChar(const std::string& input, int& i, const char& c) {
    int resetI = i;
    skipSpace(input, i);
    if(i < input.size() && input[i] == c) {
        ++i;
        return true;
    } else {
        i = resetI;
        return false;
    }
}

// checks if the string is found at position i
// updates i to the char after a successful parse
// and returns true
bool tryParseNextString(const std::string& input, int& i, const std::string& pattern) {
    int resetI = i;
    int j = 0;
    skipSpace(input, i);
    while(i < input.size() && j < pattern.size()) {
        if (input[i] != pattern[j]) {
            i = resetI;
            return false;
        } else {
            ++i;
            ++j;
        }
    }
    if(j < pattern.size()) {
        i = resetI;
        return false;
    } else {
        return true;
    }
}

// returns true if we this is a token symbol
bool isTokenSymbol(const char& c) {
    return std::isalnum(c) || c == '-' || c == '+' || c == '_';
}

// fills the next available token, a token being any combination of alphanumeric symbols, dashes, and underscores
// returns false if none available
// updates i to the char after the token
bool tryFetchNextToken(const std::string& input, int& i, std::string& out_token) {
    int resetI = i;
    out_token = "";
    skipSpace(input, i);
    while(i < input.size() && isTokenSymbol(input[i])) {
        out_token.push_back(input[i]);
        ++i;
    }

    if(out_token.size() == 0) {
        i = resetI;
        return false;
    } else {
        return true;
    }
}

// returns true if the next token was found and has a given string pattern
// updating i to after the token if successfully parsed
bool tryParseNextToken(const std::string& input, int& i, const std::string& pattern) {
    int resetI = i;
    std::string token;

    if(!tryFetchNextToken(input, i, token) || pattern != token) {
        i = resetI;
        return false;
    } else {
        return true;
    }
}

// returns true if the input has a comment at position i
bool hasComment(const std::string& input, int i) {
    return tryParseNextString(input, i, "//") || tryParseNextString(input, i, ";");
}

// tries to parse a hex value as an int
// bool tryParseNextHex(const std::string& input, int& i, int& out_val) {
//     if(!tryParseNextString(input, i, "0x")) {
//         return false;
//     }
//     return help;
// }

// returns true if we have a hex symbol
bool isHex(const char& c) {
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

// returns true if there was a decimal integer in input at position i
// filling out_val with the resultant input
// and updates i to the char after a successful parse
bool tryParseDecimal(const std::string& input, int& out_val) {
    int i = 0;
    int sign = 1;
    if(i < input.size()) {
        if(input[i] == '-') {
            sign = -1;
            ++i;
        } else if(input[i] == '+') {
            ++i;
        }
    }

    if(i >= input.size() || !std::isdigit(input[i])) {
        return false;
    }

    int result = 0;
    while(i < input.size()) {
        if(!std::isdigit(input[i])) {
            return false;
        }

        result *= 10;
        result += input[i] - '0';
        ++i;
    }
    out_val = sign * result;
    return true;
}

// returns true if the input has between 1 to 4 hex digits
bool tryParseHexDigits(const std::string& input, int& i, std::string& out_hexDigitString) {
    out_hexDigitString = "";
    for(; i < input.size(); i++) {
        if(!isHex(input[i])) {
            return false;
        } else {
            out_hexDigitString.push_back(input[i]);
        }
    }
    return true;
}

int hex2int(const char& c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

// converts hex digits to integer
int hex2int(const std::string& input) {
    int result = 0;
    for(int i = 0; i < input.size(); i++) {
        result *= 16;
        result += hex2int(input[i]);        
    }
    return result;
}

// returns true if there was a hex integer in input at position i
// in the format of "0xFFFF" with between 1 to 4 hex digits
// filling out_val with the resultant input
// and updates i to the char after a successful parse
bool tryParseHex(const std::string& token, int& out_val) {
    int i = 0;
    std::string hexDigitString;

    if(!(tryParseNextString(token, i, "0x") || tryParseNextString(token, i, "0X")) || !tryParseHexDigits(token, i, hexDigitString)) {
        return false;
    }
    out_val = hex2int(hexDigitString);
    return true;
}

// returns true if the token is a 2 byte hex value,
// filling it's integer value in out_val
bool tryParse2ByteHex(const std::string& token, int& out_val) {
    int i = 0;
    std::string hexDigitString;

    if(!(tryParseNextString(token, i, "0x") || tryParseNextString(token, i, "0X")) || !tryParseHexDigits(token, i, hexDigitString) || hexDigitString.size() < 3 || hexDigitString.size() > 4) {
        return false;
    }
    out_val = hex2int(hexDigitString);
    return true;
}

// returns true if the token is a 1 byte hex value,
// filling it's integer value in out_val
bool tryParse1ByteHex(const std::string& token, int& out_val) {
    int i = 0;
    std::string hexDigitString;

    if(!(tryParseNextString(token, i, "0x") || tryParseNextString(token, i, "0X")) || !tryParseHexDigits(token, i, hexDigitString) || hexDigitString.size() < 1 || hexDigitString.size() > 2) {
        return false;
    }
    out_val = hex2int(hexDigitString);
    return true;
}

// returns true if the input string is a hex or decimal integer
bool tryParseInt(const std::string& input, int& out_val) {
    return tryParseHex(input, out_val) || tryParseDecimal(input, out_val);
}

// returns true if an integer value was parsed in the input at position i
// updating i to the next char after the successful parse
// and filling the value in out_val
bool tryParseNextInt(const std::string& input, int& i, int& out_val) {
    int resetI = i;
    std::string token;
    if(!tryFetchNextToken(input, i, token) || !tryParseInt(token, out_val)) {
        std::cerr << "Error: expected int after pushimm, got '" << input.substr(i) << "'" << std::endl;
        i = resetI;
        return false;
    } else {
        return true;
    }
}

bool tryParseNextInt(const std::string& input, int& i, int& out_int, std::string& out_string) {
    int resetI = i;
    if(!tryFetchNextToken(input, i, out_string) || !tryParseInt(out_string, out_int)) {
        i = resetI;
        return false;
    } else {
        return true;
    }
}

// tries to fetch an address label in the format of '#mylabel'
bool tryParseLabel(const std::string& input, int& i, std::string& out_label) {
    int resetI = i;
    if(!tryParseNextChar(input, i, '#') || !tryFetchNextToken(input, i, out_label)) {
        i = resetI;
        return false;
    } else {
        return true;
    }
}

// tries to fetch an address label in the format of '#mylabel'
bool tryParseAddress(const std::string& input, int& i, std::string& out_label) {
    int resetI = i;
    if(!tryParseNextChar(input, i, '@') || !tryFetchNextToken(input, i, out_label)) {
        i = resetI;
        return false;
    } else {
        // an address may be offset (e.g. '+2' or '-2')
        if(tryParseNextChar(input, i, '-') || tryParseNextChar(input, i, '+')) {
            // offsets may be decimal, 1 or 2 byte hex value
            std::cerr << "offsets to be implemented" << std::endl;
            return false;
        }
        return true;
    }
}

// return true if the next symbol is a multi-line comment
// updating i to the start of the comment
bool tryParseMultiCommentStart(const std::string& input, int& i) {
    int resetI = i;
    if(tryParseNextString(input, i, "/*")) {
        return true;
    }
    i = resetI;
    return false;
}

// try looking for the end of a multiline comment
// returning true if found
// updating i to the end of the comment if found
// and returning the contents of the comment to out_comment
// if not found, update i to the end of input and put the rest of input into out_comment
bool tryParseMultiCommentEnd(const std::string& input, int& i, std::string &out_comment) {
    int resetI = i;
    skipSpace(input, i);
    out_comment = "";
    while(i < input.size()) {
        if(tryParseNextString(input, i, "*/")) {
            return true;
        }
        out_comment.push_back(input[i]);
        ++i;
    }
    return false;
}

// returns true if there is a single-line comment in the input at position i
// updating i to the end of the comment
// filling out_comment with the comment
bool tryParseSingleComment(const std::string& input, int& i, std::string& out_comment) {
    int resetI = i;
    if(!tryParseNextString(input, i, "//") && !tryParseNextString(input, i, ";")) {
        i = resetI;
        return false;
    }
    skipSpace(input, i);
    out_comment = "";
    while(i < input.size()) {
        out_comment.push_back(input[i]);
        ++i;
    }
    return true;
}

// returns a string with space padding of size n
std::string getPadding(int n) {
    std::string result;
    while(n > 0) {
        result.push_back(' ');
        --n;
    }
    return result;
}

int main(int argc, char** argv) {
    std::string inFileName = "../samples/sigma5.s";
    std::string outFileName = "sigma5.mac";
    std::string macLine;
    // if(!tryParseIOFileNames(argc, argv, inFileName, outFileName)) {
    //     std::cerr << "Usage: " << argv[0] << " -i infile -o outfile" << std::endl;
    //     return 1;
    // }

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

    // assembly code line number
    int linum = 1;
    // machine code line number
    int macLinum = 0;
    std::string input;
    // if true, we expect a 1 byte value
    bool exp1Byte = false;
    // if true, we expect a 2 byte value
    bool exp2Byte = false;
    // the operation which expected a value
    std::string expOp = "";
    // if true, we are inside of a multi-line comment
    bool inMultiComment = false;
    // the line number of the multi-line comment
    int startMultiLineNum = -1;
    std::string startMultiLineString = "";

    while(std::getline(inFile, input)) {
        std::string errPrepend = "Error on line [" + std::to_string(linum) + "] ";
        int i = 0;
        std::string stringVal;

        /*
            for each line of input:
                - if we are in a multi-line comment, try to look for the end of it
                and parse the rest of the line if found
                    - and add each line of the multi-line comment to the comment queue
                while we aren't at the end of input:
                    - if we see a single-line comment, add the rest of the line to the comment queue
                      and continue to the next line
                    - skip whitespace, and continue to the next line if nothing else found
                    - try to fetch a label, error if label already set
                    - else try to fetch an address marker, filling the next 2 bytes if found
                    - else try to fetch an int
                        - if expected, set 1 or 2 byte value
                        - else, set 1 byte, or 2 bytes if too large
                    - else try to fetch a 2 byte hex value, and set it if expected
                    - else try to fetch a 1 byte hex value, and set it if expected
                    - else try to fetch an operation, setting 1 or 2 byte expected flags if needed
                    - else error unexpected values
            - [ ] todo: save mac line numbers, and fill @label addresses
            at the end of a line, 

            if the comment queue isn't empty:
                if flagAddNoops, add noop and add comment to end of line
                if flagClean, wait until next machine code value
                else put comment on its own line without machine code
        */

        // if there is nothing on the line, mark this as an empty line
        skipSpace(input, i);
        if(i >= input.size()) {
            std::cout << std::endl;
            continue;
        }

        while(i <= input.size()) {
            skipSpace(input, i);
            if(i >= input.size()) {
                break;
            }

            std::string comment = "", stringVal = "";
            int n;

            /*
                comment parsing
                - note that empty lines in the middle of a multiline comment
                  will be preserved
            */
            if(inMultiComment) {
                if(tryParseMultiCommentEnd(input, i, comment)) {
                    inMultiComment = false;
                    if(comment.size() != 0) {
                        std::cout << "; " << comment << std::endl;
                    }
                    skipSpace(input, i);
                    if(i >= input.size()) {
                        break;
                    } else {
                        continue;
                    }
                } else {
                    std::cout << "; " << comment << std::endl;
                    break;
                }
            } else if(tryParseSingleComment(input, i, comment)) {
                std::cout << "; " << comment << std::endl;
                break;
            } else if(tryParseMultiCommentStart(input, i)) {
                if(tryParseMultiCommentEnd(input, i, comment)) {
                    if(comment.size() > 0) {
                        std::cout << "; " << comment << std::endl;
                    }
                    skipSpace(input, i);
                    if(i >= input.size()) {
                        break;
                    } else {
                        continue;
                    }
                } else {
                    if(comment.size() > 0) {
                        std::cout << "; " << comment << std::endl;
                    }
                    startMultiLineNum = linum;
                    startMultiLineString = input;
                    inMultiComment = true;
                    break;
                }
            }

            std::string token;
            if(tryParseLabel(input, i, stringVal)) {
                std::cout << "#" << stringVal << std::endl;
                continue;
            } else if(tryParseAddress(input, i, stringVal)) {
                std::cout << "@" << stringVal << std::endl;
                continue;
            } else if(!tryFetchNextToken(input, i, token)) {
                std::cerr << "Error, could not parse line [" + std::to_string(linum) + "]: '" << input << "'" << std::endl;
                return 1;
            } else if(exp2Byte) {
                // TODO
                /*
                    if parsed an integer, try to pad it into a 2 byte value and accept
                        - if it's too large, error with expOp
                        exp2Byte = false;
                    if parsed a 2 byte hex value accept
                        exp2Byte = false;
                    if parsed a 1 byte hex value (e.g. 0xFF), expect a 1 byte value
                        exp2Byte = false;
                        exp1Byte = true;
                    else error with expOp
                    empty expOp
                */
                std::cout << "Error on line [" << linum << "]: expected 2 byte value for operation: '" << expOp << "'" << std::endl;
                return 1;
            } else if(exp1Byte) {
                // TODO
                std::cout << "Error on line [" << linum << "]: expected 1 byte value for operation: '" << expOp << "'" << std::endl;
                return 1;
            // } else if(tryParse2Bytes())
                /*
                    an 
                */
            } else if(tryParseDecimal(token, n)) {
                // a 2's complement decimal value shall be placed
                // as-is in the machine code as 1 or 2 bytes,
                // depending on magnitude
                if(-0x80 <= n && n <= 0x7F) {
                    std::cout << num2macString(n) << "  " << token << ' ' << std::endl;
                } else if(-0x8000 <=n && n <= 0x7FFF){
                    char h = getHighByte(n);
                    char l = getLowByte(n);
                    std::string p = getPadding(token.size());
                    std::cout << num2macString(h) << "  " << token << " H" << std::endl;
                    std::cout << num2macString(l) << "  " << p << " L" << std::endl;
                } else {
                    std::cerr << "number too large to convert: " << token << std::endl;
                }
                continue;
            /*
                a hexadecimal value shall be accepted a 1 or 2 bytes, depending on padding
                e.g. 0xFF and 0x00FF are assumed to be 1 byte and 2 bytes respectively
                note: these numbers are assumed to be unsigned
            */
            } else if(tryParse2ByteHex(token, n)) {
                char h = getHighByte(n);
                char l = getLowByte(n);
                std::string p = getPadding(token.size());
                std::cout << num2macString(h) << "  " << token << " H" << std::endl;
                std::cout << num2macString(l) << "  " << p << " L" << std::endl;
                continue;
            } else if(tryParse1ByteHex(token, n)) {
                std::cout << num2macString(n) << "  " << token << ' ' << std::endl;
                continue;
            } else if(token == "pushimm") {
                expOp = "pushimm";
                exp1Byte = true;
                std::cout << num2macString(op_pushimm) << " pushimm" << std::endl;
                continue;
            } else if(token == "pushext") {
                expOp = "pushext";
                exp2Byte = true;
                std::cout << num2macString(op_pushext) << " pushext" << std::endl;
                continue;
            } else {
                std::cout << "Error on line [" << linum << "]: unrecognized token: '" << token << "'" << std::endl;
                return 1;
            }
                // } else if(tryParseDecimal(input, i, intVal)) {
                //     if()

            // return tryParseHex(input, out_val) || tryParseDecimal(input, out_val);

            std::cerr << inMultiComment << std::endl;
            std::cerr << "Error on line [" + std::to_string(linum) + "]: debugging " << std::endl;
            return 1;
        }

        ++linum;
        continue;

        // if this line has only whitespace or comments, fill it with noop
        // if(i >= input.size() || hasComment(input, i)) {
        //     macLine = num2macString(op_noop) + " " + input.substr(i);
        //     return true;
        // }

        // std::string token;
        // if(!tryFetchNextToken(input, i, token)) {
        //     return false;
        // }

        // if(token == "pushimm") {
        //     if(!tryParseNextInt(input, i, intVal, stringVal)) {
        //         std::cerr << errPrepend << "expected int value, got '" << input.substr(i) << "'" << std::endl;
        //         return false;
        //     }
        //     if(intVal > 0xFF) {
        //         std::cerr << errPrepend << "expected 1 byte value, got '" << stringVal << "'" << std::endl;
        //         return false;
        //     }

        //     macLine =
        //         num2macString(op_pushimm)
        //         + " pushimm"
        //         + input.substr(i)
        //         + "\n"
        //         + num2macString(intVal)
        //         + " " + stringVal;
        //     return true;
        // } else if(token == "add") {
        //     macLine = 
        //         num2macString(op_add)
        //         + " add"
        //         + input.substr(i);
        //     return true;
        // } else if(token == "halt") {
        //     macLine = 
        //         num2macString(op_halt)
        //         + " halt"
        //         + input.substr(i);
        //     return true;
        // } else {
        //     std::cerr << errPrepend << "token not recognized: '" << token << "'" << std::endl;
        //     return 1;
        // }

        std::cout << macLine << std::endl;
        outFile << macLine << std::endl;
    }

    if(inMultiComment) {
        std::cerr << "Error on line [" + std::to_string(startMultiLineNum) + "]: unclosed multi-line comment '" << startMultiLineString << "'" << std::endl;
        return 1;
    }

    return 0;
}