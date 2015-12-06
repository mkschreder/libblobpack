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
bool blob_buf_grow(struct blob_buf *buf, int minlen){
	char *new = 0;
	int delta = ((minlen / 256) + 1) * 256;
	new = realloc(buf->buf, buf->buflen + delta);
	if (new) {
		int offset_head = _blob_buf_attr_to_offset(buf, buf->head);
		buf->buf = new;
		memset(buf->buf + buf->buflen, 0, delta);
		buf->buflen += delta;
		buf->head = _blob_buf_offset_to_attr(buf, offset_head);
	}
	return !!new;
}

static struct blob_attr *
_blob_buf_add(struct blob_buf *buf, struct blob_attr *pos, int id, int payload)
{
	int offset = _blob_buf_attr_to_offset(buf, pos);
	int required = (offset + sizeof(struct blob_attr) + payload) - buf->buflen;
	struct blob_attr *attr;

	if (required > 0) {
		if (!blob_buf_grow(buf, required))
			return NULL;
		attr = _blob_buf_offset_to_attr(buf, offset);
	} else {
		attr = pos;
	}

	blob_attr_init(attr, id, payload + sizeof(struct blob_attr));
	blob_attr_fill_pad(attr);

	return attr;
}

int blob_buf_reinit(struct blob_buf *buf, int type){
	buf->buf = buf->head; 

	return 0;
}

int blob_buf_init(struct blob_buf *buf, const char *data, size_t size){
	memset(buf, 0, sizeof(struct blob_buf)); 

	buf->buflen = (size > 0)?size:256; 
	buf->buf = malloc(buf->buflen); 
	if(!buf->buf) return -ENOMEM; 

	if(data) {
		memcpy(buf->buf, data, size); 
		buf->head = buf->buf; 
	} else {
		memset(buf->buf, 0, buf->buflen); 
		buf->head = buf->buf; 
		blob_attr_init(buf->head, 0, sizeof(struct blob_attr)); 
		blob_attr_fill_pad(buf->head); 
	}
	return 0; 
}

void blob_buf_free(struct blob_buf *buf){
	free(buf->buf);
	buf->buf = NULL;
	buf->buflen = 0;
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
	if (!attr)
		return NULL;

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
