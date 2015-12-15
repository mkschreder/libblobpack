/*
 * blob - library for generating/parsing tagged binary data
 *
 * Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2015 Martin Schröder <mkschreder.uk@gmail.com>
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

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

uint64_t pack754(long double f, unsigned bits, unsigned expbits); 
long double unpack754(uint64_t i, unsigned bits, unsigned expbits); 

static inline struct blob_attr *blob_buf_offset_to_attr(struct blob_buf *buf, blob_offset_t offset){
	void *ptr = (char *)buf->buf + (size_t)offset;
	return ptr;
}

static inline blob_offset_t blob_buf_attr_to_offset(struct blob_buf *buf, struct blob_attr *attr){
	return (blob_offset_t)((char *)attr - (char *) buf->buf);
}

static void blob_attr_init(struct blob_attr *attr, int id, unsigned int len){
	assert(attr); 
	memset(attr, 0, sizeof(struct blob_attr)); 
	//len += sizeof(struct blob_attr); 
	len &= BLOB_ATTR_LEN_MASK;
	len |= (id << BLOB_ATTR_ID_SHIFT) & BLOB_ATTR_ID_MASK;
	attr->id_len = cpu_to_be32(len);
}

//! Attepts to reallocate the buffer to fit the new payload data
bool blob_buf_resize(struct blob_buf *buf, int minlen){
	char *new = 0;
	int newsize = ((minlen / 256) + 1) * 256;
	int cur_size = blob_buf_size(buf); 
	// reallocate the memory of the buffer if we no longer have any memory left
	if(newsize > buf->memlen){
		new = realloc(buf->buf, newsize);
		if (new) {
			buf->buf = new;
			memset(buf->buf + cur_size, 0, newsize - cur_size);
			buf->memlen = newsize;  
		} else { 
			return false; 
		}
	} 
	blob_attr_set_raw_len(blob_buf_head(buf), minlen);  
	blob_attr_fill_pad(blob_buf_head(buf)); 
	return true;
}

void blob_buf_reset(struct blob_buf *buf){
	assert(buf); 
	assert(buf->buf); 
	if(buf->memlen)
		memset(buf->buf, 0, buf->memlen); 

	blob_attr_init(blob_buf_head(buf), BLOB_ATTR_ARRAY, sizeof(struct blob_attr)); 
}

void blob_buf_init(struct blob_buf *buf, const char *data, size_t size){
	memset(buf, 0, sizeof(struct blob_buf)); 

	// default buffer is 256 bytes block with zero sized data
	buf->memlen = (size > 0)?size:256; 
	buf->buf = malloc(buf->memlen); 
	assert(buf->buf); 
	
	if(data) {
		memcpy(blob_attr_data(blob_buf_head(buf)), data, size); 
		blob_attr_init(blob_buf_head(buf), BLOB_ATTR_ARRAY, sizeof(struct blob_attr)); 
		blob_attr_fill_pad(blob_buf_head(buf)); 
	} else {
		blob_buf_reset(buf); 
	}
}

void blob_buf_free(struct blob_buf *buf){
	free(buf->buf);
	buf->buf = NULL;
	buf->memlen = 0;
}

static struct blob_attr *blob_buf_new_attr(struct blob_buf *buf, int id, const char *name, int payload){
	int namelen = 0; 
	int attr_raw_len = sizeof(struct blob_attr) + payload; 
	if(name) {
		namelen = strlen(name)+1; 
		attr_raw_len += blob_name_hdrlen(namelen);
	}
	int attr_pad_len = attr_raw_len + (BLOB_ATTR_ALIGN - ( attr_raw_len % BLOB_ATTR_ALIGN)) % BLOB_ATTR_ALIGN; 

	struct blob_attr *head = blob_buf_head(buf); 
	int cur_len = blob_attr_pad_len(head); 
	int req_len = cur_len + attr_pad_len; 

	//DEBUG("head pad len %d raw len %d payload %d attr_pad_len %d\n", blob_attr_pad_len(head), blob_attr_raw_len(head), payload, attr_pad_len); 
	if (!blob_buf_resize(buf, req_len))
		return NULL;
	
	//DEBUG("adding attr at offset %d - ", cur_len); 

	struct blob_attr *attr = (struct blob_attr*)(buf->buf + cur_len);
	
	blob_attr_init(attr, id, attr_raw_len);
	blob_attr_fill_pad(attr);

	//DEBUG("pad len %d, raw len: %d req len %d\n", blob_attr_pad_len(attr), blob_attr_raw_len(attr), req_len); 

	// update the length of the head element to enclose the whole buffer
	blob_attr_set_raw_len(blob_buf_head(buf), req_len); 

	if(name){
		attr->id_len |= be32_to_cpu(BLOB_ATTR_HAS_NAME);
		struct blob_name_hdr *hdr = (struct blob_name_hdr*)attr->data; 
		hdr->namelen = cpu_to_be16(namelen);
		strncpy((char *) hdr->name, (const char *)name, namelen);
	}

	return attr;
}

struct blob_attr *blob_buf_put_binary(struct blob_buf *buf, const char *name, const void *ptr, unsigned int len){
	struct blob_attr *attr;

	if (len < sizeof(struct blob_attr) || !ptr)
		return NULL;

	attr = blob_buf_new_attr(buf, BLOB_ATTR_BINARY, name, len - sizeof(struct blob_attr));
	if (!attr)
		return NULL;

	memcpy(attr->data, ptr, len);
	return attr;
}

struct blob_attr *blob_buf_put(struct blob_buf *buf, int id, const char *name, const void *ptr, unsigned int len){
	struct blob_attr *attr;

	attr = blob_buf_new_attr(buf, id, name, len);
	if (!attr) {
		return NULL;
	}

	if (ptr)
		memcpy(blob_attr_data(attr), ptr, len);

	return attr;
}

struct blob_attr *blob_buf_put_string(struct blob_buf *buf, const char *name, const char *str){
	return blob_buf_put(buf, BLOB_ATTR_STRING, name, str, strlen(str) + 1);
}

struct blob_attr *blob_buf_put_u8(struct blob_buf *buf, const char *name, uint8_t val){
	return blob_buf_put(buf, BLOB_ATTR_INT8, name, &val, sizeof(val));
}

struct blob_attr *blob_buf_put_u16(struct blob_buf *buf, const char *name, uint16_t val){
	val = cpu_to_be16(val);
	return blob_buf_put(buf, BLOB_ATTR_INT16, name, &val, sizeof(val));
}

struct blob_attr *blob_buf_put_u32(struct blob_buf *buf, const char *name, uint32_t val){
	val = cpu_to_be32(val);
	return blob_buf_put(buf, BLOB_ATTR_INT32, name, &val, sizeof(val));
}

struct blob_attr *blob_buf_put_u64(struct blob_buf *buf, const char *name, uint64_t val){
	val = cpu_to_be64(val);
	return blob_buf_put(buf, BLOB_ATTR_INT64, name, &val, sizeof(val));
}

blob_offset_t blob_buf_open_array(struct blob_buf *buf, const char *name){
	struct blob_attr *attr = blob_buf_new_attr(buf, BLOB_ATTR_ARRAY, name, 0);
	return blob_buf_attr_to_offset(buf, attr);
}

void blob_buf_close_array(struct blob_buf *buf, blob_offset_t offset){
	struct blob_attr *attr = blob_buf_offset_to_attr(buf, offset);
	int len = (buf->buf + blob_attr_raw_len(blob_buf_head(buf))) - (void*)attr; 
	blob_attr_set_raw_len(attr, len);
}

blob_offset_t blob_buf_open_table(struct blob_buf *buf, const char *name){
	struct blob_attr *attr = blob_buf_new_attr(buf, BLOB_ATTR_TABLE, name, 0);
	return blob_buf_attr_to_offset(buf, attr);
}

void blob_buf_close_table(struct blob_buf *buf, blob_offset_t offset){
	struct blob_attr *attr = blob_buf_offset_to_attr(buf, offset);
	int len = (buf->buf + blob_attr_raw_len(blob_buf_head(buf))) - (void*)attr; 
	blob_attr_set_raw_len(attr, len);
}


struct blob_attr *blob_buf_put_float(struct blob_buf *buf, const char *name, float value){
	uint32_t val = cpu_to_be32(pack754_32(value));  
	return blob_buf_put(buf, BLOB_ATTR_FLOAT32, name, &val, sizeof(val)); 
}
 
struct blob_attr *blob_buf_put_double(struct blob_buf *buf, const char *name, double value){
	uint64_t val = cpu_to_be64(pack754_64(value));  
	return blob_buf_put(buf, BLOB_ATTR_FLOAT64, name, &val, sizeof(val));
} 

static void __attribute__((unused)) _blob_attr_dump(struct blob_buf *self, struct blob_attr *node, int indent){
	/*if(blob_attr_type(node) == BLOB_ATTR_ARRAY || blob_attr_type(node) == BLOB_ATTR_TABLE){
		printf(">>\n"); 
		_blob_attr_dump(self, attr, blob_attr_data(attr)); 
		printf("<<\n"); 
		return; 
	}*/

	static const char *names[] = {
		[BLOB_ATTR_ROOT] = "BLOB_ATTR_ROOT",
		//[BLOB_ATTR_NESTED] = "BLOB_ATTR_NESTED",
		[BLOB_ATTR_BINARY] = "BLOB_ATTR_BINARY",
		[BLOB_ATTR_STRING] = "BLOB_ATTR_STRING",
		[BLOB_ATTR_INT8] = "BLOB_ATTR_INT8",
		[BLOB_ATTR_INT16] = "BLOB_ATTR_INT16",
		[BLOB_ATTR_INT32] = "BLOB_ATTR_INT32",
		[BLOB_ATTR_INT64] = "BLOB_ATTR_INT64",
		[BLOB_ATTR_FLOAT32] = "BLOB_ATTR_FLOAT32", 
		[BLOB_ATTR_FLOAT64] = "BLOB_ATTR_FLOAT64",
		[BLOB_ATTR_ARRAY] = "BLOB_ATTR_ARRAY", 
		[BLOB_ATTR_TABLE] = "BLOB_ATTR_TABLE"
	}; 

	for(struct blob_attr *attr = blob_attr_first_child(node); attr; attr = blob_attr_next_child(node, attr)){
		char *data = (char*)attr; //blob_attr_data(attr); 

		int id = blob_attr_type(attr);
		int len = blob_attr_pad_len(attr);

		

		int offset = (node)?((int)((char*)attr - (char*)node)):0; 
		for(int c = 0; c < indent; c++) printf("\t"); 
		printf("[ field [%s] (", blob_attr_name(attr)); 
		for(int c = 0; c < sizeof(struct blob_attr); c++){
			printf("%02x", (int)*((char*)attr + c) & 0xff); 
		}
		printf(") type=%s offset=%d padlen: %d, rawlen: %d ]\n", names[(id < BLOB_ATTR_LAST)?id:0], offset, len, blob_attr_raw_len(attr)); 

		if(id == BLOB_ATTR_ARRAY || id == BLOB_ATTR_TABLE) {
			_blob_attr_dump(self, attr, indent+1); 
			continue; 
		}

		printf("\t"); 
		for(int c = 0; c < blob_attr_pad_len(attr); c++){
			if(c > 0 && c % 10 == 0)
				printf("\n\t"); 
			printf(" %02x(%c)", data[c] & 0xff, (data[c] > 0x10 && data[c] < 128)?data[c]:'.'); 
		}
		printf("\n"); 
	}
}

void blob_buf_dump(struct blob_buf *self){
	if(!self) return; 

	printf("=========== blob ===========\n"); 
	printf("size: %lu, memory: %lu\n", blob_buf_size(self), self->memlen);
	_blob_attr_dump(self, blob_buf_head(self), 0); 
	printf("============================\n"); 
}
