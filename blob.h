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

#include "utils.h"

//#define BLOB_COOKIE		0x01234567

enum {
	BLOB_ATTR_UNSPEC,
	BLOB_ATTR_NESTED,
	BLOB_ATTR_BINARY,
	BLOB_ATTR_STRING,
	BLOB_ATTR_INT8,
	BLOB_ATTR_INT16,
	BLOB_ATTR_INT32,
	BLOB_ATTR_INT64,
	BLOB_ATTR_FLOAT32, 
	BLOB_ATTR_FLOAT64,
	BLOB_ATTR_LAST
};

#define BLOB_ATTR_ID_MASK  0x7f000000
#define BLOB_ATTR_ID_SHIFT 24
#define BLOB_ATTR_LEN_MASK 0x00ffffff
#define BLOB_ATTR_ALIGN    4
#define BLOB_ATTR_EXTENDED 0x80000000

struct blob_attr {
	uint32_t id_len;
	char data[];
} __packed;

struct blob_attr_info {
	unsigned int type;
	unsigned int minlen;
	unsigned int maxlen;
	bool (*validate)(const struct blob_attr_info *, struct blob_attr *);
};

struct blob_buf {
	//struct blob_attr *head; // pointer to current head
	//struct blob_attr *cursor; // pointer to current position in the buffer (when adding or removing items)
	//size_t cursor; 
	//size_t datalen; // length of filled data in the buffer
	size_t memlen; // total length of the allocated memory area 
	void *buf; // raw buffer data
};

//! Returns first attribute of the buffer which is always null attribute with length set to length of the whole buffer
static inline struct blob_attr *blob_buf_head(struct blob_buf *self){
	return (struct blob_attr*)self->buf; 
}

/*
 * blob_data: returns the data pointer for an attribute
 */
static inline void *
blob_attr_data(const struct blob_attr *attr){
	if(!attr) return NULL; 
	return (void *) attr->data;
}

/*
 * blob_id: returns the id of an attribute
 */
static inline unsigned int
blob_attr_id(const struct blob_attr *attr){
	if(!attr) return -1; 
	int id = (be32_to_cpu(attr->id_len) & BLOB_ATTR_ID_MASK) >> BLOB_ATTR_ID_SHIFT;
	return id;
}

static inline bool
blob_attr_is_extended(const struct blob_attr *attr){
	if(!attr) return false; 
	return !!(attr->id_len & cpu_to_be32(BLOB_ATTR_EXTENDED));
}

/*
 * blob_raw_len: returns the complete length of an attribute (including the header)
 */
static inline unsigned int
blob_attr_raw_len(const struct blob_attr *attr){
	if(!attr) return 0; 
	return (be32_to_cpu(attr->id_len) & BLOB_ATTR_LEN_MASK); 
}

/*
 * blob_len: returns the length of the attribute's payload
 */
static inline unsigned int
blob_attr_len(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_raw_len(attr) - sizeof(struct blob_attr);
}

/*
 * blob_pad_len: returns the padded length of an attribute (including the header)
 */
static inline unsigned int
blob_attr_pad_len(const struct blob_attr *attr){
	if(!attr) return 0; 
	unsigned int len = blob_attr_raw_len(attr);
	len = (len + BLOB_ATTR_ALIGN - 1) & ~(BLOB_ATTR_ALIGN - 1);
	return len;
}

static inline uint8_t
blob_attr_get_u8(const struct blob_attr *attr){
	if(!attr) return 0; 
	return *((uint8_t *) attr->data);
}

static inline uint16_t
blob_attr_get_u16(const struct blob_attr *attr){
	if(!attr) return 0; 
	uint16_t *tmp = (uint16_t*)attr->data;
	return be16_to_cpu(*tmp);
}

static inline uint32_t
blob_attr_get_u32(const struct blob_attr *attr){
	if(!attr) return 0; 
	uint32_t *tmp = (uint32_t*)attr->data;
	return be32_to_cpu(*tmp);
}

static inline uint64_t
blob_attr_get_u64(const struct blob_attr *attr){
	if(!attr) return 0; 
	uint32_t *ptr = (uint32_t *) blob_attr_data(attr);
	uint64_t tmp = ((uint64_t) be32_to_cpu(ptr[0])) << 32;
	tmp |= be32_to_cpu(ptr[1]);
	return tmp;
}

static inline int8_t
blob_attr_get_int8(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_get_u8(attr);
}

static inline int16_t
blob_attr_get_int16(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_get_u16(attr);
}

static inline int32_t
blob_attr_get_int32(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_get_u32(attr);
}

static inline int64_t
blob_attr_get_int64(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_get_u64(attr);
}

static inline const char *
blob_attr_get_string(const struct blob_attr *attr){
	if(!attr) return ""; 
	return attr->data;
}

static inline void 
blob_attr_get_raw(const struct blob_attr *attr, uint8_t *data, size_t data_size){
	if(!attr) return; 
	memcpy(data, attr->data, data_size); 
}

float blob_attr_get_float(const struct blob_attr *attr); 
double blob_attr_get_double(const struct blob_attr *attr); 


