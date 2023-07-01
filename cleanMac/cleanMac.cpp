#include <cstdio>
/*
    removes anything except for binary text on a line
*/

enum states {
    line_begin,
    in_binary,
    no_binary,
    after_binary,
};

int main() {
    int state = line_begin;

    char c;
    while((c = getchar()) != EOF) {
        switch(state) {
            case line_begin:
                switch(c) {
                    case '1':
                    case '0':
                        printf("%c", c);
                        state = in_binary;
                        break;
                    case '\n':
                    case '\r':
                    case '\t':
                    case ' ':
                        break;
                    default:
                        state = no_binary;
                }
                break;
            case in_binary:
                switch(c) {
                    case '1':
                    case '0':
                        printf("%c", c);
                        break;
                    case '\n':
                        printf("\n");
                        state = line_begin;
                        break;
                    default:
                        printf("\n");
                        state = after_binary;
                        break;
                }
                break;
            case no_binary:
                 switch(c) {
                    case '\n':
                        state = line_begin;
                        break;
                    default:
                        break;
                }
            case after_binary:
                switch(c) {
                    case '\n':
                        state = line_begin;
                        break;
                    default:
                        break;
                }
                break;
            default:
                return 1;
        }
    }
}