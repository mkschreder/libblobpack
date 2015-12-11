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
#include "blobmsg.h"

static const int blob_type[__BLOBMSG_TYPE_LAST] = {
	[BLOBMSG_TYPE_INT8] = BLOB_ATTR_INT8,
	[BLOBMSG_TYPE_INT16] = BLOB_ATTR_INT16,
	[BLOBMSG_TYPE_INT32] = BLOB_ATTR_INT32,
	[BLOBMSG_TYPE_INT64] = BLOB_ATTR_INT64,
	[BLOBMSG_TYPE_STRING] = BLOB_ATTR_STRING,
	[BLOBMSG_TYPE_UNSPEC] = BLOB_ATTR_BINARY,
	[BLOBMSG_TYPE_FLOAT32] = BLOB_ATTR_FLOAT32, 
	[BLOBMSG_TYPE_FLOAT64] = BLOB_ATTR_FLOAT64
};

static uint16_t
blobmsg_namelen(const struct blobmsg_hdr *hdr)
{
	return be16_to_cpu(hdr->namelen);
}

bool blobmsg_check_attr(const struct blob_attr *attr, bool name)
{
	const struct blobmsg_hdr *hdr;
	const char *data;
	int id, len;

	if (blob_attr_len(attr) < sizeof(struct blobmsg_hdr))
		return false;

	hdr = (void *) attr->data;
	if (!hdr->namelen && name)
		return false;

	if (blobmsg_namelen(hdr) > blob_attr_len(attr) - sizeof(struct blobmsg_hdr))
		return false;

	if (hdr->name[blobmsg_namelen(hdr)] != 0)
		return false;

	id = blob_attr_id(attr);
	len = blobmsg_data_len(attr);
	data = blobmsg_data(attr);

	if (id > BLOBMSG_TYPE_LAST)
		return false;

	if (!blob_type[id])
		return true;

	return blob_attr_check_type(data, len, blob_type[id]);
}

int blobmsg_check_array(const struct blob_attr *attr, int type)
{
	struct blob_attr *cur;
	bool name;
	//int rem;
	int size = 0;

	switch (blobmsg_type(attr)) {
	case BLOBMSG_TYPE_TABLE:
		name = true;
		break;
	case BLOBMSG_TYPE_ARRAY:
		name = false;
		break;
	default:
		return -1;
	}

	//blobmsg_for_each_attr(cur, attr, rem) {
	for(cur = blobmsg_data(attr); cur; cur = blob_attr_next(attr, cur)){
		if (type != BLOBMSG_TYPE_UNSPEC && blobmsg_type(cur) != type)
			return -1;

		if (!blobmsg_check_attr(cur, name))
			return -1;

		size++;
	}

	return size;
}

bool blobmsg_check_attr_list(const struct blob_attr *attr, int type)
{
	return blobmsg_check_array(attr, type) >= 0;
}

int blobmsg_parse_array(struct blob_attr *attr, const struct blobmsg_policy *policy, int policy_len, struct blob_attr **tb){
	int i = 0;

	memset(tb, 0, policy_len * sizeof(*tb));
	//__blob_buf_for_each_attr(attr, data, len) {
	for(struct blob_attr *pos = blobmsg_data(attr); pos; pos = blob_attr_next(attr, pos)){
		if (policy[i].type != BLOBMSG_TYPE_UNSPEC &&
		    blob_attr_id(pos) != policy[i].type)
			continue;

		if (!blobmsg_check_attr(pos, false))
			return -1;

		if (tb[i])
			continue;

		tb[i++] = pos;
		if (i == policy_len)
			break;
	}

	return 0;
}

static void _blobmsg_attr_dump(struct blob_buf *self, struct blob_attr *owner, struct blob_attr *attr){
	int id = blobmsg_type(attr); 
	char *data = (char*)attr; 
	int len = blob_attr_pad_len(attr);

	printf("%s: id=%d offset=%d\n", blobmsg_name(attr), id, (int)((char*)attr - (char*)owner)); 
	printf("\tlength: %d\n", len); 
	printf("\tpadlen: %d\n", blob_attr_pad_len(attr));
	
	// dump memory
	for(int c = 0; c < blob_attr_raw_len(attr); c++){
		if(c > 0 && c % 10 == 0)
			printf("\n"); 
		printf(" %02x(%c)", data[c] & 0xff, (data[c]>50 && data[c]<127)?data[c]:'.'); 
	}
	printf("\n"); 

	// if nested, then go through all children
	if(id == BLOBMSG_TYPE_ARRAY || id == BLOBMSG_TYPE_TABLE){
		printf(">>\n"); 

		for(struct blob_attr *child = blobmsg_data(attr); child; child = blob_attr_next(attr, child)){
			printf("child:\n"); 
			_blobmsg_attr_dump(self, attr, child); 
		}

		printf("<<\n"); 
	}
}

void blobmsg_dump(struct blob_buf *self){
	if(!self) return; 

	printf("=========== blob ===========\n"); 
	_blobmsg_attr_dump(self, NULL, blob_buf_first(self)); 
	printf("============================\n"); 
}

