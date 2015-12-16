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

int blob_attr_parse(struct blob_attr *attr, struct blob_attr **data, const struct blob_attr_policy *info, int max){
	int found = 0;

	memset(data, 0, sizeof(struct blob_attr *) * max);
	//blob_buf_for_each_attr(pos, attr, rem) {
	for(struct blob_attr *pos = blob_attr_first_child(attr); pos; pos = blob_attr_next_child(attr, pos)){ 
		int id = blob_attr_type(pos);
		int len = blob_attr_len(pos);

		if (id >= max)
			continue;

		if (info) {
			int type = info[id].type;

			if (type < BLOB_ATTR_LAST) {
				if (!blob_attr_check_type(blob_attr_data(pos), len, type))
					continue;
			}
/*
			if (info[id].minlen && len < info[id].minlen)
				continue;

			if (info[id].maxlen && len > info[id].maxlen)
				continue;

			if (info[id].validate && !info[id].validate(&info[id], pos))
				continue;*/
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


