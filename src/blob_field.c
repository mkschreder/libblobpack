#include "blob.h"
#include "blob_field.h"


static const int blob_type_minlen[BLOB_FIELD_LAST] = {
	[BLOB_FIELD_STRING] = 1,
	[BLOB_FIELD_INT8] = sizeof(uint8_t),
	[BLOB_FIELD_INT16] = sizeof(uint16_t),
	[BLOB_FIELD_INT32] = sizeof(uint32_t),
	[BLOB_FIELD_INT64] = sizeof(uint64_t),
};

/*const char *blob_field_name(struct blob_field *attr){
	if(!blob_field_has_name(attr)) return ""; 
	struct blob_name_hdr *hdr = (struct blob_name_hdr*)attr->data; 
	return (char*)hdr->name; 
}*/
void blob_field_fill_pad(struct blob_field *attr) {
	if(!attr) return; 
	char *buf = (char *) attr;
	int len = blob_field_raw_pad_len(attr);
	int delta = len - blob_field_raw_len(attr);

	if (delta > 0)
		memset(buf + len - delta, 0, delta);
}

void blob_field_set_raw_len(struct blob_field *attr, uint32_t len){
	if(!attr) return; 
	if(len < sizeof(struct blob_field)) len = sizeof(struct blob_field);
	len &= BLOB_FIELD_LEN_MASK;
	attr->id_len &= ~cpu_to_be32(BLOB_FIELD_LEN_MASK);
	attr->id_len |= cpu_to_be32(len);
}
/*
bool blob_field_check_type(const void *ptr, unsigned int len, int type){
	const char *data = ptr;

	if (type >= BLOB_FIELD_LAST)
		return false;

	if (type >= BLOB_FIELD_INT8 && type <= BLOB_FIELD_INT64) {
		if (len != blob_type_minlen[type])
			return false;
	} else {
		if (len < blob_type_minlen[type])
			return false;
	}

	if (type == BLOB_FIELD_STRING && data[len - 1] != 0)
		return false;

	return true;
}
*/

bool
blob_field_equal(const struct blob_field *a1, const struct blob_field *a2){
	if((!a1 || !a2) && (a1 != a2)) return false; 
	if((!a1 || !a2) && (a1 == a2)) return true; 

	if (!a1 && !a2)
		return true;

	if (!a1 || !a2)
		return false;

	if (blob_field_raw_pad_len(a1) != blob_field_raw_pad_len(a2))
		return false;

	return !memcmp(a1, a2, blob_field_raw_pad_len(a1));
}

//! returns the data of the attribute
const void *blob_field_data(const struct blob_field *attr){
	if(!attr) return NULL; 
	return (const void *) attr->data;
}

/*
struct blob_field *blob_field_copy(struct blob_field *attr){
	if(!attr) return NULL; 
	struct blob_field *ret;
	int size = blob_field_raw_pad_len(attr);

	ret = malloc(size);
	if (!ret)
		return NULL;

	memcpy(ret, attr, size);
	return ret;
}
*/

static uint8_t blob_field_get_u8(const struct blob_field *attr){
	assert(attr); 
	return *((const uint8_t *) attr->data);
}
/*
void blob_field_set_u8(const struct blob_field *attr, uint8_t val){
	if(!attr) return; 
	*((uint8_t *) attr->data) = val;
}
*/
static uint16_t blob_field_get_u16(const struct blob_field *attr){
	assert(attr);
	const uint16_t *tmp = (const uint16_t*)(const void*)attr->data;
	return be16_to_cpu(*tmp);
}
/*
void
blob_field_set_u16(const struct blob_field *attr, uint16_t val){
	if(!attr) return; 
	uint16_t *tmp = (uint16_t*)attr->data;
	*tmp = cpu_to_be16(val); 
}
*/
static uint32_t blob_field_get_u32(const struct blob_field *attr){
	assert(attr); 
	const uint32_t *tmp = (const uint32_t*)(const void*)attr->data;
	return be32_to_cpu(*tmp);
}
/*
void
blob_field_set_u32(const struct blob_field *attr, uint32_t val){
	if(!attr) return; 
	uint32_t *tmp = (uint32_t*)attr->data;
	*tmp = cpu_to_be32(val); 
}
*/
static uint64_t blob_field_get_u64(const struct blob_field *attr){
	assert(attr); 
	const uint32_t *ptr = (const uint32_t *) blob_field_data(attr);
	uint64_t tmp = ((uint64_t) be32_to_cpu(ptr[0])) << 32;
	tmp |= be32_to_cpu(ptr[1]);
	return tmp;
}

