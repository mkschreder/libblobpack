#include <stdio.h>
#include <inttypes.h>

#include <blobpack.h>

static const char *indent_str = "\t\t\t\t\t\t\t\t\t\t\t\t\t";

#define indent_printf(indent, ...) do { \
	if (indent > 0) \
		fwrite(indent_str, indent, 1, stderr); \
	fprintf(stderr, __VA_ARGS__); \
} while(0)

static void dump_attr_data(struct blob_field *data, int indent, int next_indent);

static void
dump_table(struct blob_field *head, int len, int indent, bool array)
{
	struct blob_field *attr;

	indent_printf(indent, "{\n");
	for(attr = blob_field_first_child(head); attr; attr = blob_field_next_child(head, attr)){
		if (!array){
			indent_printf(indent + 1, "%s : ", blob_field_get_string(attr));
			attr = blob_field_next_child(head, attr); 
		}
		dump_attr_data(attr, 0, indent + 1);
	}
	indent_printf(indent, "}\n");
}
static void dump_attr_data(struct blob_field *data, int indent, int next_indent)
{
	int type = blob_field_type(data);
	switch(type) {
	case BLOB_FIELD_STRING:
		indent_printf(indent, "%s\n", blob_field_get_string(data));
		break;
	case BLOB_FIELD_INT8:
	case BLOB_FIELD_INT16:
	case BLOB_FIELD_INT32:
	case BLOB_FIELD_INT64:
		indent_printf(indent, "%lli\n", blob_field_get_int(data));
		break;
	case BLOB_FIELD_FLOAT32: 
	case BLOB_FIELD_FLOAT64: 
		indent_printf(indent, "%Le\n", (long double) blob_field_get_real(data)); 
		break; 
	case BLOB_FIELD_TABLE:
	case BLOB_FIELD_ARRAY:
		if (!indent)
			indent_printf(indent, "\n");
		//dump_table(data, blob_data_len(data),
		//		   next_indent, type == BLOBMSG_TYPE_ARRAY);
		break;
	}
}

enum {
	FOO_MESSAGE,
	FOO_LIST,
	FOO_TESTDATA
};

static const struct blob_field_policy pol[] = {
	[FOO_MESSAGE] = {
		.name = "message",
		.type = BLOB_FIELD_STRING,
	},
	[FOO_LIST] = {
		.name = "list",
		.type = BLOB_FIELD_ARRAY,
	},
	[FOO_TESTDATA] = {
		.name = "testdata",
		.type = BLOB_FIELD_TABLE,
	},
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static void dump_message(struct blob *buf)
{
	//struct blob_field *tb[ARRAY_SIZE(pol)];

	/*if (blob_parse(buf, pol, ARRAY_SIZE(pol), tb) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}
	if (tb[FOO_MESSAGE])
		fprintf(stderr, "Message: %s\n", (char *) blobmsg_data(tb[FOO_MESSAGE]));

	if (tb[FOO_LIST]) {
		fprintf(stderr, "List: ");
		dump_table(tb[FOO_LIST], blobmsg_data_len(tb[FOO_LIST]), 0, true);
	}
	if (tb[FOO_TESTDATA]) {
		fprintf(stderr, "Testdata: ");
		dump_table(tb[FOO_TESTDATA], blobmsg_data_len(tb[FOO_TESTDATA]), 0, false);
	}*/
}

static void
fill_message(struct blob *buf)
{
	void *tbl;

	//blob_put_u32(buf, BLOB_FIELD_INT32, 0xbe); 
	//blob_put_string(buf, BLOB_FIELD_STRING, "a string"); 

	void *root = blob_open_table(buf); 
	
	blob_put_string(buf, "message"); 
	blob_put_string(buf, "Hello, world!");
	
	blob_put_string(buf, "testtable"); 
	tbl = blob_open_table(buf);
		blob_put_string(buf, "hello"); 
		blob_put_int(buf, 1);
		blob_put_string(buf, "world"); 
		blob_put_string(buf, "2");
	blob_close_table(buf, tbl);
	blob_close_table(buf, root); 
	
	blob_put_string(buf, "list"); 
	tbl = blob_open_array(buf);
	root = blob_open_table(buf);
		blob_put_string(buf, "world"); 
		blob_put_int(buf, 0);
		blob_put_string(buf, "world"); 
		blob_put_int(buf, 1);
		blob_put_string(buf, "world"); 
		blob_put_int(buf, 2);
	blob_close_table(buf, root); 
	root = blob_open_table(buf);
		blob_put_string(buf, "world"); 
		blob_put_int(buf, 0);
		blob_put_string(buf, "world"); 
		blob_put_int(buf, 1);
		blob_put_string(buf, "world"); 
		blob_put_int(buf, 2);
	blob_close_table(buf, root); 

	blob_close_array(buf, tbl);

}

int main(int argc, char **argv)
{
	struct blob buf;
	
	blob_init(&buf, 0, 0);
	for(int c = 0; c < 10; c++){
		blob_reset(&buf); 
		fill_message(&buf);
		
		blob_dump(&buf); 
		dump_message(&buf);
		char *json = blob_to_json(&buf); 
	 	printf("json: %s\n", json);
		free(json); 
		const char *sig = "{sv}s[{si}]"; 
		if(!blob_field_validate(blob_head(&buf), sig)) {
			printf("invalid signature! %s\n", sig); 
			break; 
		} else {
			printf("VALIDATION OK! %s\n", sig); 
		}
	}
	
	blob_reset(&buf); 
	const char *json = "{\"test\":[123,2,3,4,\"string\",{\"foo\":\"bar\"}],\"foo\":\"bar\",\"obj\":{\"arr\":[]},\"part\":1.4}"; 
	struct timespec start, end; 
	clock_gettime(CLOCK_MONOTONIC, &start); 
	blob_put_json(&buf, json); 
	clock_gettime(CLOCK_MONOTONIC, &end); 
	printf("encode json: %s\n", json); 
	printf("time taken %lums\n", (end.tv_nsec - start.tv_nsec) / 1000); 
	blob_dump(&buf); 
	blob_field_dump_json(blob_head(&buf)); 
	
	// test the normal encoder/decoder
	blob_reset(&buf); 
	clock_gettime(CLOCK_MONOTONIC, &start); 
	blob_put_json(&buf, json); 
	clock_gettime(CLOCK_MONOTONIC, &end); 
	printf("encode jsconc: %s\n", json); 
	printf("time taken %lums\n", (end.tv_nsec - start.tv_nsec) / 1000); 
	
	fflush(stdout); 
	blob_free(&buf); 
	return 0;
}