int blobmsg_parse(struct blob_buf *buf, const struct blobmsg_policy *policy, int policy_len,
                  struct blob_attr **tb)
{
	//struct blobmsg_hdr *hdr;
	struct blob_attr *pos;
	uint8_t *pslen;
	int i;

	memset(tb, 0, policy_len * sizeof(*tb));
	pslen = alloca(policy_len);
	for (i = 0; i < policy_len; i++) {
		if (!policy[i].name)
			continue;

		pslen[i] = strlen(policy[i].name);
	}
	
	//__blob_buf_for_each_attr(pos, attr, len) {
	struct blob_attr *root = blob_buf_first(buf); 
	for(pos = blobmsg_data(root); pos; pos = blob_attr_next(root, pos)){
		//hdr = blob_attr_data(pos);
		for (i = 0; i < policy_len; i++) {
			if (!policy[i].name)
				continue;

			if (policy[i].type != BLOBMSG_TYPE_UNSPEC &&
			    blob_attr_id(pos) != policy[i].type)
				continue;

			if (strlen(blobmsg_name(pos)) != pslen[i])
					continue;

			if (!blobmsg_check_attr(pos, true))
				return -1;

			if (tb[i])
				continue;

			if (strcmp(policy[i].name, blobmsg_name(pos)) != 0)
				continue;

			tb[i] = pos;
		}
	}

	return 0;
}


static struct blob_attr *
blobmsg_new(struct blob_buf *buf, int type, const char *name, int payload_len){
	struct blob_attr *attr;
	struct blobmsg_hdr *hdr;
	int attrlen, namelen;
	char *pad_start, *pad_end;

	if (!name)
		name = "";

	namelen = strlen(name);
	attrlen = blobmsg_hdrlen(namelen) + payload_len;
	attr = blob_buf_new_attr(buf, type, attrlen);
	if (!attr)
		return NULL;

	attr->id_len |= be32_to_cpu(BLOB_ATTR_EXTENDED);
	hdr = blob_attr_data(attr);
	hdr->namelen = cpu_to_be16(namelen);
	strncpy((char *) hdr->name, (const char *)name, namelen);
	pad_end = blobmsg_data(attr);
	pad_start = (char *) &hdr->name[namelen];
	if (pad_start < pad_end)
		memset(pad_start, 0, pad_end - pad_start);

	return attr;
}

static inline int attr_to_offset(struct blob_buf *buf, struct blob_attr *attr){
	return (char *)attr - (char *) buf->buf;
}

void *blobmsg_open_nested(struct blob_buf *buf, const char *name, bool array){
	int type = array ? BLOBMSG_TYPE_ARRAY : BLOBMSG_TYPE_TABLE;
	if (!name)
		name = "";
	struct blob_attr *attr = blobmsg_new(buf, type, name, 0);
	return (void*)blob_buf_attr_to_offset(buf, attr);
}

void blobmsg_vprintf(struct blob_buf *buf, const char *name, const char *format, va_list arg){
	va_list arg2;
	char cbuf;
	int len;

	va_copy(arg2, arg);
	len = vsnprintf(&cbuf, sizeof(cbuf), format, arg2);
	va_end(arg2);
	
	struct blob_attr *attr = blobmsg_new(buf, BLOBMSG_TYPE_STRING, name, len + 1); 
	vsprintf(blobmsg_data(attr), format, arg);
}

void
blobmsg_printf(struct blob_buf *buf, const char *name, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	blobmsg_vprintf(buf, name, format, ap);
	va_end(ap);
}

struct blob_attr *blobmsg_alloc_string(struct blob_buf *buf, const char *name, unsigned int maxlen){
	struct blob_attr *attr;

	attr = blobmsg_new(buf, BLOBMSG_TYPE_STRING, name, maxlen);
	if (!attr)
		return NULL;

	return attr;
}
/*
void *
blobmsg_realloc_string_buffer(struct blob_buf *buf, unsigned int maxlen){
	struct blob_attr *attr = blob_attr_next(buf->head);
	int offset = attr_to_offset(buf, blob_attr_next(buf->head)) + blob_attr_pad_len(attr);
	int required = maxlen - (buf->datalen - offset);

	if (required <= 0)
		goto out;

	blob_buf_resize(buf, buf->datalen + required);
	attr = blob_attr_next(buf->head);

out:
	return blobmsg_data(attr);
}
void blobmsg_add_string_buffer(struct blob_buf *buf){
	struct blob_attr *attr;
	int len, attrlen;

	attr = blob_attr_next(buf->head);
	len = strlen(blobmsg_data(attr)) + 1;

	attrlen = blob_attr_raw_len(attr) + len;
	blob_attr_set_raw_len(attr, attrlen);
	blob_attr_fill_pad(attr);

	blob_attr_set_raw_len(buf->head, blob_attr_raw_len(buf->head) + blob_attr_pad_len(attr));
}
*/
int
blobmsg_add_field(struct blob_buf *buf, int type, const char *name,
                  const void *data, unsigned int len){
	struct blob_attr *attr;

	attr = blobmsg_new(buf, type, name, len);
	if (!attr)
		return -1;

	if (len > 0)
		memcpy(blobmsg_data(attr), data, len);

	return 0;
}
