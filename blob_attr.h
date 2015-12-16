#pragma once

#include <stdint.h>
#include "utils.h"

#define BLOB_ATTR_ID_MASK  0x7f000000
#define BLOB_ATTR_ID_SHIFT 24
#define BLOB_ATTR_LEN_MASK 0x00ffffff
#define BLOB_ATTR_ALIGN    4
#define BLOB_ATTR_HAS_NAME 0x80000000

// a blob attribute header 
struct blob_attr {
	uint32_t id_len;
	char data[];
} __packed;

struct blob_attr_policy {
	const char *name;
	int type;
};

struct blob_attr_info {
	unsigned int type;
	unsigned int minlen;
	unsigned int maxlen;
	int (*validate)(const struct blob_attr_info *, struct blob_attr *);
};

struct blob_name_hdr {
	uint16_t namelen;
	uint8_t name[];
} __packed;

#define BLOB_NAME_ALIGN	2
#define BLOB_NAME_PADDING(len) (((len) + (1 << BLOB_NAME_ALIGN) - 1) & ~((1 << BLOB_NAME_ALIGN) - 1))

static inline int blob_name_hdrlen(unsigned int namelen){
	return BLOB_NAME_PADDING(sizeof(struct blob_name_hdr) + namelen + 1);
}


//! returns the data of the attribute
void *blob_attr_data(const struct blob_attr *attr); 

//! returns the type of the attribute 
static inline unsigned int blob_attr_type(const struct blob_attr *attr){
	if(!attr) return -1; 
	int id = (be32_to_cpu(attr->id_len) & BLOB_ATTR_ID_MASK) >> BLOB_ATTR_ID_SHIFT;
	return id;
}

static inline bool
blob_attr_has_name(const struct blob_attr *attr){
	if(!attr) return false; 
	return !!(attr->id_len & cpu_to_be32(BLOB_ATTR_HAS_NAME));
}

//const char *blob_attr_name(struct blob_attr *attr); 

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
blob_attr_get_i8(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_get_u8(attr);
}

static inline int16_t
blob_attr_get_i16(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_get_u16(attr);
}

static inline int32_t
blob_attr_get_i32(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_get_u32(attr);
}

static inline int64_t
blob_attr_get_i64(const struct blob_attr *attr){
	if(!attr) return 0; 
	return blob_attr_get_u64(attr);
}

static inline const char *
blob_attr_get_string(const struct blob_attr *attr){
	if(!attr) return "(nil)"; 
	return attr->data;
}

static inline void 
blob_attr_get_raw(const struct blob_attr *attr, uint8_t *data, size_t data_size){
	if(!attr) return; 
	memcpy(data, attr->data, data_size); 
}

float blob_attr_get_float(const struct blob_attr *attr); 
double blob_attr_get_double(const struct blob_attr *attr); 

extern void blob_attr_fill_pad(struct blob_attr *attr);
extern void blob_attr_set_raw_len(struct blob_attr *attr, unsigned int len);
extern bool blob_attr_equal(const struct blob_attr *a1, const struct blob_attr *a2);
extern int blob_attr_parse(struct blob_attr *attr, struct blob_attr **data, const struct blob_attr_policy *info, int max);
extern struct blob_attr *blob_attr_memdup(struct blob_attr *attr);
extern bool blob_attr_check_type(const void *ptr, unsigned int len, int type);

static inline struct blob_attr *blob_attr_first_child(const struct blob_attr *self){
	assert(self); 
	if(blob_attr_raw_len(self) <= sizeof(struct blob_attr)) return NULL; 
	return (struct blob_attr*)blob_attr_data(self); 
}

static inline struct blob_attr *blob_attr_next_child(const struct blob_attr *self, const struct blob_attr *child){
	if(!child) return NULL;
	struct blob_attr *ret = (struct blob_attr *) ((char *) child + blob_attr_pad_len(child));
	// check if we are still within bounds
	size_t offset = (char*)ret - (char*)self; 
	if(offset >= blob_attr_pad_len(self)) return NULL; 
	return ret; 
}

void blob_attr_dump(struct blob_attr *self); 
