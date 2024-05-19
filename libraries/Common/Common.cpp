#include "Common.h"

String combineStrings(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // Calculate the length of the resulting string
    int length = vsnprintf(nullptr, 0, format, args) + 1; // +1 for null terminator
    
    char* combinedString = new char[length];
    
    vsprintf(combinedString, format, args);

    String result = String(combinedString);
    
    delete[] combinedString;
    va_end(args);
	
	return result;
}