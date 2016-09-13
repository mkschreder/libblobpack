#include "test-funcs.h"
#include <blobpack.h>
#include <stdbool.h>
#include <math.h>
#include <memory.h>

int main(void){
	struct blob blob; 
	blob_init(&blob, 0, 0); 

	blob_put_bool(&blob, true); 
	blob_put_string(&blob, "foo"); 
	blob_put_int(&blob, -13); 
	blob_put_real(&blob, M_PI); 
	
	struct blob_field *f = blob_put_string(&blob, "copyme"); 
	blob_put_attr(&blob, f); 

	blob_offset_t o = blob_open_table(&blob); 
	blob_put_string(&blob, "one"); 
	blob_put_int(&blob, 1); 
	blob_put_string(&blob, "two"); 
	blob_put_int(&blob, 2); 
	blob_put_string(&blob, "three"); 
	blob_put_int(&blob, 3); 
	blob_close_table(&blob, o); 

	o = blob_open_array(&blob); 
	blob_put_int(&blob, 100); 
	blob_put_int(&blob, 200); 
	blob_put_int(&blob, 1000); 
	blob_put_int(&blob, 70000); 
	blob_put_int(&blob, 5000000000lu); 
	blob_close_array(&blob, o); 

	char *json = blob_to_json(&blob); 
	printf("json: %s\n", json); 

	const struct blob_field *out[8]; 
	TEST(blob_field_parse(blob_head(&blob), "isifssta", out, 8)); 

	TEST(strcmp("[1,\"foo\",243,3.141593e+00,\"copyme\",\"copyme\",{\"one\":1,\"two\":2,\"three\":3},[100,200,1000,70000,5000000000]]", json) == 0); 

	struct blob b2; 
	blob_init_from_json(&b2, json); 
	blob_dump_json(&b2); 
	char *json2 = blob_to_json(&b2); 

	TEST(strcmp(json, json2) == 0); 

	free(json); 
	free(json2); 

	blob_free(&blob); 

	return 0; 
}