static inline struct blob_attr *blob_buf_next(struct blob_buf *self, const struct blob_attr *attr){
	struct blob_attr *ret = (struct blob_attr *) ((void *) attr + blob_attr_pad_len(attr));
	// return null if the new element is outside of the buffer
	if((char*)ret >= (char*)(self->buf + blob_attr_pad_len(blob_buf_head(self)))) return NULL; 
	return ret; 
}
static inline struct blob_attr *blob_attr_next(const struct blob_attr *owner, const struct blob_attr *attr){
	struct blob_attr *ret = (struct blob_attr *) ((char *) attr + blob_attr_pad_len(attr));
	assert(attr); 
	size_t offset = (char*)ret - (char*)owner; 
	if(offset >= blob_attr_pad_len(owner)) return NULL; 
	return ret; 
}
extern void blob_attr_fill_pad(struct blob_attr *attr);
extern void blob_attr_set_raw_len(struct blob_attr *attr, unsigned int len);
extern bool blob_attr_equal(const struct blob_attr *a1, const struct blob_attr *a2);
extern int blob_attr_parse(struct blob_attr *attr, struct blob_attr **data, const struct blob_attr_info *info, int max);
extern struct blob_attr *blob_attr_memdup(struct blob_attr *attr);
extern bool blob_attr_check_type(const void *ptr, unsigned int len, int type);

extern int blob_buf_init(struct blob_buf *buf, const char *data, size_t size);
extern void blob_buf_reset(struct blob_buf *buf);
extern void blob_buf_free(struct blob_buf *buf);
extern bool blob_buf_resize(struct blob_buf *buf, int newsize);

static inline void* blob_buf_data(struct blob_buf *buf) { return buf->buf; }
static inline size_t blob_buf_size(struct blob_buf *self){ return blob_attr_pad_len(blob_buf_head(self)); }

extern struct blob_attr *blob_buf_new_attr(struct blob_buf *buf, int id, int payload);
extern void *blob_buf_nest_start(struct blob_buf *buf, int id);
extern void blob_buf_nest_end(struct blob_buf *buf, void *cookie);
extern struct blob_attr *blob_buf_put(struct blob_buf *buf, int id, const void *ptr, unsigned int len);
extern struct blob_attr *blob_buf_put_raw(struct blob_buf *buf, const void *ptr, unsigned int len);
void blob_buf_dump(struct blob_buf *self); 

static inline struct blob_attr *blob_buf_offset_to_attr(struct blob_buf *buf, size_t offset){
	void *ptr = (char *)buf->buf + offset;
	return ptr;
}

static inline size_t blob_buf_attr_to_offset(struct blob_buf *buf, struct blob_attr *attr){
	return (char *)attr - (char *) buf->buf;
}

static inline struct blob_attr *blob_buf_first(struct blob_buf *self){
	return (struct blob_attr*)blob_attr_data(blob_buf_head(self));
}
/*
static inline struct blob_attr *blob_buf_current(struct blob_buf *self){
	return (struct blob_attr*)((char*)self->buf + self->cursor); 
}
*/
/*
static inline struct blob_attr *blob_buf_next(struct blob_buf *self){
	int len = blob_attr_pad_len(blob_buf_current(self));
	if(len < 0 || (self->cursor + len) > self->datalen) return NULL; 
	self->cursor += len; 
	return blob_buf_current(self);
}
*/
static inline struct blob_attr *
blob_buf_put_string(struct blob_buf *buf, int id, const char *str)
{
	return blob_buf_put(buf, id, str, strlen(str) + 1);
}

static inline struct blob_attr *
blob_buf_put_u8(struct blob_buf *buf, int id, uint8_t val)
{
	return blob_buf_put(buf, id, &val, sizeof(val));
}

static inline struct blob_attr *
blob_buf_put_u16(struct blob_buf *buf, int id, uint16_t val)
{
	val = cpu_to_be16(val);
	return blob_buf_put(buf, id, &val, sizeof(val));
}

static inline struct blob_attr *
blob_buf_put_u32(struct blob_buf *buf, int id, uint32_t val)
{
	val = cpu_to_be32(val);
	return blob_buf_put(buf, id, &val, sizeof(val));
}

static inline struct blob_attr *
blob_buf_put_u64(struct blob_buf *buf, int id, uint64_t val)
{
	val = cpu_to_be64(val);
	return blob_buf_put(buf, id, &val, sizeof(val));
}

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(long double f, unsigned bits, unsigned expbits); 
long double unpack754(uint64_t i, unsigned bits, unsigned expbits); 

struct blob_attr *blob_buf_put_float(struct blob_buf *buf, int id, float value); 
struct blob_attr *blob_buf_put_double(struct blob_buf *buf, int id, double value); 

#define blob_buf_put_int8	blob_buf_put_u8
#define blob_buf_put_int16	blob_buf_put_u16
#define blob_buf_put_int32	blob_buf_put_u32
#define blob_buf_put_int64	blob_buf_put_u64

/*
#define __blob_buf_for_each_attr(pos, attr, rem) \
	for (pos = (void *) attr; \
	     rem > 0 && (blob_attr_pad_len(pos) <= rem) && \
	     (blob_attr_pad_len(pos) >= sizeof(struct blob_attr)); \
	     rem -= blob_attr_pad_len(pos), pos = blob_attr_next(attr, pos))


#define blob_buf_for_each_attr(pos, attr, rem) \
	for (rem = attr ? blob_attr_len(attr) : 0, \
	     pos = attr ? blob_attr_data(attr) : 0; \
	     rem > 0 && (blob_attr_pad_len(pos) <= rem) && \
	     (blob_attr_pad_len(pos) >= sizeof(struct blob_attr)); \
	     rem -= blob_attr_pad_len(pos), pos = blob_attr_next(attr, pos))
*/

#endif
