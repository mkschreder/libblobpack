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

#include "blob.h"

static void blob_attr_init(struct blob_attr *attr, int id, unsigned int len){
	memset(attr, 0, sizeof(struct blob_attr)); 
	len &= BLOB_ATTR_LEN_MASK;
	len |= (id << BLOB_ATTR_ID_SHIFT) & BLOB_ATTR_ID_MASK;
	attr->id_len = cpu_to_be32(len);
}

static inline struct blob_attr *
_blob_buf_offset_to_attr(struct blob_buf *buf, int offset)
{
	void *ptr = (char *)buf->buf + offset;
	return ptr;
}

static inline int
_blob_buf_attr_to_offset(struct blob_buf *buf, struct blob_attr *attr)
{
	return (char *)attr - (char *) buf->buf;
}

//! Attepts to reallocate the buffer to fit the new payload data
bool blob_buf_resize(struct blob_buf *buf, int minlen){
	char *new = 0;
	int newsize = ((minlen / 256) + 1) * 256;
	// reallocate the memory of the buffer if we no longer have any memory left
	if(newsize > buf->memlen){
		new = realloc(buf->buf, newsize);
		if (new) {
			int offset_head = _blob_buf_attr_to_offset(buf, buf->head);
			buf->buf = new;
			memset(buf->buf + buf->datalen, 0, newsize - buf->datalen);
			buf->datalen = minlen;
			buf->memlen = newsize;  
			buf->head = _blob_buf_offset_to_attr(buf, offset_head);
		} else { 
			return false; 
		}
	} else {
		// otherwise just increase the datalen without reallocating memory
		buf->datalen = minlen; 
	}
	return true;
}

static struct blob_attr *
_blob_buf_add(struct blob_buf *buf, struct blob_attr *pos, int id, int payload)
{
	int offset = _blob_buf_attr_to_offset(buf, pos); // get offset to attr header in the buffer
	int newsize = offset + sizeof(struct blob_attr) + payload;
	struct blob_attr *attr = pos;

	if (newsize > buf->datalen) {
		// if buffer needs to grow then we grow it
		if (!blob_buf_resize(buf, newsize))
			return NULL;
		attr = _blob_buf_offset_to_attr(buf, offset);
	} 

	blob_attr_init(attr, id, payload + sizeof(struct blob_attr));
	blob_attr_fill_pad(attr);
	
	return attr;
}

int blob_buf_reset(struct blob_buf *buf, int type){
	memset(buf->buf, 0, buf->memlen); 
	buf->head = buf->buf; 
	blob_attr_init(buf->head, type, sizeof(struct blob_attr));
	blob_attr_fill_pad(buf->head);
	return 0;
}

int blob_buf_init(struct blob_buf *buf, const char *data, size_t size){
	memset(buf, 0, sizeof(struct blob_buf)); 

	// default buffer is 256 bytes block with zero sized data
	buf->memlen = (size > 0)?size:256; 
	buf->buf = malloc(buf->memlen); 
	if(!buf->buf) return -ENOMEM; 

	if(data) {
		memcpy(buf->buf, data, size); 
		buf->head = buf->buf; 
		buf->datalen = size; 
	} else {
		memset(buf->buf, 0, buf->memlen); 
		blob_buf_reset(buf, 0); 
	}
	return 0; 
}

void blob_buf_free(struct blob_buf *buf){
	free(buf->buf);
	buf->buf = NULL;
	buf->memlen = 0;
	buf->datalen = 0; 
}

void blob_attr_fill_pad(struct blob_attr *attr) {
	char *buf = (char *) attr;
	int len = blob_attr_pad_len(attr);
	int delta = len - blob_attr_raw_len(attr);

	if (delta > 0)
		memset(buf + len - delta, 0, delta);
}

void
blob_attr_set_raw_len(struct blob_attr *attr, unsigned int len)
{
	len &= BLOB_ATTR_LEN_MASK;
	attr->id_len &= ~cpu_to_be32(BLOB_ATTR_LEN_MASK);
	attr->id_len |= cpu_to_be32(len);
}

struct blob_attr *
blob_buf_new_attr(struct blob_buf *buf, int id, int payload)
{
	struct blob_attr *attr;

	attr = _blob_buf_add(buf, blob_attr_next(buf->head), id, payload);
	if (!attr)
		return NULL;

	blob_attr_set_raw_len(buf->head, blob_attr_pad_len(buf->head) + blob_attr_pad_len(attr));
	return attr;
}

