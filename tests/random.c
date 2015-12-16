#include "blobpack.h"

enum {
	FIELD_STRING, 
	FIELD_INT32, 
	FIELD_MAX
} ; 
/*
static struct blob_attr_policy policy[FIELD_MAX] = {
	[FIELD_STRING] = { .name = "string", .type = BLOB_ATTR_STRING }, 
	[FIELD_INT32] = { .name = "int32", .type = BLOB_ATTR_INT32 }
}; 
*/
int main(int argc, char **argv){
	struct blob_buf buf; 
	char data[256]; 

	srand(time(0)); 

	for(int c = 0; c < 256; c++){
		data[c] = rand(); 
	}

	//buf.buf = data; 
	//buf.head = (struct blob_attr*)data; 

	struct blob_buf b; 
	blob_buf_init(&b, 0, 0); 

	blob_buf_put_string(&b, "foo"); 
	blob_buf_put_u32(&b, 123); 

	printf("raw len: %u\n", (uint32_t)blob_buf_size(&b)); 
	blob_buf_init(&buf, data, 256); 
	struct blob_attr *out[FIELD_MAX]; 
/*
	if(-1 == blob_buf_parse(&buf, policy, FIELD_MAX, out)){
		printf("Error parsing blob!\n"); 
	}
*/	
	if(!out[FIELD_STRING]) printf("OK! no string field!\n"); 
	if(!out[FIELD_INT32]) printf("OK! no int field!\n"); 

	printf("Data: \n"); 
	printf("string: %s\n", (char*)blob_attr_data(out[FIELD_STRING])); 
	printf("int32: %d\n", blob_attr_get_u32(out[FIELD_INT32])); 

	return 0; 
}
