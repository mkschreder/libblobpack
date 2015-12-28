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

#include "blob_attr.h"
#include "utils.h"

// arbitrary max size just to make sure we don't try to resize to a crazy value (should be something large yet reasonable!)
#define BLOB_MAX_SIZE (10000000)

/*
Blob attributes can have any of the following types. 
*/ 

enum {
	BLOB_ATTR_ROOT, // this is root element. For historical reasons it is has type of 0 
	//BLOB_ATTR_NESTED, // this element conains any number of other elements. Size is the padded length of all child elements combined. 
	BLOB_ATTR_BINARY, // a binary blob. We don't care about content. 
	BLOB_ATTR_STRING, // a null terminated string
	BLOB_ATTR_INT8, // an 8 bit signed/unsigned integer 
	BLOB_ATTR_INT16, // a 16 bit signed/unsigned integer
	BLOB_ATTR_INT32, // a 32 bit signed/unsigned int
	BLOB_ATTR_INT64, // a 64 bit signed/unsigned int
	BLOB_ATTR_FLOAT32, // a packed 32 bit float
	BLOB_ATTR_FLOAT64, // a packed 64 bit float
	BLOB_ATTR_ARRAY, // only unnamed elements
	BLOB_ATTR_TABLE, // only named elements
	BLOB_ATTR_LAST
};

struct blob_buf {
	size_t memlen; // total length of the allocated memory area 
	void *buf; // raw buffer data
};

//! Initializes a blob_buf structure. Optionally takes memory area to be copied into the buffer which must represent a valid blob buf. 
void blob_buf_init(struct blob_buf *buf, const char *data, size_t size);
//! Frees the memory allocated with the buffer
void blob_buf_free(struct blob_buf *buf);
//! Resets header but does not deallocate any memory.  
void blob_buf_reset(struct blob_buf *buf);
//! Resizes the buffer. Can only be used to increase size.  
bool blob_buf_resize(struct blob_buf *buf, int newsize);

//! Returns pointer to header attribute (the first element) which is also raw buffer
static inline struct blob_attr *blob_buf_head(struct blob_buf *self){
	return (struct blob_attr*)self->buf; 
}

//! returns size of the whole buffer (including header element and padding)
static inline size_t blob_buf_size(struct blob_buf *self){ return blob_attr_pad_len(blob_buf_head(self)); }

typedef void* blob_offset_t; 

/********************************
** NESTED ELEMENTS 
********************************/

//! opens an array element
blob_offset_t 	blob_buf_open_array(struct blob_buf *buf);
//! closes an array element
void 			blob_buf_close_array(struct blob_buf *buf, blob_offset_t);
//! opens an table element
blob_offset_t 	blob_buf_open_table(struct blob_buf *buf);
//! closes an table element
void 			blob_buf_close_table(struct blob_buf *buf, blob_offset_t);

/********************************
** WRITING FUNCTIONS
********************************/

//! Write a string at the end of the buffer 
struct blob_attr *blob_buf_put_string(struct blob_buf *buf, const char *str); 
struct blob_attr *blob_buf_put_u8(struct blob_buf *buf, uint8_t val); 
struct blob_attr *blob_buf_put_u16(struct blob_buf *buf, uint16_t val); 
struct blob_attr *blob_buf_put_u32(struct blob_buf *buf, uint32_t val); 
struct blob_attr *blob_buf_put_u64(struct blob_buf *buf, uint64_t val); 

#define blob_buf_put_i8		blob_buf_put_u8
#define blob_buf_put_i16	blob_buf_put_u16
#define blob_buf_put_i32	blob_buf_put_u32
#define blob_buf_put_i64	blob_buf_put_u64

struct blob_attr *blob_buf_put_float(struct blob_buf *buf, float value); 
struct blob_attr *blob_buf_put_double(struct blob_buf *buf, double value); 

struct blob_attr *blob_buf_put_attr(struct blob_buf *buf, struct blob_attr *attr); 

//! print out the whole buffer 
void blob_buf_dump(struct blob_buf *self); 

#endif
