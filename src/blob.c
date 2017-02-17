/*
	Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 	Copyright (C) 2015 Martin Schröder <mkschreder.uk@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <endian.h>
#include "blob.h"
#include "ieee754.h"

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

static inline struct blob_field *blob_offset_to_attr(struct blob *buf, blob_offset_t offset){
	void *ptr = (char *)buf->buf + (size_t)offset;
	return ptr;
}

static inline blob_offset_t blob_field_to_offset(struct blob *buf, struct blob_field *attr){
	return (blob_offset_t)((char *)attr - (char *) buf->buf);
}

static void blob_field_init(struct blob_field *attr, uint32_t id, uint32_t len){
	assert(attr); 
	memset(attr, 0, sizeof(struct blob_field)); 
	//len += sizeof(struct blob_field); 
	len &= BLOB_FIELD_LEN_MASK;
	len |= (id << BLOB_FIELD_ID_SHIFT) & BLOB_FIELD_ID_MASK;
	attr->id_len = htobe32(len);
}

//! Attepts to reallocate the buffer to fit the new payload data
bool blob_resize(struct blob *buf, uint32_t minlen){
	assert(minlen > 0 && minlen < BLOB_MAX_SIZE); 

	char *new = 0;
	uint32_t newsize = ((minlen / 256) + 1) * 256;
	uint32_t cur_size = blob_size(buf); 
	// reallocate the memory of the buffer if we no longer have any memory left
	if(newsize > buf->memlen){
		new = realloc(buf->buf, newsize);
		if (new) {
			buf->buf = new;
			memset((char*)buf->buf + cur_size, 0, newsize - cur_size);
			buf->memlen = newsize;  
		} else { 
			return false; 
		}
	} 
	blob_field_set_raw_len(blob_head(buf), minlen);  
	blob_field_fill_pad(blob_head(buf)); 
	return true;
}

void blob_reset(struct blob *buf){
	assert(buf); 
	assert(buf->buf); 
	if(buf->memlen)
		memset(buf->buf, 0, buf->memlen); 

	blob_field_init(blob_head(buf), BLOB_FIELD_ARRAY, sizeof(struct blob_field)); 
}

void blob_init(struct blob *buf, const char *data, size_t size){
	memset(buf, 0, sizeof(struct blob)); 

	// default buffer is 256 bytes block with zero sized data
	buf->memlen = (size > 0)?size:256; 
	buf->buf = malloc(buf->memlen); 
	assert(buf->buf); 
	
	if(data) {
		memcpy(buf->buf, data, size); 
		//blob_field_init(blob_head(buf), BLOB_FIELD_ARRAY, sizeof(struct blob_field)); 
		//blob_field_fill_pad(blob_head(buf)); 
	} else {
		blob_reset(buf); 
	}
}

void blob_free(struct blob *buf){
	free(buf->buf);
	buf->buf = NULL;
	buf->memlen = 0;
}

static struct blob_field *blob_new_attr(struct blob *buf, int id, int payload){
	int attr_raw_len = sizeof(struct blob_field) + payload; 
	int attr_pad_len = attr_raw_len + (BLOB_FIELD_ALIGN - ( attr_raw_len % BLOB_FIELD_ALIGN)) % BLOB_FIELD_ALIGN; 

	struct blob_field *head = blob_head(buf); 
	int cur_len = blob_field_raw_pad_len(head); 
	int req_len = cur_len + attr_pad_len; 

	if (!blob_resize(buf, req_len))
		return NULL;

	// cast to void* to ignore alignment warning since this is already handled above
	struct blob_field *attr = (struct blob_field*)(void*)((char*)buf->buf + cur_len);
	
	blob_field_init(attr, id, attr_raw_len);
	blob_field_fill_pad(attr);

	// update the length of the head element to enclose the whole buffer
	blob_field_set_raw_len(blob_head(buf), req_len); 

	return attr;
}
/*
struct blob_field *blob_put_binary(struct blob *buf, const void *ptr, unsigned int len){
	assert(ptr); 
	struct blob_field *attr;

	if (len < sizeof(struct blob_field) || !ptr)
		return NULL;

	attr = blob_new_attr(buf, BLOB_FIELD_BINARY, len - sizeof(struct blob_field));
	if (!attr)
		return NULL;

	memcpy(attr->data, ptr, len);
	return attr;
}
*/
static struct blob_field *blob_put(struct blob *buf, int id, const void *ptr, unsigned int len){
	struct blob_field *attr;

	attr = blob_new_attr(buf, id, len);
	if (!attr) {
		return NULL;
	}

	if (ptr)
		memcpy(attr->data, ptr, len);

	return attr;
}

struct blob_field *blob_put_string(struct blob *buf, const char *str){
	assert(str); 
	return blob_put(buf, BLOB_FIELD_STRING, str, strlen(str) + 1);
}

static struct blob_field *blob_put_u8(struct blob *buf, uint8_t val){
	return blob_put(buf, BLOB_FIELD_INT8, &val, sizeof(val));
}

static struct blob_field *blob_put_u16(struct blob *buf, uint16_t val){
	val = htobe16(val);
	return blob_put(buf, BLOB_FIELD_INT16, &val, sizeof(val));
}

static struct blob_field *blob_put_u32(struct blob *buf, uint32_t val){
	val = htobe32(val);
	return blob_put(buf, BLOB_FIELD_INT32, &val, sizeof(val));
}

static struct blob_field *blob_put_u64(struct blob *buf, uint64_t val){
	val = htobe64(val);
	return blob_put(buf, BLOB_FIELD_INT64, &val, sizeof(val));
}

struct blob_field *blob_put_bool(struct blob *buf, bool val){
	return blob_put_u8(buf, val); 
}

