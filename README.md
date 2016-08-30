Binary Blob Library
-------------------

[![Coverage Status](https://coveralls.io/repos/github/mkschreder/libblobpack/badge.svg?branch=master)](https://coveralls.io/github/mkschreder/libblobpack?branch=master)

Blobpack is a library for packing arbitrary structured data into binary blobs.
Originally based on OpenWRT blob packing code and slightly inspired by msgpack. 

The blobs are packed in platform independent format so you can use blobpack to
pack data on one platform and then unpack it on a different one regardless of
it's endianness. 

Blobpack shall be used strictly for serialization and deserialization of
arbitrary structured data. Supporting runtime representation of data with
support for modifying/inserting fields shall not be supported to keep library
simple and fast. You may keep readonly represetation of data as blobs but you
should not attempt to use blobs for storing values that change. 

Data Types
-------------

Blobpack API shall be agnostic of actual packing implementation and shall
present the user with ability to pack following types: 

- int: [ −9223372036854775807, +9223372036854775807 ] range
- real: double precision float 
- string: an array of characters with 0 at the end
- array: an aggregation of any number of fields of any type
- table: an aggregation of any number of fields where each element is a pair of
  string and any other type

Internally the types shall be packed to balance quick unpacking with space
efficiency. 

The format is intentionally made to include length even on fixed length types
like integers to simplify and speed up packing and unpacking. Some
optimizations can of course be done to the packing format, but at current time
such optimizations are deemed to be unnecessary because the gains are marginal. 

JSON Support
------------

All blobs shall be exportable and importable to and from JSON. Binary data
shall be encoded as base64 strings. 

The mapping between blobpack types and json types is as follows: 

	+-----------------------------+
	| blobpack           | JSON   | 
	+-----------------------------+
	| BLOB_FIELD_BINARY  | base64 | 
	| BLOB_FIELD_STRING  | string |
	| BLOB_FIELD_INT8    | number |
	| BLOB_FIELD_INT16   | number |
	| BLOB_FIELD_INT32   | number |
	| BLOB_FIELD_INT64   | number |
	| BLOB_FIELD_FLOAT32 | number |
	| BLOB_FIELD_FLOAT64 | number |
	| BLOB_FIELD_ARRAY   | array  | 
	| BLOB_FIELD_TABLE   | object |
	+-----------------------------+

Validation
----------

Validation is done using a validation expression that represents the blob structure. 

	Example:
	siia[sv] would match a blob like this [string, int, int, array, array of string value pairs ]

Valid elements are: 
	
	[ - open nested expression that describes an array
	] - close nested array
	{ - open nested expression that describes a table
	} - close nested table
	i - an integer
	f - a floating point number
	s - a string
	t - an arbitrary table (don't care about content)
	a - an arbitrary array (don't care about content) 

Binary Format
-------------
The binary format shall automatically find the smallest possible wire type to
pack the above as and the unpacking function shall read the correct type (which
can be smaller) and unpack it into a long long int which is returned to the
application. The only distinction is done between fixed point and floating
point numbers for the sake of performance.  

Note: the performance hit of casting numbers to long long is marginal enough
even on mips, to be ignored completely for the sake of a clean api. 

Note nr2: even though currently smallest size of number in blobpack is 4 byte
(32 bit), we still pack small integers as int8 inside a 4 byte memory area
because the packing format may be altered in the future to be more efficient. 

Blobpack packs fields into a blob buffer. Each field has a 4 byte
header containing attribyte type and length of the whole field including
both header and data. If NAME bit is set then the next two bytes are the length
of the name of the field followed by the name and then followed by the
data. 

For the sake of documenting current format, the format is outlined below: 

Unnamed fields: 

            field             field
	+-----------------+-----------------+
	| hhhh | data..   | hhhh | data     |  
	+-----------------+-----------------+

The header consists of 4 bytes which have this layout: 

	[ eeee tttt ssssssss ssssssss ssssssss ]

	- e: reserved
	- t: type of the field (see below)
	- s: size of whole field (header+data)

Type can be one of the following: 

	BLOB_FIELD_ARRAY: this element can only contain unnamed elements
	BLOB_FIELD_TABLE: this element can contain named elements
	BLOB_FIELD_BINARY: a binary blob. We don't care about content (never included in json!) 
	BLOB_FIELD_STRING: a null terminated string
	BLOB_FIELD_INT8: an 8 bit signed/unsigned integer 
	BLOB_FIELD_INT16: a 16 bit signed/unsigned integer
	BLOB_FIELD_INT32: a 32 bit signed/unsigned int
	BLOB_FIELD_INT64: a 64 bit signed/unsigned int
	BLOB_FIELD_FLOAT32: a packed 32 bit float
	BLOB_FIELD_FLOAT64: a packed 64 bit float

A blob itself always contains one root element which has size of the whole
buffer. Root element has type Array and can thus contain any number of other
elements.    

    +--------------------------+
	| hhhh | other elements... |
	+--------------------------+

API Reference
-------------
	
	//! Initializes a blob_buf structure. Optionally takes memory area to be
	//! copied into the buffer which must represent a valid blob buf. 
	//! if data is NULL and size is > 0 then the library shall create an empty
	//! memory area of the size "size" to avoid allocating more memory until the
	//! contents of the blob no longer can fit inside this memory area. 
	
	void blob_init(struct blob_buf *buf, const char *data, size_t size);

	//! deletes memroy associated with this buffer 

	void blob_free(struct blob *buf);

	//! Resets header but does not deallocate any memory.  

	void blob_reset(struct blob *buf);

	//! Resizes the buffer. Can only be used to increase size.  

	bool blob_resize(struct blob *buf, int newsize);

	//! returns the full size of the buffer memory including padding

	static inline size_t blob_size(struct blob *self){ return blob_field_raw_pad_len(blob_head(self)); }

	//! puts a string into the blob buffer

	struct blob_field *blob_put_string(struct blob *self, const char *str); 

Array/Table functions
---------------------

The following functions open a new nested element and return a "tag" to this element which can be used later to close the element. 

	//! opens an array element

	blob_offset_t blob_open_array(struct blob *buf);

	//! closes an array element

	void blob_close_array(struct blob *buf, blob_offset_t);

	//! opens an table element

	blob_offset_t blob_open_table(struct blob *buf);

	//! closes an table element

	void blob_close_table(struct blob *buf, blob_offset_t);

Write Functions
---------------

Writing to the buffer is always done at the end of the buffer. Functionality to
insert elements has been intentionally left out. Reading is done by reading
elements directly from a blob field to which they belong (all elements always
belong to a field, top level elements belong to root field which can be
retreived using blob\_head(buffer); 

	//! write a boolean into the buffer
	struct blob_field *blob_put_bool(struct blob *buf, bool val); 

	//! write a string into the buffer
	struct blob_field *blob_put_string(struct blob *buf, const char *str); 

	//! write binary data into the buffer
	struct blob_field *blob_put_binary(struct blob *buf, const char *data, size_t size); 

	//! write a number into the buffer
	struct blob_field *blob_put_int(struct blob *buf, long long val); 

	//! write a real into the buffer
	struct blob_field *blob_put_real(struct blob *buf, double value); 

	//! write a raw attribute into the buffer
	struct blob_field *blob_put_attr(struct blob *buf, struct blob_field *attr); 

	//! print out the whole buffer 
	void blob_dump(struct blob *self); 

Reading/Writing JSON
--------------------

	//! allocate a string and return it filled with json representation
	char *blob_to_json(struct blob *buf); 

	//! convert json element to blob_field and write it to the blob
	bool blob_put_json(struct blob *buf, const char *json); 

Debugging 
---------

	//! dump blob buffer contents to console using detailed info. 
	
Packing data
------------

	struct blob buf; 
	blob_init(&buf, 0, 0); // initialized to a single root element of zero length
	blob_put_int(&buf, 1234); // will put an int16 because it is the smallest type into which the value would fit! 
	blob_put_string(&buf, "a string"); 
	blob_field_dump_json(blob_head(&buf)); 
	
	// get the data
	struct blob_field *field = blob_field_first_child(blob_head(&buf)); 
	// will unpack int16 above into a long long and return the value
	printf("number: %d\n", (int)blob_field_get_int(field)); // will print 1234

Classes
-------

	* blob - a cross platform blob buffer implementation that allows packing structured data into binary blobs
	* blob\_field - represents a single field inside a blob

Binary Blob Buffer
------------------

This is a class that allocates a binary buffer where you can then pack data. You can reuse the same blob buffer to pack data without reallocating memory by using blobbuf\_reinit() method. 
	
	blob_init(struct blob_buf *buf, const char *data, size_t size) - initialize the buffer
		
		This function zeroes the blob_buf structure and allocates a default 256
		byte buffer.
		
		Optionally initialize the new buffer with the provided data. The data
		is copied into the newly allocated buffer.  
			
		usage: blob_buf_init(&buf); 
		return: returns -ENOMEM if the buffer could not be allocated. 

	blob_reset(struct blob_buf *buf) - reset the buffer to point to the begining 

		Does not deallocate any memory, just resets internal pointers and total buffer length. 
		
		Reinitializes the buffer and rewinds all offsets. Use this function
		when you want to reuse a previously allocated buffer without having to
		allocate memory. 
		
		Id parameter is for usage by higher level code for giving the buffer an id. Default is 0. 

LICENSE
-------

This software is distributed under the terms of GPLv2 general public license. 