static int8_t blob_field_get_i8(const struct blob_field *attr){
	assert(attr); 
	return blob_field_get_u8(attr);
}
/*
void
blob_field_set_i8(const struct blob_field *attr, int8_t val){
	if(!attr) return; 
	blob_field_set_u8(attr, val);
}
*/
static int16_t blob_field_get_i16(const struct blob_field *attr){
	assert(attr); 
	return blob_field_get_u16(attr);
}
/*
void
blob_field_set_i16(const struct blob_field *attr, int16_t val){
	if(!attr) return; 
	blob_field_set_u16(attr, val);
}
*/
static int32_t blob_field_get_i32(const struct blob_field *attr){
	assert(attr); 
	return blob_field_get_u32(attr);
}
/*
void
blob_field_set_i32(const struct blob_field *attr, int32_t val){
	if(!attr) return;
	blob_field_set_u32(attr, val);
}
*/
static int64_t blob_field_get_i64(const struct blob_field *attr){
	assert(attr); 
	return blob_field_get_u64(attr);
}

static float blob_field_get_f32(const struct blob_field *attr){
	assert(attr); 
	return unpack754_32(blob_field_get_u32(attr)); 
}

static double blob_field_get_f64(const struct blob_field *attr){
	assert(attr); 
	return unpack754_64(blob_field_get_u64(attr)); 
}


long long blob_field_get_int(const struct blob_field *self){
	assert(self);  
	int type = blob_field_type(self); 
	switch(type){
		case BLOB_FIELD_INT8: return blob_field_get_i8(self); 
		case BLOB_FIELD_INT16: return blob_field_get_i16(self); 
		case BLOB_FIELD_INT32: return blob_field_get_i32(self); 
		case BLOB_FIELD_INT64: return blob_field_get_i64(self); 
		case BLOB_FIELD_FLOAT32: return blob_field_get_f32(self); 
		case BLOB_FIELD_FLOAT64: return blob_field_get_f64(self); 
		case BLOB_FIELD_STRING: {
			long long val; 
			sscanf(self->data, "%lli", &val); 
			return val; 
		} 
	}
	return 0; 
}

double blob_field_get_real(const struct blob_field *self){
	assert(self); 
	int type = blob_field_type(self); 
	switch(type){
		case BLOB_FIELD_INT8: return blob_field_get_i8(self); 
		case BLOB_FIELD_INT16: return blob_field_get_i16(self); 
		case BLOB_FIELD_INT32: return blob_field_get_i32(self); 
		case BLOB_FIELD_INT64: return blob_field_get_i64(self); 
		case BLOB_FIELD_FLOAT32: return blob_field_get_f32(self); 
		case BLOB_FIELD_FLOAT64: return blob_field_get_f64(self); 
		case BLOB_FIELD_STRING: {
			double val; 
			sscanf(self->data, "%lf", &val); 
			return val; 
		} 
	}
	return 0; 
}

bool blob_field_get_bool(const struct blob_field *self){
	assert(self); 
	return !!blob_field_get_int(self); 
}

const char *
blob_field_get_string(const struct blob_field *attr){
	if(!attr) return NULL; 
	return attr->data;
}

/*
size_t blob_field_get_raw(const struct blob_field *attr, uint8_t *data, size_t data_size){
	assert(attr); 
	size_t s = blob_field_size(attr); 
	if(data_size > s) data_size = s; 
	memcpy(data, attr->data, data_size); 
	return data_size; 
}
*/

//! returns the type of the attribute 
uint8_t blob_field_type(const struct blob_field *attr){
	if(!attr) return BLOB_FIELD_INVALID; 
	int id = (be32_to_cpu(attr->id_len) & BLOB_FIELD_ID_MASK) >> BLOB_FIELD_ID_SHIFT;
	return id;
}
/*
void blob_field_set_type(struct blob_field *self, int type){
	assert(self); 
	int id_len = be32_to_cpu(self->id_len); 
	id_len = (id_len & ~BLOB_FIELD_ID_MASK) | (type << BLOB_FIELD_ID_SHIFT); 	
	self->id_len = cpu_to_be32(id_len);
}
*/
//! returns full length of attribute
unsigned int
blob_field_raw_len(const struct blob_field *attr){
	assert(attr); 
	return (be32_to_cpu(attr->id_len) & BLOB_FIELD_LEN_MASK); 
}

//! includes length of data of the attribute
unsigned int
blob_field_data_len(const struct blob_field *attr){
	assert(attr); 
	return blob_field_raw_len(attr) - sizeof(struct blob_field);
}

//! returns padded length of full attribute
uint32_t 
blob_field_raw_pad_len(const struct blob_field *attr){
	assert(attr); 
	uint32_t len = blob_field_raw_len(attr);
	len = (len + BLOB_FIELD_ALIGN - 1) & ~(BLOB_FIELD_ALIGN - 1);
	return len;
}

const struct blob_field *blob_field_first_child(const struct blob_field *self){
	if(!self) return NULL; 
	if(blob_field_raw_len(self) <= sizeof(struct blob_field)) return NULL; 
	return (const struct blob_field*)blob_field_data(self); 
}

