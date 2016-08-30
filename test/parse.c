#include "test-funcs.h"
#include <blobpack.h>
#include <stdbool.h>
#include <math.h>
#include <memory.h>

int main(){
	struct blob blob; 
	blob_init(&blob, 0, 0); 

	blob_put_bool(&blob, true); 
	blob_put_string(&blob, "foo"); 
	blob_put_int(&blob, -13); 
	blob_put_real(&blob, M_PI); 

	struct blob_field *out[4]; 

	blob_dump_json(&blob); 

	TEST(blob_field_parse(blob_head(&blob), "isif", out, 4)); 

	TEST(blob_field_get_bool(out[0]) == true); 
	TEST(strcmp(blob_field_get_string(out[1]), "foo") == 0); 
	TEST(blob_field_get_int(out[2]) == -13); 
	TEST(is_equal(blob_field_get_real(out[3]), M_PI)); 

	return 0; 
}

