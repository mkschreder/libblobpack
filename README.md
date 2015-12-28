Binary Blob Library
-------------------

Blobpack is a library for packing arbitrary structured data into binary blobs.
Originally based on OpenWRT blob packing code and slightly inspired by msgpack. 

The blobs are packed in platform independent format so you can use blobpack to
pack data on one platform and then unpack it on a different one regardless of
it's endianness. 

Binary Format
-------------

Blobpack API shall be agnostic of actual packing implementation and shall
present the user with ability to pack following types: 

- int: [ −9223372036854775807, +9223372036854775807 ] range
- float: double precision float 
- string: an array of characters with 0 at the end
- array: an aggregation of any number of fields of any type
- table: an aggregation of any number of fields where each element is a pair of string and any other type

The binary format shall automatically find the smallest possible wire type to
pack the above as and the unpacking function shall read the correct type (which
can be smaller) and unpack it into a long long int which is returned to the
application. The only distinction is done between fixed point and floating
point numbers for the sake of performance.  

Note: the performance hit of casting numbers to long long is marginal enough
even on mips, to be ignored completely for the sake of a clean api. 

Blobpack packs fields into a blob buffer. Each field has a 4 byte
header containing attribyte type and length of the whole field including
both header and data. If NAME bit is set then the next two bytes are the length
of the name of the field followed by the name and then followed by the
data. 

For the sake of documenting current format, the format is outlined below: 

Unnamed fields: 

            attr             attr
	+-----------------+-----------------+
	| hhhh | data..   | hhhh | data     |  
	+-----------------+-----------------+

Named field: 
	
	+-----------------------------+
	| hhhh | ll | nnnn.. | data.. | 
	+-----------------------------+

The header consists of 4 bytes which have this layout: 

	[ ettt tttt ssssssss ssssssss ssssssss ]

	- e: reserved
	- t: type of the field (see below)
	- s: size of whole field (header+name+data)

Type can be one of the following: 

	BLOB_FIELD_ROOT: this is root element. For historical reasons it is has type of 0 
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
	
	//! Initializes a blob_buf structure. Optionally takes memory area to be copied into the buffer which must represent a valid blob buf. 
	void blob_buf_init(struct blob_buf *buf, const char *data, size_t size);
		
Packing data
------------

	struct blob buf; 
	blob_init(&buf, 0, 0); // initialized to a single root element of zero length
	blob_put_int(&buf, 1234); // will put an int16 because it is the smallest type into which the value would fit! 
	blob_put_string(&buf, "a string"); 
	blob_attr_dump_json(blob_head(&buf)); 
	
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

	blob_grow(struct blob_buf *buf, int len) - resize the buffer
		
		Resizes the allocated buffer to the specified size.  

	TODO: finish the documentation

LICENSE
-------

This software is distributed under the terms of GPLv2 general public license. 
