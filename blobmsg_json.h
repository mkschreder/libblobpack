/*
 * Copyright (C) 2010-2012 Felix Fietkau <nbd@openwrt.org>
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
#ifndef __BLOBMSG_JSON_H
#define __BLOBMSG_JSON_H

struct json_object;

#include <stdbool.h>
#include "blob.h"

bool blob_buf_add_json_object(struct blob_buf *b, struct json_object *obj);
bool blob_buf_add_json_element(struct blob_buf *b, struct json_object *obj);
bool blob_buf_add_json_from_string(struct blob_buf *b, const char *str);
bool blob_buf_add_json_from_file(struct blob_buf *b, const char *file);

typedef const char *(*blob_json_format_t)(void *priv, struct blob_attr *attr);

char *blob_buf_format_json_with_cb(struct blob_attr *attr, bool list,
				  blob_json_format_t cb, void *priv,
				  int indent);

static inline char *blob_buf_format_json(struct blob_attr *attr, bool list){
	return blob_buf_format_json_with_cb(attr, list, NULL, NULL, -1);
}

static inline char *blob_buf_format_json_indent(struct blob_attr *attr, bool list, int indent){
	return blob_buf_format_json_with_cb(attr, list, NULL, NULL, indent);
}

static inline void blob_attr_dump_json(struct blob_attr *self){
	char *json = blob_buf_format_json(self, true); 
	printf("%s\n", json); 
	free(json); 
}

#endif
