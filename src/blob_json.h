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
/*
bool blob_put_json_object(struct blob *b, struct json_object *obj);
bool blob_put_json_element(struct blob *b, struct json_object *obj);
bool blob_put_json_from_string(struct blob *b, const char *str);
bool blob_put_json_from_file(struct blob *b, const char *file);
char *blob_format_json(struct blob_field *attr, bool list); 
char *blob_format_json_indent(struct blob_field *attr, bool list, int indent); 
*/

void blob_field_dump_json(const struct blob_field *self); 
void blob_field_dump_json_pretty(const struct blob_field *self); 

static inline void blob_dump_json(const struct blob *self){ blob_field_dump_json(blob_head_const(self)); }

char *blob_field_to_json(const struct blob_field *self); 
static inline char *blob_to_json(const struct blob *self){ return blob_field_to_json(blob_head_const(self)); }

bool blob_init_from_json(struct blob *self, const char *json); 

bool blob_put_json(struct blob *self, const char *json); 
bool blob_put_json_from_file(struct blob *self, const char *file); 

#endif
