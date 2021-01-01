/*
 * utils.cpp
 *
 *  Created on: Oct 3, 2020
 *      Author: zbc
 */

/*   Byte值转换为bytes字符串
*   @param src：Byte指针 srcLen:src长度 des:转换得到的bytes字符串
**/
#include <cstring>
#include <string>
#include <cstdlib>


/*
 * Return a hex string representing the data pointed to by `p`,
 * converting `n` bytes.
 *
 * The string should be deallocated using `free()` by the caller.
 */
char *phex(const void *p, unsigned long n)
{
    const unsigned char *cp =(const unsigned char *) p;              /* Access as bytes. */
    char *s = (char*)malloc(2*n + 1);       /* 2*n hex digits, plus NUL. */
    size_t k;

    /*
     * Just in case - if allocation failed.
     */
    if (s == NULL)
        return s;

    for (k = 0; k < n; ++k) {
        /*
         * Convert one byte of data into two hex-digit characters.
         */
        sprintf(s + 2*k, "%02X", cp[k]);
    }

    /*
     * Terminate the string with a NUL character.
     */
    s[2*n] = '\0';

    return s;
}


void Bytes2HexString(const unsigned char* input, unsigned int length, std::string& output)
{
	output.reserve(length << 1);
	output.clear();
	char b[3];
	for (unsigned int i = 0; i < length; i++)
	{
		sprintf(b, "%02X", input[i]);
		output.append(1, b[0]);
		output.append(1, b[1]);
	}
}

/**
 * bytes字符串转换为Byte值
* @param String src Byte字符串，每个Byte之间没有分隔符
* @return byte[]
*/
static unsigned char* hexStr2Bytes(std::string src)
{
	char *strEnd;
	int m=0;
	int len = src.length()/2;
	unsigned char* ret = new unsigned char[len];

	for(int i =0;i<len;i++)
	{
		m = i*2;
		std::string subs = src.substr(m,2);
		ret[i] = strtol(subs.c_str(),&strEnd,16);
	}
	return ret;
}


