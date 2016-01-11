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
#include <inttypes.h>
#include "blob.h"
#include "blob_json.h"

#include <json-c/json.h>

typedef const char *(*blob_json_format_t)(void *priv, struct blob_field *attr);
/*
bool blob_put_json_object(struct blob *b, json_object *obj)
{
	json_object_object_foreach(obj, key, val) {
		blob_put_string(b, key); 
		if (!blob_put_json_element(b, val))
			break; 
	}
	return true;
}

static bool blob_put_json_array(struct blob *b, struct array_list *a)
{
	int i, len;

	for (i = 0, len = array_list_length(a); i < len; i++) {
		if (!blob_put_json_element(b, array_list_get_idx(a, i)))
			return false;
	}

	return true;
}

bool blob_put_json_element(struct blob *b, json_object *obj)
{
	bool ret = true;
	void *c;

	if (!obj)
		return false;

	switch (json_object_get_type(obj)) {
	case json_type_object:
		c = blob_open_table(b);
		ret = blob_put_json_object(b, obj);
		blob_close_table(b, c);
		break;
	case json_type_array:
		c = blob_open_array(b);
		ret = blob_put_json_array(b, json_object_get_array(obj));
		blob_close_array(b, c);
		break;
	case json_type_string:
		blob_put_string(b, json_object_get_string(obj));
		break;
	case json_type_boolean:
		blob_put_bool(b, json_object_get_boolean(obj));
		break;
	case json_type_int:
		blob_put_int(b, json_object_get_int(obj));
		break;
	case json_type_double: 
		blob_put_float(b, json_object_get_double(obj)); 
		break; 
	default:
		return false;
	}
	return ret;
}

static bool __blob_add_json(struct blob *b, json_object *obj)
{
	bool ret = false;

	if (!obj)
		return false;

	ret = blob_put_json_element(b, obj); 
	json_object_put(obj);
	return ret;
}

bool blob_put_json_from_file(struct blob *b, const char *file){
	return __blob_add_json(b, json_object_from_file(file));
}

bool blob_put_json_from_string(struct blob *b, const char *str){
	return __blob_add_json(b, json_tokener_parse(str));
}
*/
struct strbuf {
	int len;
	int pos;
	char *buf;

	blob_json_format_t custom_format;
	void *priv;
	bool indent;
	int indent_level;
};

static bool blob_puts(struct strbuf *s, const char *c, int len)
{
	if (len <= 0)
		return true;

	if (s->pos + len >= s->len) {
		s->len += 16 + len;
		s->buf = realloc(s->buf, s->len);
		if (!s->buf)
			return false;
	}
	memcpy(s->buf + s->pos, c, len);
	s->pos += len;
	return true;
}

static void add_separator(struct strbuf *s)
{
	static char indent_chars[17] = "\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	int indent;
	char *start;

	if (!s->indent)
		return;

	indent = s->indent_level;
	if (indent > 16)
		indent = 16;

	start = &indent_chars[sizeof(indent_chars) - indent - 1];
	*start = '\n';
	blob_puts(s, start, indent + 1);
	*start = '\t';
}


static void blob_format_string(struct strbuf *s, const char *str)
{
	const unsigned char *p, *last, *end;
	char buf[8] = "\\u00";

	end = (unsigned char *) str + strlen(str);
	blob_puts(s, "\"", 1);
	for (p = (unsigned char *) str, last = p; *p; p++) {
		char escape = '\0';
		int len;

		switch(*p) {
		case '\b':
			escape = 'b';
			break;
		case '\n':
			escape = 'n';
			break;
		case '\t':
			escape = 't';
			break;
		case '\r':
			escape = 'r';
			break;
		case '"':
		case '\\':
		case '/':
			escape = *p;
			break;
		default:
			if (*p < ' ')
				escape = 'u';
			break;
		}

		if (!escape)
			continue;

		if (p > last)
			blob_puts(s, (char *) last, p - last);
		last = p + 1;
		buf[1] = escape;

		if (escape == 'u') {
			sprintf(buf + 4, "%02x", (unsigned char) *p);
			len = 6;
		} else {
			len = 2;
		}
		blob_puts(s, buf, len);
	}

	blob_puts(s, (char *) last, end - last);
	blob_puts(s, "\"", 1);
}

static void blob_format_json_list(struct strbuf *s, struct blob_field *attr, bool array);