struct blob_attr *
blob_buf_put_raw(struct blob_buf *buf, const void *ptr, unsigned int len)
{
	struct blob_attr *attr;

	if (len < sizeof(struct blob_attr) || !ptr)
		return NULL;

	attr = _blob_buf_add(buf, blob_attr_next(buf->head), 0, len - sizeof(struct blob_attr));
	if (!attr)
		return NULL;
	blob_attr_set_raw_len(buf->head, blob_attr_pad_len(buf->head) + len);
	memcpy(attr, ptr, len);
	return attr;
}

struct blob_attr *
blob_buf_put(struct blob_buf *buf, int id, const void *ptr, unsigned int len)
{
	struct blob_attr *attr;

	attr = blob_buf_new_attr(buf, id, len);
	if (!attr) {
		return NULL;
	}

	if (ptr)
		memcpy(blob_attr_data(attr), ptr, len);

	return attr;
}

void *
blob_buf_nest_start(struct blob_buf *buf, int id)
{
	unsigned long offset = _blob_buf_attr_to_offset(buf, buf->head);
	buf->head = blob_buf_new_attr(buf, id, 0);
	if (!buf->head)
		return NULL;
	return (void *) offset;
}

void
blob_buf_nest_end(struct blob_buf *buf, void *cookie)
{
	struct blob_attr *attr = _blob_buf_offset_to_attr(buf, (unsigned long) cookie);
	blob_attr_set_raw_len(attr, blob_attr_pad_len(attr) + blob_attr_len(buf->head));
	buf->head = attr;
}

static const int blob_type_minlen[BLOB_ATTR_LAST] = {
	[BLOB_ATTR_STRING] = 1,
	[BLOB_ATTR_INT8] = sizeof(uint8_t),
	[BLOB_ATTR_INT16] = sizeof(uint16_t),
	[BLOB_ATTR_INT32] = sizeof(uint32_t),
	[BLOB_ATTR_INT64] = sizeof(uint64_t),
};

bool
blob_attr_check_type(const void *ptr, unsigned int len, int type)
{
	const char *data = ptr;

	if (type >= BLOB_ATTR_LAST)
		return false;

	if (type >= BLOB_ATTR_INT8 && type <= BLOB_ATTR_INT64) {
		if (len != blob_type_minlen[type])
			return false;
	} else {
		if (len < blob_type_minlen[type])
			return false;
	}

	if (type == BLOB_ATTR_STRING && data[len - 1] != 0)
		return false;

	return true;
}

int
blob_attr_parse(struct blob_attr *attr, struct blob_attr **data, const struct blob_attr_info *info, int max)
{
	struct blob_attr *pos;
	int found = 0;
	int rem;

	memset(data, 0, sizeof(struct blob_attr *) * max);
	blob_buf_for_each_attr(pos, attr, rem) {
		int id = blob_attr_id(pos);
		int len = blob_attr_len(pos);

		if (id >= max)
			continue;

		if (info) {
			int type = info[id].type;

			if (type < BLOB_ATTR_LAST) {
				if (!blob_attr_check_type(blob_attr_data(pos), len, type))
					continue;
			}

			if (info[id].minlen && len < info[id].minlen)
				continue;

			if (info[id].maxlen && len > info[id].maxlen)
				continue;

			if (info[id].validate && !info[id].validate(&info[id], pos))
				continue;
		}

		if (!data[id])
			found++;

		data[id] = pos;
	}
	return found;
}

bool
blob_attr_equal(const struct blob_attr *a1, const struct blob_attr *a2)
{
	if (!a1 && !a2)
		return true;

	if (!a1 || !a2)
		return false;

	if (blob_attr_pad_len(a1) != blob_attr_pad_len(a2))
		return false;

	return !memcmp(a1, a2, blob_attr_pad_len(a1));
}

struct blob_attr *
blob_memdup(struct blob_attr *attr)
{
	struct blob_attr *ret;
	int size = blob_attr_pad_len(attr);

	ret = malloc(size);
	if (!ret)
		return NULL;

	memcpy(ret, attr, size);
	return ret;
}

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(long double f, unsigned bits, unsigned expbits){
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits){
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0) return 0.0;

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}

float blob_attr_get_float(const struct blob_attr *attr){
	return unpack754_32(blob_attr_get_u32(attr)); 
}

double blob_attr_get_double(const struct blob_attr *attr){
	return unpack754_64(blob_attr_get_u64(attr)); 
}

struct blob_attr *blob_buf_put_float(struct blob_buf *buf, int id, float value){
	return blob_buf_put_u32(buf, id, pack754_32(value)); 
}
 
struct blob_attr *blob_buf_put_double(struct blob_buf *buf, int id, double value){
	return blob_buf_put_u64(buf, id, pack754_64(value));
} 


