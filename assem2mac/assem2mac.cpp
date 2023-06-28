/*
    accepts an ssbc assembly file and transforms it into machine code
*/

#include <string>
#include <sstream>
#include <queue>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include "../common.h"

// if true, print mac line numbers in hex
bool HEX_LINE_NUMBER = false;
// if true, add a noops to comments and empty lines
bool ADD_NOOPS = false;

/*
    - every line of assembly code can have 0 or 1 instruction.
    - whitespace is ignored
    - comments are prepended by a semicolon (;) or a double-slash (//)
*/

// opcodes
enum {
    op_noop = 0,      // no operation     - 00000000
    op_halt = 1,      // halt             - 00000001
    op_pushimm = 2,   // push immediate   - 00000010
    op_pushext = 3,   // push external    - 00000011
    op_popinh = 4,    // pop inherent     - 00000100
    op_popext = 5,    // pop external     - 00000101
    op_jnz = 6,       // jump not zero    - 00000110
    op_jnn = 7,       // jump not negative- 00000111
    op_add = 8,       // add              - 00001000
    op_sub = 9,       // subtract         - 00001001
    op_nor = 10,      // nor              - 00001010
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
    return std::isalnum(c) || c == '_';
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

// returns true if we have a hex symbol
bool isHex(const char& c) {
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}

// returns true if there was a decimal integer in input at position i
// filling out_val with the resultant input
// and updates i to the char after a successful parse
bool tryParseDecimal(const std::string& input, int& i, std::string& out_string, int& out_val) {
    int resetI = i;
    out_string = "";
    int sign = 1;

    if(i < input.size()) {
        if(input[i] == '-') {
            out_string.push_back(input[i]);
            sign = -1;
            ++i;
        } else if(input[i] == '+') {
            out_string.push_back(input[i]);
            ++i;
        }
    }

    if(i >= input.size() || !std::isdigit(input[i])) {
        i = resetI;
        return false;
    }

    int result = 0;
    while(i < input.size() && std::isdigit(input[i])) {
        result *= 10;
        result += input[i] - '0';
        out_string.push_back(input[i]);
        ++i;
    }
    out_val = sign * result;
    return true;
}

// returns true if the input has between 1 to 4 hex digits
bool tryParseHexDigits(const std::string& input, int& i, std::string& out_hexDigitString) {
    int resetI = i;
    out_hexDigitString = "";
    while(i < input.size() && isHex(input[i])) {
        out_hexDigitString.push_back(input[i]);
        ++i;
    }
    if(out_hexDigitString.size() == 0){
        i = resetI;
        return false;
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

// returns a hex char for 0 <= n <= 15
char intToHexChar(int n) {
    if(0 <= n && n <= 9) {
        return n + '0';
    } else if (10 <= n && n <= 15) {
        return n - 10 + 'A';
    } else {
        return 'X';
    }
}

// accepts a number and returns a 4-byte hex string
std::string intToFourHex(int n) {
    if(n > 0xFFFF) {
        return "xxxx";
    }
    std::string result = "0x";
    for(int i = 0; i < 4; i++) {
        result.insert(result.begin()+2, intToHexChar(n % 16));
        n /= 16;                
    }
    return result;
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
bool tryParseHex(const std::string& input, int& i, std::string& out_string, int& out_int) {
    int resetI = i;
    std::string hexDigitString;

    if(!(tryParseNextString(input, i, "0x") || tryParseNextString(input, i, "0X")) || !tryParseHexDigits(input, i, hexDigitString)) {
        i = resetI;
        return false;
    }

    out_string = "";
    out_string.append("0x");
    out_string.append(hexDigitString);
    out_int = hex2int(hexDigitString);
    return true;
}

// returns true if the token is a 2 byte hex value,
// filling it's integer value in out_val
bool tryParse2ByteHex(const std::string& input, int& i, std::string& out_string, int& out_int) {
    int resetI = i;
    std::string hexDigitString;
    if(!(tryParseNextString(input, i, "0x") || tryParseNextString(input, i, "0X")) || !tryParseHexDigits(input, i, hexDigitString) || hexDigitString.size() < 3 || hexDigitString.size() > 4) {
        i = resetI;
        return false;
    }
    out_string = "";
    out_string.append("0x");
    out_string.append(hexDigitString);
    out_int = hex2int(hexDigitString);
    return true;
}

// returns true if the token is a 1 byte hex value,
// filling it's integer value in out_val
bool tryParse1ByteHex(const std::string& input, int& i, std::string& out_string, int& out_int) {
    int resetI = i;
    std::string hexDigitString;

    if(!(tryParseNextString(input, i, "0x") || tryParseNextString(input, i, "0X")) || !tryParseHexDigits(input, i, hexDigitString) || hexDigitString.size() < 1 || hexDigitString.size() > 2) {
        i = resetI;
        return false;
    }
    out_string = "";
    out_string.append("0x");
    out_string.append(hexDigitString);
    out_int = hex2int(hexDigitString);
    return true;
}

// returns true if the input string is a hex or decimal integer
bool tryParseInt(const std::string& input, int& i, std::string& out_string, int& out_int) {
    return tryParseHex(input, i, out_string, out_int) || tryParseDecimal(input, i, out_string, out_int);
}

// returns true if an integer value was parsed in the input at position i
// updating i to the next char after the successful parse
// and filling the value in out_val
bool tryParseNextInt(const std::string& input, int& i, std::string& out_string, int& out_int) {
    int resetI = i;
    skipSpace(input, i);
    if(!tryParseInt(input, i, out_string, out_int)) {
        i = resetI;
        return false;
    } else {
        return true;
    }
}

// returns true if an integer value was parsed in the input at position i
// updating i to the next char after the successful parse
// and filling the value in out_val
bool tryParseNextInt(const std::string& input, int& i, int& out_int) {
    std::string _;
    int resetI = i;
    skipSpace(input, i);
    if(!tryParseInt(input, i, _, out_int)) {
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

/* specify the high part or low part of a byte */
enum BytePart {
    byte_high,
    byte_low
};

class AddressPart {
    public:
    AddressPart() { }
    AddressPart(std::string _label, BytePart _bytePart) : label(_label), bytePart(_bytePart) {
        offset = 0;
        macLine = -1;
    }

    AddressPart(std::string _label, BytePart _bytePart, int offset) : label(_label), bytePart(_bytePart){ }
    std::string label;
    int offset = 0;
    int macLine = -1;
    BytePart bytePart;

    // set the machine code line
    // returns true if the resulting macLine is possible 
    // (i.e. it isn't negative and fits in two bytes (<=0xFFFF))
    bool set(int _macLine) {
        macLine = _macLine + offset;
        return macLine < 0 || macLine > 0xFFFF;
    }

    // returns the machine code line after offset
    int get() {
        return macLine;
    }

    // returns the mac-string which corresponds to this address part
    std::string getMacString() {
        if(this->bytePart == byte_high) {
            char h = getHighByte(this->macLine);
            return num2macString(h);
        } else if(this->bytePart == byte_low) {
            char l = getLowByte(this->macLine);
            return num2macString(l);
        } else {
            return "xxxxxxxx";
        }
    }

    // return a new address part object pointer from a string
    // returns null if not possible
    static std::shared_ptr<AddressPart> tryParse(const std::string& input, int& i) {
        int resetI = i;
        auto addressPart = std::make_shared<AddressPart>();
        if(!tryParseNextChar(input, i, '@') || !tryFetchNextToken(input, i, addressPart->label)) {
            i = resetI;
            return nullptr;
        }

        // parse the offset if possible
        if(tryParseNextChar(input, i, '-')) {
            if(!tryParseNextInt(input, i, addressPart->offset)) {
                i = resetI;
                return nullptr;
            }
            addressPart->offset *= -1;
        } else if(tryParseNextChar(input, i, '+')) {
            if(!tryParseNextInt(input, i, addressPart->offset)) {
                i = resetI;
                return nullptr;
            }
        } else {
            addressPart->offset = 0;
        }

        if(!tryParseNextChar(input, i, '.')) {
            i = resetI;
            return nullptr;
        }

        if(tryParseNextChar(input, i, 'h') || tryParseNextChar(input, i, 'H')) {
            addressPart->bytePart = byte_high;
        } else if(tryParseNextChar(input, i, 'l') || tryParseNextChar(input, i, 'L')) {
            addressPart->bytePart = byte_low;
        } else {
            i = resetI;
            return nullptr;
        }

        return addressPart;
    }

    // returns the address label string
    std::string toString() {
        std::string result;
        // TODO
        result.push_back('@');
        result.append(label);
        if(offset > 0) {
            result.push_back('+');
            result.append(std::to_string(offset));
        } else if(offset < 0) {
            result.append(std::to_string(offset));
        }
        result.push_back('.');
        if(bytePart == byte_high) {
            result.push_back('H');
        } else if(bytePart == byte_low) {
            result.push_back('L');
        }
        return result;
    }
};

// a full address, composed of a high byte part and a low byte part
class Address {
    public:
    Address() {}
    Address(std::string _label, BytePart _bytePart, int offset) : label(_label) { }

    std::shared_ptr<AddressPart> highPart;
    std::shared_ptr<AddressPart> lowPart;
    std::string label;
    int offset;

    // sets the machine line number of this address
    void set(int _macLineNum) {
        highPart->set(_macLineNum);
        lowPart->set(_macLineNum);
    }

    // returns a pointer to an address if successfully parsed,
    // updating i to the char after a successful parse
    // else return nullptr
    static std::shared_ptr<Address> tryParse(const std::string& input, int& i) {
        int resetI = i;
        auto address = std::make_shared<Address>();
        if(!tryParseNextChar(input, i, '@') || !tryFetchNextToken(input, i, address->label)) {
            i = resetI;
            return nullptr;
        }

        // parse the offset if possible
        if(tryParseNextChar(input, i, '-')) {
            if(!tryParseNextInt(input, i, address->offset)) {
                i = resetI;
                return nullptr;
            }
            address->offset *= -1;
        } else if(tryParseNextChar(input, i, '+')) {
            if(!tryParseNextInt(input, i, address->offset)) {
                i = resetI;
                return nullptr;
            }
        } else {
            address->offset = 0;
        }

        address->highPart = std::make_shared<AddressPart>(address->label, byte_high, address->offset);
        address->lowPart = std::make_shared<AddressPart>(address->label, byte_low, address->offset);
        return address;
    }

    // returns this address as a string
    std::string toString() {
        std::string result;
        // TODO
        result.push_back('@');
        result.append(label);
        if(offset > 0) {
            result.push_back('+');
            result.append(std::to_string(offset));
        } else if(offset < 0) {
            result.append(std::to_string(offset));
        }
        return result;
    }
};

/*
    a mac line is a line of resultant machine code
    with a machine code string (left empty for comments),
    a line number
    an assembly code line
    an optional address label,
    an optional comment
    an address-part specifier pointer which,
        if filled, is used to dereference an address after partial assembly
*/
class MacLine {
    public:
    MacLine(int _assemLineNum) : assemLineNum(_assemLineNum) { }
    MacLine(int _assemLineNum, std::string _macString, std::string _assemString) : assemLineNum(_assemLineNum), macString(_macString), assemString(_assemString) { }
    MacLine(int _assemLineNum, std::string _macString, std::string _assemString, std::shared_ptr<AddressPart> _addressRef)
        : assemLineNum(_assemLineNum), macString(_macString), assemString(_assemString), addressRef(_addressRef) { }

    // the line number which corresponds to the assembly code
    int assemLineNum;
    // the machine code line number (this is unique)
    int macLineNum;
    std::string macString = "";
    std::string assemString = "";
    std::string addressLabel = "";
    std::string comment = "";
    std::shared_ptr<AddressPart> addressRef = nullptr;
    friend std::ostream& operator<<(std::ostream& os, MacLine& macLine) {
        if(macLine.macString != "" && HEX_LINE_NUMBER) {
            os << intToFourHex(macLine.macLineNum) << " "; // TODO remove
        }
        os << macLine.macString;
        if(macLine.assemString != "") {
            os << " " << macLine.assemString;
        }
        if(macLine.addressLabel != "") {
            os << " #" << macLine.addressLabel;
        }
        if(macLine.comment != "") {
            os << " ; " << macLine.comment;
        }
        return os;
    }
};

// a queue of machine code lines
class MacQueue {
    private:
    std::queue<std::shared_ptr<MacLine>> codeQueue;

    public:
    std::shared_ptr<MacLine> push(int assemLineNum) {
        auto macLine = std::make_shared<MacLine>(assemLineNum);
        return this->push(macLine);
    }
    std::shared_ptr<MacLine> push(int assemLineNum, std::string macString, std::string assemString) {
        auto macLine = std::make_shared<MacLine>(assemLineNum, macString, assemString);
        return this->push(macLine);
    }

    std::shared_ptr<MacLine> push(int assemLineNum, std::string macString, std::string assemString, std::shared_ptr<AddressPart> addressPart) {
        auto macLine = std::make_shared<MacLine>(assemLineNum, macString, assemString, addressPart);
        return this->push(macLine);
    }
    
    std::shared_ptr<MacLine> push(std::shared_ptr<MacLine> macLine) {
        codeQueue.push(macLine);
        return macLine;
    }

    bool empty() {
        return codeQueue.empty();
    }

    void pop() {
        codeQueue.pop();
    }

    std::shared_ptr<MacLine> front() {
        return codeQueue.front();
    }
};

// a map which resolves a given address to its integer machine code line
class AddressMap {
    private:
    std::map<std::string, int> m;

    public:
    bool has(std::string s) {
        return m.find(s) != m.end();
    }
    void set(std::string s, int n) {
        m[s] = n;
    }
    int get(std::string s) {
        return m[s];
    }
};

int main(int argc, char** argv) {
    std::string inFileName;
    std::string outFileName;
    std::string macLine;
    if(!tryParseIOFileNames(argc, argv, inFileName, outFileName)) {
        std::cerr << "Usage: " << argv[0] << " -i infile -o outfile" << std::endl;
        return 1;
    }

    if(tryParseArg(argc, argv, "--add-noops")) {
        ADD_NOOPS = true;        
    }

    if(tryParseArg(argc, argv, "--hex-line-number")) {
        HEX_LINE_NUMBER = true;        
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

    // assembly code line number
    int assemLineNum = 0;
    // machine code line number
    int macLineNum = 0;
    std::string input;
    // if true, we expect a 1 byte value
    bool exp1Byte = false;
    // if true, we expect a 2 byte value
    bool exp2Byte = false;
    // the operation which expected a value
    std::string expOp = "";
    // a queue of machine code lines to be added
    MacQueue macQueue;
    // a queue of comments
    std::queue<std::string> commentQueue;
    // a label for the next machine code byte
    std::string addressLabel = "";
    // a list of machine code line
    std::vector<std::shared_ptr<MacLine>> macLines;
    // a list of machine code lines which need a resolved address
    std::queue<std::shared_ptr<MacLine>> unresolvedMacLineQueue;
    // a map which resolves a given address to its integer machine code line
    AddressMap addressMap;

    while(std::getline(inFile, input)) {
        ++assemLineNum;

        int i = 0;
        std::string stringVal;
        std::shared_ptr<Address> address;
        std::shared_ptr<AddressPart> addressPart;

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
            commentQueue.push("");
            continue;
        }

        while(i <= input.size()) {

            skipSpace(input, i);
            if(i >= input.size()) {
                break;
            }

            std::string comment = "", stringVal = "";
            int intVal;
            char charVal;

            std::string token;
            if(tryParseSingleComment(input, i, comment)) {
                commentQueue.push(comment);
                continue;
            } else if(tryParseLabel(input, i, stringVal)) {
                addressLabel = stringVal;
                continue;
            } else if((addressPart = AddressPart::tryParse(input, i)) != nullptr) {
                std::string label = addressPart->label;

                auto macLine = macQueue.push(assemLineNum, "XXXXXXXX", addressPart->toString(), addressPart);
                unresolvedMacLineQueue.push(macLine);
                exp1Byte = false;
                continue;
            } else if((address = Address::tryParse(input, i)) != nullptr) {
                std::string label = stringVal;
                if(exp1Byte) {
                    std::cerr << "Error on line [" << assemLineNum << "]: expected 1 byte value for operation: '" << expOp << "', received full address reference '@" << label << "'" << std::endl;
                    std::cerr << "use '@" << label << ".H' or '@" << label << ".L' instead to use the high-byte or low-byte of an address respectively" << std::endl;
                    return 1;
                } else {
                    exp2Byte = false;
                }

                // high byte
                {
                    auto macLine = macQueue.push(assemLineNum, "XXXXXXXX", address->toString() + ".H", address->highPart);
                    unresolvedMacLineQueue.push(macLine);
                }

                // low byte
                {
                    std::string p = getPadding(address->toString().size());
                    auto macLine = macQueue.push(assemLineNum, "XXXXXXXX", p + ".L", address->lowPart);
                    unresolvedMacLineQueue.push(macLine);
                }
                continue;
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
            } else if(exp2Byte) {
                /*
                    when expecting 2 bytes, we shall accept either:
                    - a decimal value, which will be cast as a 2 byte value
                    - a 4-digit hex value
                    - a 2-digit hex value, which will be assumed to be a 1 byte value
                        - if we only received 1 byte hex value, expect another 1 byte value on the next line
                        - note: must add padding to be accepted as a 2 byte value:
                            0x00FF vs 0xFF
                */
                if(tryParse2ByteHex(input, i, stringVal, intVal)) {
                    int& n = intVal;
                    std::string& numString = stringVal;

                    char h = getHighByte(n);
                    char l = getLowByte(n);
                    std::string p = getPadding(numString.size());
                    macQueue.push(assemLineNum, num2macString(h), numString + " H");
                    macQueue.push(assemLineNum, num2macString(l), p + " L");
                    exp1Byte = false;
                    exp2Byte = false;
                    continue;
                } else if(tryParse1ByteHex(input, i, stringVal, intVal)) {
                    int& n = intVal;
                    std::string& numString = stringVal;

                    macQueue.push(assemLineNum, num2macString(n), numString);
                    exp2Byte = false;
                    exp1Byte = true;
                    continue;
                } else if(tryParseDecimal(input, i, stringVal, intVal)) {
                    int& n = intVal;
                    std::string& numString = stringVal;
                    if(-0x80 <= n && n <= 0x7F) {
                        macQueue.push(assemLineNum, "00000000", numString + " H");
                        macQueue.push(assemLineNum, num2macString(n), getPadding(numString.size()) + " L");
                    } else if(-0x8000 <=n && n <= 0x7FFF){
                        char h = getHighByte(n);
                        char l = getLowByte(n);
                        std::string p = getPadding(numString.size());
                        macQueue.push(assemLineNum, num2macString(h), numString + " H");
                        macQueue.push(assemLineNum, num2macString(l), numString + " L");
                    } else {
                        std::cerr << "Error on line [" << assemLineNum << "]: number too large to convert: " << numString << std::endl;
                        return 1;
                    }
                    exp1Byte = false;
                    exp2Byte = false;
                    continue;
                /*
                    an unsigned hexadecimal value shall be accepted a 1 or 2 bytes, depending on padding
                    e.g. 0xFF and 0x00FF are assumed to be 1 byte and 2 bytes respectively
                */
                } else {
                    std::cerr << "Error on line [" << assemLineNum << "]: expected 2 byte value for operation: '" << expOp << std::endl;
                    std::cerr << "'" << input << "'" << std::endl;
                    return 1;
                }
            } else if(exp1Byte) {
                /*
                    expect a decimal or hex value, print an error if it is too large or incorrect
                    no parsing of 2 byte values here since it would be inapprpriate,
                */
                if(tryParse1ByteHex(input, i, stringVal, intVal)) {
                    int& n = intVal;
                    std::string& numString = stringVal;

                    macQueue.push(assemLineNum, num2macString(n), numString);
                    exp1Byte = false;
                    exp2Byte = false;
                    continue;
                } else if(tryParseDecimal(input, i, stringVal, intVal) && (-0x80 <= intVal && intVal <= 0x7F)) {
                    int& n = intVal;
                    std::string& numString = stringVal;

                    macQueue.push(assemLineNum, num2macString(n), numString);
                    exp1Byte = false;
                    exp2Byte = false;
                    continue;
                } else {
                    std::cerr << "Error on line [" << assemLineNum << "]: expected 1 byte value for operation: '" << expOp << std::endl;
                    std::cerr << "'" << input << "'" << std::endl;
                    return 1;
                }
                return 1;
            } else if(tryParse2ByteHex(input, i, stringVal, intVal)) {
                int& n = intVal;
                std::string& numString = stringVal;

                char h = getHighByte(n);
                char l = getLowByte(n);
                std::string p = getPadding(stringVal.size());
                    macQueue.push(assemLineNum, num2macString(h), numString + " H");
                    macQueue.push(assemLineNum, num2macString(l), p + " L");
                continue;
            } else if(tryParse1ByteHex(input, i, stringVal, intVal)) {
                int& n = intVal;
                std::string& numString = stringVal;

                macQueue.push(assemLineNum, num2macString(n), numString);
                continue;
            } else if(tryParseDecimal(input, i, stringVal, intVal)) {
                int& n = intVal;
                std::string& numString = stringVal;

                // a 2's complement decimal value shall be placed
                // as-is in the machine code as 1 or 2 bytes,
                // depending on magnitude
                if(-0x80 <= n && n <= 0x7F) {
                    macQueue.push(assemLineNum, num2macString(n), numString);
                } else if(-0x8000 <=n && n <= 0x7FFF){
                    char h = getHighByte(n);
                    char l = getLowByte(n);
                    std::string p = getPadding(numString.size());
                    macQueue.push(assemLineNum, num2macString(h), numString + " H");
                    macQueue.push(assemLineNum, num2macString(l), p + " L");
                } else {
                    std::cerr << "Error on line [" << assemLineNum << "]: number too large to convert: " << numString << std::endl;
                    return 1;
                }
                continue;
            /*
                an unsigned hexadecimal value shall be accepted a 1 or 2 bytes, depending on padding
                e.g. 0xFF and 0x00FF are assumed to be 1 byte and 2 bytes respectively
            */
            } else if(!tryFetchNextToken(input, i, token)) {
                std::cerr << "Error, could not parse line [" + std::to_string(assemLineNum) + "]: '" << input << "'" << std::endl;
                return 1;
            } else if(token == "pushimm") {
                expOp = "pushimm";
                exp1Byte = true;
                macQueue.push(assemLineNum, num2macString(op_pushimm), "pushimm");
                continue;
            } else if(token == "pushext") {
                expOp = "pushext";
                exp2Byte = true;
                exp1Byte = false;
                macQueue.push(assemLineNum, num2macString(op_pushext), "pushext");
                continue;
            } else {
                std::cerr << "Error on line [" << assemLineNum << "]: unrecognized token: '" << token << "'" << std::endl;
                return 1;
            }
        }

        // if there is a comment without machine line codes
        // just add empty machine lines with the comment
        if(macQueue.empty()) {
            while(!commentQueue.empty()) {
                std::string comment = commentQueue.front();
                commentQueue.pop();
                if(ADD_NOOPS) {
                    auto macLine = macQueue.push(assemLineNum);
                    macLine->comment = comment;
                    macLine->macString = num2macString(op_noop);
                } else {
                    auto macLine = std::make_shared<MacLine>(assemLineNum);
                    macLine->comment = comment;
                    macLine->macLineNum = -1;
                    macLines.push_back(macLine);
                }
            }
        }

        while(!macQueue.empty()) {
            auto macLine = macQueue.front();
            macQueue.pop();
            macLine->macLineNum = macLineNum;

            if(addressLabel != "") {
                macLine->addressLabel = addressLabel;
                if(addressMap.has(addressLabel)) {
                    std::cerr << "Error on line [" << assemLineNum << "]: address label used already: '" << addressLabel << "'" << std::endl;
                    std::cerr << "'" << input << "'" << std::endl;
                    return 1;
                } else {
                    addressMap.set(addressLabel, macLineNum);
                }
                addressLabel = "";
            }

            if(!commentQueue.empty()) {
                std::string comment = commentQueue.front();
                commentQueue.pop();
                macLine->comment = comment;
            }

            macLines.push_back(macLine);
            macLineNum++;
        }
        continue;
    }

    // resolve address references
    while(!unresolvedMacLineQueue.empty()) {
        auto macLine = unresolvedMacLineQueue.front();
        unresolvedMacLineQueue.pop();
        if(macLine->addressRef == nullptr) {
            // should never happen
            std::cerr << "Error on line [" << macLine->assemLineNum << "]: mac line in address ref queue without address" << std::endl;
            return 1;
        }

        std::string refLabel = macLine->addressRef->label;
        if(!addressMap.has(refLabel)) {
            std::cerr << "Error on line [" << macLine->assemLineNum << "]: unrecognized address label: '" << refLabel << "'" << std::endl;
            return 1;
        } else {
            // update the machine code to use the referenced address
            // (the high part or low part value)
            int macLineNum = addressMap.get(refLabel);
            macLine->addressRef->set(macLineNum);
            macLine->macString = macLine->addressRef->getMacString();
        }
    }

    for(const auto& macLine : macLines) {
        std::cout << *macLine << std::endl;
    }

    return 0;
}