static void blob_format_element(struct strbuf *s, struct blob_field *attr, bool array, bool head)
{
	const char *data_str;
	char buf[32];
	void *data;

	//if (!blob_check_attr(attr, false))
	//		return;

	data = blob_field_data(attr);

	if (!head && s->custom_format) {
		data_str = s->custom_format(s->priv, attr);
		if (data_str)
			goto out;
	}

	data_str = buf;
	memset(buf, 0, sizeof(buf)); 

	switch(blob_field_type(attr)) {
	case BLOB_FIELD_INVALID:
		sprintf(buf, "(invalid)");
		break;
	//case BLOB_FIELD_BOOL:
	//	sprintf(buf, "%s", *(uint8_t *)data ? "true" : "false");
//		break;
	case BLOB_FIELD_INT8: 
		sprintf(buf, "%d", *(uint8_t*)data); 
		break; 
	case BLOB_FIELD_INT16:
		sprintf(buf, "%d", be16_to_cpu(*(uint16_t *)data));
		break;
	case BLOB_FIELD_INT32:
		sprintf(buf, "%d", (int32_t) be32_to_cpu(*(uint32_t *)data));
		break;
	case BLOB_FIELD_INT64:
		sprintf(buf, "%" PRId64, (int64_t) be64_to_cpu(*(uint64_t *)data));
		break;
	case BLOB_FIELD_FLOAT32: 
		sprintf(buf, "%f", (float) unpack754_32(be32_to_cpu(*(uint32_t*)data))); 
		break; 
	case BLOB_FIELD_FLOAT64: 
		sprintf(buf, "%Le", unpack754_64(be64_to_cpu(*(uint64_t*)data))); 
		break; 
	case BLOB_FIELD_STRING:
		blob_format_string(s, blob_field_data(attr));
		return;
	case BLOB_FIELD_ARRAY:
		blob_format_json_list(s, attr, true);
		return;
	case BLOB_FIELD_TABLE:
		blob_format_json_list(s, attr, false);
		return;
	}

out:
	blob_puts(s, data_str, strlen(data_str));
}

static void blob_format_json_list(struct strbuf *s, struct blob_field *attr, bool array){
	struct blob_field *pos;
	bool first = true;

	blob_puts(s, (array ? "[" : "{" ), 1);
	s->indent_level++;
	add_separator(s);

	//__blob_for_each_attr(pos, attr, rem) {
	for(pos = blob_field_first_child(attr); pos; pos = blob_field_next_child(attr, pos)){
		if (!first) {
			blob_puts(s, ",", 1);
			add_separator(s);
		}
		
		if(!array){
			blob_format_string(s, blob_field_data(pos)); 
			blob_puts(s, ": ", s->indent ? 2 : 1);
			pos = blob_field_next_child(attr, pos); 
		}
		blob_format_element(s, pos, array, false);
		first = false;
	}
	s->indent_level--;
	add_separator(s);
	blob_puts(s, (array ? "]" : "}"), 1);
}

char *blob_format_json_with_cb(struct blob_field *attr, bool list, blob_json_format_t cb, void *priv, int indent)
{
	struct strbuf s;
	bool array;

	s.len = blob_field_data_len(attr);
	s.buf = malloc(s.len);
	s.pos = 0;
	s.custom_format = cb;
	s.priv = priv;
	s.indent = false;

	if (indent >= 0) {
		s.indent = true;
		s.indent_level = indent;
	}

	array = blob_field_type(attr) == BLOB_FIELD_ARRAY;
	
	if (list)
		blob_format_json_list(&s, attr, array);
	else
		blob_format_element(&s, attr, false, false);

	if (!s.len) {
		free(s.buf);
		return NULL;
	}

	s.buf = realloc(s.buf, s.pos + 1);
	s.buf[s.pos] = 0;

	return s.buf;
}

char *blob_field_to_json(struct blob_field *attr){
	return blob_format_json_with_cb(attr, false, NULL, NULL, -1);
}

char *blob_field_to_json_pretty(struct blob_field *attr){
	return blob_format_json_with_cb(attr, false, NULL, NULL, 1);
}

static void _blob_field_dump_json(struct blob_field *self, int indent){
	assert(self); 
	char *json = NULL; 
	if(indent == 1)
		json = blob_field_to_json(self); 
	else 
		json = blob_field_to_json_pretty(self); 
	printf("%s\n", json); 
	free(json); 
}

void blob_field_dump_json(struct blob_field *self){
	assert(self); 
	_blob_field_dump_json(self, 0); 
}

void blob_field_dump_json_pretty(struct blob_field *self){
	assert(self); 
	_blob_field_dump_json(self, 1); 
}

