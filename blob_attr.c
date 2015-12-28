#include "blob.h"
#include "blob_attr.h"


static const int blob_type_minlen[BLOB_ATTR_LAST] = {
	[BLOB_ATTR_STRING] = 1,
	[BLOB_ATTR_INT8] = sizeof(uint8_t),
	[BLOB_ATTR_INT16] = sizeof(uint16_t),
	[BLOB_ATTR_INT32] = sizeof(uint32_t),
	[BLOB_ATTR_INT64] = sizeof(uint64_t),
};

/*const char *blob_attr_name(struct blob_attr *attr){
	if(!blob_attr_has_name(attr)) return ""; 
	struct blob_name_hdr *hdr = (struct blob_name_hdr*)attr->data; 
	return (char*)hdr->name; 
}*/

bool blob_attr_check_type(const void *ptr, unsigned int len, int type){
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

//! returns the data of the attribute
void *blob_attr_data(const struct blob_attr *attr){
	if(!attr) return NULL; 

	if (blob_attr_has_name(attr)){
		struct blob_name_hdr *hdr = (struct blob_name_hdr *) attr->data;
		return (char*)attr->data + blob_name_hdrlen(be16_to_cpu(hdr->namelen));
	}

	return (void *) attr->data;
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

float blob_attr_get_float(const struct blob_attr *attr){
	if(!attr) return 0; 
	return unpack754_32(blob_attr_get_u32(attr)); 
}

double blob_attr_get_double(const struct blob_attr *attr){
	if(!attr) return 0; 
	return unpack754_64(blob_attr_get_u64(attr)); 
}

void blob_attr_fill_pad(struct blob_attr *attr) {
	char *buf = (char *) attr;
	int len = blob_attr_pad_len(attr);
	int delta = len - blob_attr_raw_len(attr);

	if (delta > 0)
		memset(buf + len - delta, 0, delta);
}

void
blob_attr_set_raw_len(struct blob_attr *attr, unsigned int len){
	if(len < sizeof(struct blob_attr)) len = sizeof(struct blob_attr); 	
	len &= BLOB_ATTR_LEN_MASK;
	attr->id_len &= ~cpu_to_be32(BLOB_ATTR_LEN_MASK);
	attr->id_len |= cpu_to_be32(len);
}

bool _blob_attr_validate(struct blob_attr *attr, const char *signature, const char **nk){
	const char *k = signature; 
	//printf("validating %s\n", signature); 
	struct blob_attr *field = blob_attr_first_child(attr); 
	if(!field) return false; // correctly handle empty message!
	while(*k && field){
		//printf("KEY: %c\n", *k); 
		switch(*k){
			case '{': 
				if(blob_attr_type(field) != BLOB_ATTR_TABLE) return false; 
				if(!_blob_attr_validate(field, k + 1, &k)) return false; 
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
				if(blob_attr_type(field) != BLOB_ATTR_ARRAY) return false;  
				if(!_blob_attr_validate(field, k + 1, &k)) return false; 
				break;
			case ']': 
				//printf("closing array\n"); 
				if(!field) return true; 
				k = signature; 
				continue; 
				break; 
			case 'i': 
				if(blob_attr_type(field) != BLOB_ATTR_INT32 && blob_attr_type(field) != BLOB_ATTR_INT8) return false; 
				break; 
			case 's': 
				if(blob_attr_type(field) != BLOB_ATTR_STRING) return false; 
				break; 
			case 't': 
				if(blob_attr_type(field) != BLOB_ATTR_TABLE) return false; 
				break; 
			case 'a': 
				if(blob_attr_type(field) != BLOB_ATTR_ARRAY) return false; 
				break; 
			case 'v': 
				break; 
		}
		k++; 
		field = blob_attr_next_child(attr, field); 
		//printf("next field %s %d\n", k, blob_attr_type(field)); 
	}
	//printf("exiting at %s\n", k); 
	if(nk) *nk = k; 
	return true; 
}

bool blob_attr_validate(struct blob_attr *attr, const char *signature){
	return _blob_attr_validate(attr, signature, NULL); 
}

bool blob_attr_parse(struct blob_attr *attr, const char *signature, struct blob_attr **out, int out_size){
	memset(out, 0, sizeof(struct blob_attr*) * out_size); 
	if(!blob_attr_validate(attr, signature)) return false; 
	for(struct blob_attr *a = blob_attr_first_child(attr); a && out_size; a = blob_attr_next_child(attr, a)){
		*out = a; 
		out++; 
		out_size--; 
	}
	return true; 
}
