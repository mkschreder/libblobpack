#include <stdio.h>
#include <inttypes.h>

#include <blobpack.h>

static const char *indent_str = "\t\t\t\t\t\t\t\t\t\t\t\t\t";

#define indent_printf(indent, ...) do { \
	if (indent > 0) \
		fwrite(indent_str, indent, 1, stderr); \
	fprintf(stderr, __VA_ARGS__); \
} while(0)

static void dump_attr_data(struct blob_attr *data, int indent, int next_indent);

static void
dump_table(struct blob_attr *head, int len, int indent, bool array)
{
	struct blob_attr *attr;

	indent_printf(indent, "{\n");
	for(attr = blob_attr_first_child(head); attr; attr = blob_attr_next_child(head, attr)){
		if (!array){
			indent_printf(indent + 1, "%s : ", blob_attr_get_string(attr));
			attr = blob_attr_next_child(head, attr); 
		}
		dump_attr_data(attr, 0, indent + 1);
	}
	indent_printf(indent, "}\n");
}
static void dump_attr_data(struct blob_attr *data, int indent, int next_indent)
{
	int type = blob_attr_type(data);
	switch(type) {
	case BLOB_ATTR_STRING:
		indent_printf(indent, "%s\n", blob_attr_get_string(data));
		break;
	case BLOB_ATTR_INT8:
		indent_printf(indent, "%d\n", blob_attr_get_u8(data));
		break;
	case BLOB_ATTR_INT16:
		indent_printf(indent, "%d\n", blob_attr_get_u16(data));
		break;
	case BLOB_ATTR_INT32:
		indent_printf(indent, "%d\n", blob_attr_get_u32(data));
		break;
	case BLOB_ATTR_INT64:
		indent_printf(indent, "%"PRIu64"\n", blob_attr_get_u64(data));
		break;
	case BLOB_ATTR_FLOAT32: 
		indent_printf(indent, "%f\n", blob_attr_get_float(data)); 
		break; 
	case BLOB_ATTR_FLOAT64: 
		indent_printf(indent, "%Le\n", (long double) blob_attr_get_double(data)); 
		break; 
	case BLOB_ATTR_TABLE:
	case BLOB_ATTR_ARRAY:
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

static const struct blob_attr_policy pol[] = {
	[FOO_MESSAGE] = {
		.name = "message",
		.type = BLOB_ATTR_STRING,
	},
	[FOO_LIST] = {
		.name = "list",
		.type = BLOB_ATTR_ARRAY,
	},
	[FOO_TESTDATA] = {
		.name = "testdata",
		.type = BLOB_ATTR_TABLE,
	},
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static void dump_message(struct blob_buf *buf)
{
	//struct blob_attr *tb[ARRAY_SIZE(pol)];

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
fill_message(struct blob_buf *buf)
{
	void *tbl;

	//blob_buf_put_u32(buf, BLOB_ATTR_INT32, 0xbe); 
	//blob_buf_put_string(buf, BLOB_ATTR_STRING, "a string"); 

	void *root = blob_buf_open_table(buf); 
	
	blob_buf_put_string(buf, "message"); 
	blob_buf_put_string(buf, "Hello, world!");

	blob_buf_put_string(buf, "testtable"); 
	tbl = blob_buf_open_table(buf);
	blob_buf_put_string(buf, "hello"); 
	blob_buf_put_u32(buf, 1);
	blob_buf_put_string(buf, "world"); 
	blob_buf_put_string(buf, "2");
	blob_buf_close_table(buf, tbl);

	blob_buf_put_string(buf, "list"); 
	tbl = blob_buf_open_array(buf);
	blob_buf_put_u32(buf, 0);
	blob_buf_put_u32(buf, 1);
	blob_buf_put_u32(buf, 2);
	blob_buf_put_float(buf, 0.123);
	blob_buf_close_array(buf, tbl);

	blob_buf_close_table(buf, root); 
}

int main(int argc, char **argv)
{
	struct blob_buf buf;
	
	blob_buf_init(&buf, 0, 0);
	for(int c = 0; c < 10; c++){
		blob_buf_reset(&buf); 
		fill_message(&buf);
		blob_buf_dump(&buf); 
		dump_message(&buf);
		char *json = blob_buf_format_json(blob_buf_head(&buf), false); 
	 	printf("json: %s\n", json);
		free(json); 
	}
	
	blob_buf_reset(&buf); 
	//blob_buf_add_json_from_string(&buf, "{\"string\":\"Hello World\",\"array\":[1,2,3,4],\"object\":{\"foo\":\"bar\"}}"); 
	blob_buf_add_json_from_string(&buf, "[123,2,3,4]"); 
	blob_buf_dump(&buf); 

	fflush(stdout); 
	blob_buf_free(&buf); 
	return 0;
}
