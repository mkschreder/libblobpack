#include "test-funcs.h"
#include <blobpack.h>
#include <stdbool.h>
#include <math.h>
#include <memory.h>

int main(){
	struct blob blob; 
	blob_init(&blob, 0, 0); 
	char data_in[] = {1, 2, 3, 4, 5, 6, 7}; 
	char data_out[sizeof(data_in)]; 

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
	blob_put_int(&blob, 1); 
	blob_put_int(&blob, 2); 
	blob_put_int(&blob, 3); 
	blob_close_array(&blob, o); 

	blob_dump(&blob); 

	struct blob_field *root = blob_head(&blob); 
	struct blob_field *child = blob_field_first_child(root); 

	TEST(blob_field_get_bool(child) == true); 
	TEST(strcmp(blob_field_get_string(child = blob_field_next_child(root, child)), "foo") == 0); 
	TEST(blob_field_get_int(child = blob_field_next_child(root, child)) == -13); 
	TEST(blob_field_get_real(child = blob_field_next_child(root, child)) == M_PI); 

	TEST(strcmp(blob_field_get_string(child = blob_field_next_child(root, child)), "copyme") == 0); 
	TEST(strcmp(blob_field_get_string(child = blob_field_next_child(root, child)), "copyme") == 0); 

	// iterate over the list 
	child = blob_field_next_child(root, child); 
	struct blob_field *list = child; 
	child = blob_field_first_child(child); 

	TEST(strcmp(blob_field_get_string(child), "one") == 0); 
	TEST(blob_field_get_int(child = blob_field_next_child(list, child)) == 1); 

	TEST(strcmp(blob_field_get_string(child = blob_field_next_child(list, child)), "two") == 0); 
	TEST(blob_field_get_int(child = blob_field_next_child(list, child)) ==  2); 

	TEST(strcmp(blob_field_get_string(child = blob_field_next_child(list, child)), "three") == 0); 
	TEST(blob_field_get_int(child = blob_field_next_child(list, child)) == 3); 

	// make sure we correctly return null when no more children available
	TEST(blob_field_next_child(list, child) == 0); 

	// test the array element
	list = blob_field_next_child(root, list); 
	child = blob_field_first_child(list); 
	TEST(blob_field_get_int(child) == 1); 
	TEST(blob_field_get_int(child = blob_field_next_child(list, child)) == 2); 
	TEST(blob_field_get_int(child = blob_field_next_child(list, child)) == 3); 

	return 0; 
}

