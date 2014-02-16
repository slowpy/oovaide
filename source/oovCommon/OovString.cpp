/*
 * OovString.cpp
 *
 *  Created on: Oct 6, 2013
 *  \copyright 2013 DCBlaha.  Distributed under the GPL.
 */
#include "OovString.h"
#include <memory.h>
#include <stdio.h>

static int isAsciiLenOk(char const * const buf, int maxLen)
    {
    bool success = false;
    for(int i=0; i<maxLen; i++)
	{
	if(buf[i] == '\0')
	    {
	    success = true;
	    break;
	    }
	}
    return success;
    }

// srcBytes must include null.
static bool copyString(char * const dstStr, int maxDstBytes,
    char const * const srcStr, int srcBytes)
    {
    unsigned int len = std::min(maxDstBytes, srcBytes);
    memcpy(dstStr, srcStr, len);
    dstStr[maxDstBytes-1] = '\0';
    return(srcBytes > maxDstBytes);
    }

bool StringToFloat(char const * const mbStr, float min, float max, float &val)
    {
    bool success = false;
    float valNum = 0;
    char dummyc = 0;
    int ret = sscanf(mbStr, "%f%c", &valNum, &dummyc);
    if(ret == 1 && (valNum >= min && valNum <= max))
	{
	val = valNum;
	success = true;
	}
    return success;
    }

bool StringToInt(const char *mbStr, int min, int max, int &val)
    {
    bool success = false;
    int valNum = 0;
    char dummyc = 0;
    int ret = sscanf(mbStr, "%d%c", &valNum, &dummyc);
    if(ret == 1 && (valNum >= min && valNum <= max))
	{
	val = valNum;
	success = true;
	}
    return success;
    }

bool StringToUnsignedInt(const char *mbStr, unsigned int min, unsigned int max,
	unsigned int &val)
    {
    bool success = false;
    unsigned int valNum = 0;
    char dummyc = 0;
    int ret = sscanf(mbStr, "%u%c", &valNum, &dummyc);
    if(ret == 1 && (valNum >= min && valNum <= max))
	{
	val = valNum;
	success = true;
	}
    return success;
    }

bool IntToAsciiString(int value, char * const buffer, size_t dstSizeInBytes, int radix)
    {
    bool success = false;
    if(dstSizeInBytes > 0)
    	{
	const int numBufSize = 30;
    	char numBuf[numBufSize];
	if(radix == 10)
	    snprintf(numBuf, sizeof(numBuf), "%d", value);
	else
	    snprintf(numBuf, sizeof(numBuf), "%x", value);
	if(isAsciiLenOk(numBuf, dstSizeInBytes))
	    {
	    copyString(buffer, dstSizeInBytes, numBuf, numBufSize);
	    success = true;
	    }
	else if(dstSizeInBytes >= 2)
	    {
	    buffer[0] = '*';
	    buffer[1] = '\0';
	    }
	else
	    buffer[0] = '\0';
    	}
    return success;
    }

// This can count the number of multibyte or ASCII characters.
// Count all first-bytes (the ones that don't match 10xxxxxx).
int OovStringRefInterface::numChars() const
    {
    const char *s = c_str();
    int len = 0;
    while(*s)
	    {
	    len += ((*s++ & 0xc0) != 0x80);
	    }
    return len;
    }

bool OovStringRefInterface::isAscii() const
    {
    const char *p = c_str();
    while(*p)
	    {
	    if(*p & 0x80)	// Stop on any character that is not ASCII
		    break;
	    p++;
	    }
    return *p == '\0';	// If at end, there is no non-ASCII character
    }

size_t OovStringRefInterface::findSpace(size_t startPos) const
    {
    char const * p = c_str()+startPos;
    while(*p)
	{
	if(isspace(*p))
	    break;
	else
	    p++;
	}
    if(*p == '\0')
	return std::string::npos;
    else
	return p-c_str();
    }

//////////////

void OovString::setUpperCase(char const * const str)
{
    char const *p = str;
    while(*p)
	{
	*this += toupper(*p);
	p++;
	}
}

void OovString::setLowerCase(char const * const str)
{
    char const *p = str;
    while(*p)
	{
	*this += tolower(*p);
	p++;
	}
}

void OovString::appendInt(int val, int radix)
    {
    char buf[30];
    IntToAsciiString(val, buf, sizeof(buf), radix);
    append(buf);
    }

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = 0;
    const int delimLen = 1;
    while(end != std::string::npos)
	{
        end = str.find(delimiter, start);
        tokens.push_back(str.substr(start,
	   (end == std::string::npos) ? std::string::npos : end - start));
        start = (( end > (std::string::npos - delimLen)) ?
        	std::string::npos : end + delimLen);
	}
    return tokens;
    }
