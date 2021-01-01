/*
 * utils.h
 *
 *  Created on: Oct 3, 2020
 *      Author: zbc
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>

void Bytes2HexString(const unsigned char* input, unsigned int length, std::string& output);
char *phex(const void *p, unsigned long n);


#endif /* UTILS_H_ */
