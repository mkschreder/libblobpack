/*
Developed by ESN, an Electronic Arts Inc. studio. 
Copyright (c) 2014, Electronic Arts Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of ESN, Electronic Arts Inc. nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS INC. BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Portions of code from MODP_ASCII - Ascii transformations (upper/lower, etc)
http://code.google.com/p/stringencoders/
Copyright (c) 2007	Nick Galbreath -- nickg [at] modp [dot] com. All rights reserved.

Numeric decoder derived from from TCL library
http://www.opensource.apple.com/source/tcl/tcl-14/tcl/license.terms
 * Copyright (c) 1988-1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
*/

#include "blobpack.h"
#include <stdlib.h>
#include <string.h>
#include "ujson.h"

#define DEBUG(...) {}

void Object_objectAddKey(void *prv, JSOBJ obj, JSOBJ name, JSOBJ value){
	DEBUG("new key %p %p %p\n", obj, name, value); 
	//blob_put_string(prv, name); 
}

void Object_arrayAddItem(void *prv, JSOBJ obj, JSOBJ value){
	DEBUG("new array item \n"); 
}

JSOBJ Object_newString(void *prv, wchar_t *start, wchar_t *end){
	size_t len = end - start + 1; 
	char *str = alloca(len); 
	memset(str, 0, len); 
	wcstombs(str, start, end - start); 
	DEBUG("new string %s\n", str); 
	return blob_put_string(prv, str);  
}

JSOBJ Object_newTrue(void *prv){
	DEBUG("new true\n"); 
	return blob_put_int(prv, 1); 
}

JSOBJ Object_newFalse(void *prv){
	DEBUG("new false\n"); 
	return blob_put_int(prv, 0); 
}

JSOBJ Object_newNull(void *prv){
	DEBUG("new null\n"); 
	return blob_put_int(prv, 0); 
}

JSOBJ Object_newObject(void *prv){	
	DEBUG("new object\n"); 
	return blob_open_table(prv);
}

JSOBJ Object_newArray(void *prv){
	DEBUG("new array\n"); 
	return blob_open_array(prv); 
}

JSOBJ Object_newInteger(void *prv, JSINT32 value){
	DEBUG("new int\n"); 
	return blob_put_int(prv, value); 
}

JSOBJ Object_newLong(void *prv, JSINT64 value){
	DEBUG("new long\n"); 
	return blob_put_int(prv, value); 
}

JSOBJ Object_newUnsignedLong(void *prv, JSUINT64 value){
	DEBUG("new ulong\n"); 
	return blob_put_int(prv, value); 
}

JSOBJ Object_newDouble(void *prv, double value){
	DEBUG("new double\n"); 
	return blob_put_real(prv, value); 
}

static void Object_releaseObject(void *prv, JSOBJ obj){
	DEBUG("close array\n"); 
	// with blobs close_table and close_array is the same function
	blob_close_table(prv, obj); 
}

static void *Object_Malloc(size_t size){
	DEBUG("alloc %lu bytes\n", size); 
	return malloc(size); 
}

static void Object_Free(void *ptr){
	DEBUG("free object\n"); 
	free(ptr); 
}

static void *Object_Realloc(void *ptr, size_t size){
	DEBUG("Object realloc\n"); 
	return realloc(ptr, size); 
}

bool blob_put_json(struct blob *self, const char *json){
	JSONObjectDecoder decoder = {
		.newString = Object_newString,
		.objectAddKey = Object_objectAddKey,
		.arrayAddItem = Object_arrayAddItem,
		.newTrue = Object_newTrue,
		.newFalse = Object_newFalse,
		.newNull = Object_newNull,
		.newObject = Object_newObject,
		.newArray = Object_newArray,
		.newInt = Object_newInteger,
		.newLong = Object_newLong,
		.newUnsignedLong = Object_newUnsignedLong,
		.newDouble = Object_newDouble,
		.releaseObject = Object_releaseObject,
		.malloc = Object_Malloc,
		.free = Object_Free,
		.realloc = Object_Realloc,
		.errorStr = 0,
		.errorOffset = 0,
		.preciseFloat = 1,
		.prv = self
	};

	JSON_DecodeObject(&decoder, json, strlen(json));

	if (decoder.errorStr){
		DEBUG("json parsing failed: %s", decoder.errorStr);
		return false;
	}

	return true;
}

