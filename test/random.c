#include "blobpack.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

enum {
	FIELD_STRING, 
	FIELD_INT32, 
	FIELD_MAX
} ; 
/*
static struct blob_field_policy policy[FIELD_MAX] = {
	[FIELD_STRING] = { .name = "string", .type = BLOB_ATTR_STRING }, 
	[FIELD_INT32] = { .name = "int32", .type = BLOB_ATTR_INT32 }
}; 
*/
int main(int argc, char **argv){
	struct blob buf; 
	char data[256]; 

	srand(time(0)); 

	for(int c = 0; c < 256; c++){
		data[c] = rand(); 
	}

	//buf.buf = data; 
	//buf.head = (struct blob_field*)data; 

	struct blob b; 
	blob_init(&b, 0, 0); 

	blob_put_string(&b, "foo"); 
	blob_put_int(&b, 123); 

	printf("raw len: %u\n", (uint32_t)blob_size(&b)); 
	blob_init(&buf, data, 256); 
	struct blob_field *out[FIELD_MAX] = {0}; 
/*
	if(-1 == blob_parse(&buf, policy, FIELD_MAX, out)){
		printf("Error parsing blob!\n"); 
	}
*/	
	if(!out[FIELD_STRING]) printf("OK! no string field!\n"); 
	if(!out[FIELD_INT32]) printf("OK! no int field!\n"); 

/*
	printf("Data: \n"); 
	printf("string: %s\n", (char*)blob_field_data(out[FIELD_STRING])); 
	printf("int32: %lli\n", blob_field_get_int(out[FIELD_INT32])); 
*/

	return 0; 
}