struct blob_field *blob_put_int(struct blob *self, long long val){
	if(val >= INT8_MIN && val <= INT8_MAX) return blob_put_u8(self, val); 
	if(val >= INT16_MIN && val <= INT16_MAX) return blob_put_u16(self, val); 
	if(val >= INT32_MIN && val <= INT32_MAX) return blob_put_u32(self, val); 
	if(val >= INT64_MIN && val <= INT64_MAX) return blob_put_u64(self, val); 
	return NULL; 
}

blob_offset_t blob_open_array(struct blob *buf){
	struct blob_field *attr = blob_new_attr(buf, BLOB_FIELD_ARRAY, 0);
	return blob_field_to_offset(buf, attr);
}

void blob_close_array(struct blob *buf, blob_offset_t offset){
	if((long)offset > (long)blob_size(buf)) return; 
	struct blob_field *attr = blob_offset_to_attr(buf, offset);
	int len = ((char*)buf->buf + blob_field_raw_len(blob_head(buf))) - (char*)attr; 
	blob_field_set_raw_len(attr, len);
}

blob_offset_t blob_open_table(struct blob *buf){
	struct blob_field *attr = blob_new_attr(buf, BLOB_FIELD_TABLE, 0);
	return blob_field_to_offset(buf, attr);
}

void blob_close_table(struct blob *buf, blob_offset_t offset){
	if((long)offset > (long)blob_size(buf)) return; 
	struct blob_field *attr = blob_offset_to_attr(buf, offset);
	int len = ((char*)buf->buf + blob_field_raw_len(blob_head(buf))) - (char*)attr; 
	blob_field_set_raw_len(attr, len);
}

static struct blob_field *blob_put_float(struct blob *buf, double value){
	uint32_t val = htobe32(pack754_32((float)value));  
	return blob_put(buf, BLOB_FIELD_FLOAT32, &val, sizeof(val)); 
}
 
static struct blob_field *blob_put_double(struct blob *buf, double value){
	uint64_t val = htobe64(pack754_64(value));  
	return blob_put(buf, BLOB_FIELD_FLOAT64, &val, sizeof(val));
} 

struct blob_field *blob_put_real(struct blob *buf, double value){
	if((float)value == value) return blob_put_float(buf, value); 
	return blob_put_double(buf, value); 
}

struct blob_field *blob_put_attr(struct blob *buf, const struct blob_field *attr){
	if(!attr) return NULL; 
	
	size_t s =  blob_field_data_len(attr); 
	struct blob_field *f = blob_new_attr(buf, blob_field_type(attr), s); 
	memcpy(f, attr, blob_field_raw_pad_len(attr)); 
	return f; 
}

static void __attribute__((unused)) _blob_field_dump(const struct blob_field *node, uint32_t indent){
	static const char *names[] = {
		[BLOB_FIELD_INVALID] = "BLOB_FIELD_INVALID",
		[BLOB_FIELD_BINARY] = "BLOB_FIELD_BINARY",
		[BLOB_FIELD_STRING] = "BLOB_FIELD_STRING",
		[BLOB_FIELD_INT8] = "BLOB_FIELD_INT8",
		[BLOB_FIELD_INT16] = "BLOB_FIELD_INT16",
		[BLOB_FIELD_INT32] = "BLOB_FIELD_INT32",
		[BLOB_FIELD_INT64] = "BLOB_FIELD_INT64",
		[BLOB_FIELD_FLOAT32] = "BLOB_FIELD_FLOAT32", 
		[BLOB_FIELD_FLOAT64] = "BLOB_FIELD_FLOAT64",
		[BLOB_FIELD_ARRAY] = "BLOB_FIELD_ARRAY", 
		[BLOB_FIELD_TABLE] = "BLOB_FIELD_TABLE"
	}; 

	for(const struct blob_field *attr = blob_field_first_child(node); attr; attr = blob_field_next_child(node, attr)){
		const unsigned char *data = (const unsigned char*)attr; //blob_field_data(attr); 

		uint32_t id = blob_field_type(attr);
		uint32_t len = blob_field_raw_pad_len(attr);

		uint32_t offset = (node)?((uint32_t)((const char*)attr - (const char*)node)):0; 
		for(uint32_t c = 0; c < indent; c++) printf("\t"); 
		printf("[ field ("); 
		for(uint32_t c = 0; c < sizeof(struct blob_field); c++){
			printf("%02x", (int)*((const char*)attr + c) & 0xff); 
		}
		printf(") type=%s offset=%d full padded len: %d, header+data: %d, data len: %d ]\n", names[(id < BLOB_FIELD_LAST)?id:0], (int)offset, (int)len, (int)blob_field_raw_len(attr), blob_field_data_len(attr)); 

		if(id == BLOB_FIELD_ARRAY || id == BLOB_FIELD_TABLE) {
			_blob_field_dump(attr, indent+1); 
			continue; 
		}

		printf("\t"); 
		for(uint32_t c = 0; c < blob_field_raw_pad_len(attr); c++){
			if(c > 0 && c % 10 == 0)
				printf("\n\t"); 
			printf(" %02x(%c)", data[c] & 0xff, (data[c] > 0x10 && data[c] < 128)?data[c]:'.'); 
		}
		printf("\n"); 
	}
}

void blob_field_dump(const struct blob_field *self){
	_blob_field_dump(self, 0); 
}

void blob_dump(struct blob *self){
	if(!self) return; 

	printf("=========== blob ===========\n"); 
	printf("size: %d, memory: %d\n", (int)blob_size(self), (int)self->memlen);
	for(int c = 0; c < 8; c++) printf("%02x ", ((char*)self->buf)[c] & 0xff); 
	printf("\n"); 
	blob_field_dump(blob_head(self)); 
	printf("============================\n"); 
}