const struct blob_field *blob_field_next_child(const struct blob_field *self, const struct blob_field *child){
	if(!child) return NULL;
	const struct blob_field *ret = (const struct blob_field *) ((const char *) child + blob_field_raw_pad_len(child));
	// check if we are still within bounds
	size_t offset = (const char*)ret - (const char*)self; 
	if(offset >= blob_field_raw_pad_len(self)) return NULL; 
	return ret; 
}
static bool _blob_field_validate(const struct blob_field *attr, const char *signature, const char **nk){
	const char *k = signature; 
	//printf("validating %s\n", signature); 
	const struct blob_field *field = blob_field_first_child(attr); 
	if(!field) return false; // correctly handle empty message!
	while(*k && field){
		//printf("KEY: %c\n", *k); 
		switch(*k){
			case '{': 
				if(blob_field_type(field) != BLOB_FIELD_TABLE) return false; 
				if(!_blob_field_validate(field, k + 1, &k)) return false; 
				//printf("continuing at %s\n", k); 
				break; 
			case '}': 
				//printf("closing\n"); 
				if(!field) {
					printf("exiting level\n"); 
					if(nk) *nk = k ; 
					return true; 
				}
				k = signature; 
				continue; 
				break; 
			case '[': 
				//printf("parsing array!\n"); 
				if(blob_field_type(field) != BLOB_FIELD_ARRAY) return false;  
				if(!_blob_field_validate(field, k + 1, &k)) return false; 
				break;
			case ']': 
				//printf("closing array\n"); 
				if(!field) return true; 
				k = signature; 
				continue; 
				break; 
			case 'i': 
				switch(blob_field_type(field)){
					case BLOB_FIELD_INT8: 
					case BLOB_FIELD_INT16: 
					case BLOB_FIELD_INT32: 
					case BLOB_FIELD_INT64: 
						break; 
					default: 
						return false; 
				}
				break; 
			case 'f': 
				switch(blob_field_type(field)){
					case BLOB_FIELD_FLOAT32: 
					case BLOB_FIELD_FLOAT64: 
						break; 
					default: 
						return false; 
				}
				break; 
			case 's': 
				if(blob_field_type(field) != BLOB_FIELD_STRING) return false; 
				break; 
			case 't': 
				if(blob_field_type(field) != BLOB_FIELD_TABLE) return false; 
				break; 
			case 'a': 
				if(blob_field_type(field) != BLOB_FIELD_ARRAY) return false; 
				break; 
			case 'v': 
				break; 
		}
		k++; 
		field = blob_field_next_child(attr, field); 
		//printf("next field %s %d\n", k, blob_field_type(field)); 
	}
	//printf("exiting at %s\n", k); 
	if(nk) *nk = k; 
	return true; 
}

bool blob_field_validate(const struct blob_field *attr, const char *signature){
	if(!attr) return false; 
	return _blob_field_validate(attr, signature, NULL); 
}

bool blob_field_parse(const struct blob_field *attr, const char *signature, const struct blob_field **out, int out_size){
	if(!attr) return false;  
	memset(out, 0, sizeof(struct blob_field*) * out_size); 
	if(!blob_field_validate(attr, signature)) return false; 
	for(const struct blob_field *a = blob_field_first_child(attr); a && out_size; a = blob_field_next_child(attr, a)){
		*out = a; 
		out++; 
		out_size--; 
	}
	return true; 
}

bool blob_field_parse_values(const struct blob_field *attr, struct blob_policy *policy, int policy_size){
	if(!attr) return false; 
	bool valid = true; 
	if(blob_field_type(attr) == BLOB_FIELD_TABLE){
		const struct blob_field *key, *value; 
		int processed = 0; 
		blob_field_for_each_kv(attr, key, value){
			if(processed == policy_size) return false; 
			for(int c = 0; c < policy_size; c++){
				if(strcmp(blob_field_get_string(key), policy[c].name) == 0){
					if(policy[c].type == BLOB_FIELD_ANY || blob_field_type(value) == policy[c].type) {
						policy[c].value = value; 
					} else { policy[c].value = NULL; valid = false; }
					processed++; 
					break; 
				}
			}
		}
	} else if(blob_field_type(attr) == BLOB_FIELD_ARRAY){
		const struct blob_field *child; 
		int pidx = 0; 
		blob_field_for_each_child(attr, child){
			if(!policy_size) return false; 
			if(policy[pidx].type == BLOB_FIELD_ANY || blob_field_type(child) == policy[pidx].type) {
				policy[pidx].value = child; 
			} else { policy[pidx].value = NULL; valid = false; }
			pidx++; 
			policy_size--; 
		}
	}
	return valid; 
}
