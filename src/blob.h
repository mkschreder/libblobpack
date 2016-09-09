/*
 * blob - library for generating/parsing tagged binary data
 *
 * Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _BLOB_H__
#define _BLOB_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "blob_field.h"
#include "utils.h"

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
	struct blob_field *value; 
}; 

//! Initializes a blob structure. Optionally takes memory area to be copied into the buffer which must represent a valid blob buf. 
void blob_init(struct blob *buf, const char *data, size_t size);
//! Frees the memory allocated with the buffer
void blob_free(struct blob *buf);
//! Resets header but does not deallocate any memory.  
void blob_reset(struct blob *buf);
//! Resizes the buffer. Can only be used to increase size.  
bool blob_resize(struct blob *buf, int newsize);

//! Returns pointer to header attribute (the first element) which is also raw buffer
static inline struct blob_field *blob_head(struct blob *self){
	return (struct blob_field*)self->buf; 
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
struct blob_field *blob_put_attr(struct blob *buf, struct blob_field *attr); 

//! print out the whole buffer 
void blob_dump(struct blob *self); 

#endif
