/*
 * Regexp - regular expression searching
 */

#ifndef regexp_h
#define regexp_h

class Regexp {
public:
    Regexp(const char*);
    Regexp(const char*, int length);
    ~Regexp();

    int Search(const char* text, int length, int index, int range);
    int Match(const char* text, int length, int index);
    int BeginningOfMatch(int subexp = 0);
    int EndOfMatch(int subexp = 0);
protected:
    char* pattern;
    int beginning;
    int end;
};

#endif
