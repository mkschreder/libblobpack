/*
	Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 	Copyright (C) 2015 Martin Schröder <mkschreder.uk@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <endian.h>


#include "blob_field.h"

// arbitrary max size just to make sure we don't try to resize to a crazy value (should be something large yet reasonable!)
#define BLOB_MAX_SIZE (10000000)

/*
Blob attributes can have any of the following types. 
*/ 

enum {
	BLOB_FIELD_INVALID, 
	BLOB_FIELD_BINARY, // a binary blob. We don't care about content. 
	BLOB_FIELD_STRING, // a null terminated string
	BLOB_FIELD_INT8, // an 8 bit signed/unsigned integer 
	BLOB_FIELD_INT16, // a 16 bit signed/unsigned integer
	BLOB_FIELD_INT32, // a 32 bit signed/unsigned int
	BLOB_FIELD_INT64, // a 64 bit signed/unsigned int
	BLOB_FIELD_FLOAT32, // a packed 32 bit float
	BLOB_FIELD_FLOAT64, // a packed 64 bit float
	BLOB_FIELD_ARRAY, // only unnamed elements
	BLOB_FIELD_TABLE, // only named elements
	BLOB_FIELD_ANY, // to be used only as a wildcard
	BLOB_FIELD_LAST
};

struct blob {
	size_t memlen; // total length of the allocated memory area 
	void *buf; // raw buffer data
};

struct blob_policy {
	const char *name; 
	int type; 
	const struct blob_field *value; 
}; 

//! Initializes a blob structure. Optionally takes memory area to be copied into the buffer which must represent a valid blob buf. 
void blob_init(struct blob *buf, const char *data, size_t size);
//! Frees the memory allocated with the buffer
void blob_free(struct blob *buf);
//! Resets header but does not deallocate any memory.  
void blob_reset(struct blob *buf);
//! Resizes the buffer. Can only be used to increase size.  
bool blob_resize(struct blob *buf, uint32_t newsize);

//! Returns pointer to header attribute (the first element) which is also raw buffer
static inline struct blob_field *blob_head(struct blob *self){
	return (struct blob_field*)self->buf; 
}

static inline const struct blob_field *blob_head_const(const struct blob *self){
	return (const struct blob_field*)self->buf; 
}

//! returns size of the whole buffer (including header element and padding)
static inline uint32_t blob_size(struct blob *self){ return blob_field_raw_pad_len(blob_head(self)); }

#ifdef __AVR
typedef uint16_t blob_offset_t; 
#else
typedef void* blob_offset_t; 
#endif

/********************************
** NESTED ELEMENTS 
********************************/

//! opens an array element
blob_offset_t 	blob_open_array(struct blob *buf);
//! closes an array element
void 			blob_close_array(struct blob *buf, blob_offset_t);
//! opens an table element
blob_offset_t 	blob_open_table(struct blob *buf);
//! closes an table element
void 			blob_close_table(struct blob *buf, blob_offset_t);

/********************************
** WRITING FUNCTIONS
********************************/

//! Write a string at the end of the buffer 
/*
struct blob_field *blob_put_u8(struct blob *buf, uint8_t val); 
struct blob_field *blob_put_u16(struct blob *buf, uint16_t val); 
struct blob_field *blob_put_u32(struct blob *buf, uint32_t val); 
struct blob_field *blob_put_u64(struct blob *buf, uint64_t val); 

#define blob_put_i8		blob_put_u8
#define blob_put_i16	blob_put_u16
#define blob_put_i32	blob_put_u32
#define blob_put_i64	blob_put_u64
*/

//! write a boolean into the buffer
struct blob_field *blob_put_bool(struct blob *buf, bool val); 

//! write a string into the buffer
struct blob_field *blob_put_string(struct blob *buf, const char *str); 

//! write binary data into the buffer
// NOTE: binary no longer supported because it is not representable in json and also because for binary data we actually need a whole new field type since we need to use size and our blob size is always padded. 
// for binary use an array with integers instead
//struct blob_field *blob_put_binary(struct blob *buf, const void *data, unsigned int size); 

//! write a number into the buffer
struct blob_field *blob_put_int(struct blob *buf, long long val); 

//! write a real into the buffer
struct blob_field *blob_put_real(struct blob *buf, double value); 

//! write a raw attribute into the buffer
struct blob_field *blob_put_attr(struct blob *buf, const struct blob_field *attr); 

//! print out the whole buffer 
void blob_dump(struct blob *self); 

