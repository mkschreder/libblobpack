/*
 * blob - library for generating/parsing tagged binary data
 *
 * Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "blob.h"

static bool
blob_buffer_grow(struct blob_buf *buf, int minlen)
{
	int delta = ((minlen / 256) + 1) * 256;
	buf->buflen += delta;
	buf->buf = realloc(buf->buf, buf->buflen);
	if (buf->buf)
		memset(buf->buf + buf->buflen - delta, 0, delta);
	return !!buf->buf;
}

static void
blob_init(struct blob_attr *attr, int id, unsigned int len)
{
	len &= BLOB_ATTR_LEN_MASK;
	len |= (id << BLOB_ATTR_ID_SHIFT) & BLOB_ATTR_ID_MASK;
	attr->id_len = cpu_to_be32(len);
}

static inline struct blob_attr *
offset_to_attr(struct blob_buf *buf, int offset)
{
	void *ptr = (char *)buf->buf + offset;
	return ptr;
}

static inline int
attr_to_offset(struct blob_buf *buf, struct blob_attr *attr)
{
	return (char *)attr - (char *) buf->buf;
}

static struct blob_attr *
blob_add(struct blob_buf *buf, struct blob_attr *pos, int id, int payload)
{
	int offset = attr_to_offset(buf, pos);
	int required = (offset + sizeof(struct blob_attr) + payload) - buf->buflen;
	struct blob_attr *attr;

	if (required > 0) {
		int offset_head = attr_to_offset(buf, buf->head);

		if (!buf->grow || !buf->grow(buf, required))
			return NULL;

		buf->head = offset_to_attr(buf, offset_head);
		attr = offset_to_attr(buf, offset);
	} else {
		attr = pos;
	}

	blob_init(attr, id, payload + sizeof(struct blob_attr));
	return attr;
}

int
blob_buf_init(struct blob_buf *buf, int id)
{
	if (!buf->grow)
		buf->grow = blob_buffer_grow;

	buf->head = buf->buf;
	if (blob_add(buf, buf->buf, id, 0) == NULL)
		return -ENOMEM;

	return 0;
}

void
blob_buf_free(struct blob_buf *buf)
{
	free(buf->buf);
	buf->buf = NULL;
	buf->buflen = 0;
}

static void
blob_fill_pad(struct blob_attr *attr)
{
	char *buf = (char *) attr;
	int len = blob_pad_len(attr);
	int delta = len - blob_raw_len(attr);

	if (delta > 0)
		memset(buf + len - delta, 0, delta);
}

void
blob_set_raw_len(struct blob_attr *attr, unsigned int len)
{
	int id = blob_id(attr);
	len &= BLOB_ATTR_LEN_MASK;
	len |= (id << BLOB_ATTR_ID_SHIFT) & BLOB_ATTR_ID_MASK;
	attr->id_len = cpu_to_be32(len);
	blob_fill_pad(attr);
}

struct blob_attr *
blob_new(struct blob_buf *buf, int id, int payload)
{
	struct blob_attr *attr;

	attr = blob_add(buf, blob_next(buf->head), id, payload);
	if (!attr)
		return NULL;

	blob_set_raw_len(buf->head, blob_pad_len(buf->head) + blob_pad_len(attr));
	return attr;
}

struct blob_attr *
blob_put(struct blob_buf *buf, int id, const void *ptr, int len)
{
	struct blob_attr *attr;

	attr = blob_new(buf, id, len);
	if (!attr)
		return NULL;

	if (ptr)
		memcpy(blob_data(attr), ptr, len);
	return attr;
}

void *
blob_nest_start(struct blob_buf *buf, int id)
{
	unsigned long offset = attr_to_offset(buf, buf->head);
	buf->head = blob_new(buf, id, 0);
	return (void *) offset;
}

void
blob_nest_end(struct blob_buf *buf, void *cookie)
{
	struct blob_attr *attr = offset_to_attr(buf, (unsigned long) cookie);
	blob_set_raw_len(attr, blob_pad_len(attr) + blob_len(buf->head));
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
blob_check_type(const void *ptr, int len, int type)
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
blob_parse(struct blob_attr *attr, struct blob_attr **data, const struct blob_attr_info *info, int max)
{
	struct blob_attr *pos;
	int found = 0;
	int rem;

	memset(data, 0, sizeof(struct blob_attr *) * max);
	blob_for_each_attr(pos, attr, rem) {
		int id = blob_id(pos);
		int len = blob_len(pos);

		if (id >= max)
			continue;

		if (info) {
			int type = info[id].type;

			if (type < BLOB_ATTR_LAST) {
				if (!blob_check_type(blob_data(pos), len, type))
					continue;
			}

			if (info[id].minlen && len < info[id].minlen)
				continue;

			if (info[id].maxlen && len > info[id].maxlen)
				continue;

			if (info[id].validate && !info[id].validate(&info[id], attr))
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

	if (blob_pad_len(a1) != blob_pad_len(a2))
		return false;

	return !memcmp(a1, a2, blob_pad_len(a1));
}
