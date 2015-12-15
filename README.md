Binary Blob Library
-------------------

Blobpack is a library for packing arbitrary structured data into binary blobs.
Originally based on OpenWRT blob packing code and slightly inspired by msgpack. 

The blobs are packed in platform independent format so you can use blobpack to
pack data on one platform and then unpack it on a different one regardless of
it's endianness. 

How does it work?
-----------------

Blobpack packs attributes into a blob buffer. Each attribute has a 4 byte
header containing attribyte type and length of the whole attribute including
both header and data. If NAME bit is set then the next two bytes are the length
of the name of the attribute followed by the name and then followed by the
data. 

Unnamed attributes: 

            attr             attr
	+-----------------+-----------------+
	| hhhh | data..   | hhhh | data     |  
	+-----------------+-----------------+

Named attribute: 
	
	+-----------------------------+
	| hhhh | ll | nnnn.. | data.. | 
	+-----------------------------+

The header consists of 4 bytes which have this layout: 

	[ ettt tttt ssssssss ssssssss ssssssss ]

	- n: if set then the attribute has a name 
	- t: type of the attribute (see below)
	- s: size of whole attribute (header+name+data)

Type can be one of the following: 

	BLOB_ATTR_ROOT: this is root element. For historical reasons it is has type of 0 
	BLOB_ATTR_ARRAY: this element can only contain unnamed elements
	BLOB_ATTR_TABLE: this element can contain named elements
	BLOB_ATTR_BINARY: a binary blob. We don't care about content.  
	BLOB_ATTR_STRING: a null terminated string
	BLOB_ATTR_INT8: an 8 bit signed/unsigned integer 
	BLOB_ATTR_INT16: a 16 bit signed/unsigned integer
	BLOB_ATTR_INT32: a 32 bit signed/unsigned int
	BLOB_ATTR_INT64: a 64 bit signed/unsigned int
	BLOB_ATTR_FLOAT32: a packed 32 bit float
	BLOB_ATTR_FLOAT64: a packed 64 bit float

A blob buffer always contains one root element which has size of the whole
buffer. Root element has type bits set to all zeros.  

    +--------------------------+
	| hhhh | other elements... |
	+--------------------------+

API Reference
-------------
	
	//! Initializes a blob_buf structure. Optionally takes memory area to be copied into the buffer which must represent a valid blob buf. 
	void blob_buf_init(struct blob_buf *buf, const char *data, size_t size);

		
Packing complex structures
--------------------------

	struct blob_buf buf; 
	blob_buf_init(&buf); // buffer now contains header with size 4 bytes
	// put an unnamed element of type u8
	blob_buf_put_u8(&buf, NULL, 0x12); 
	// put a named string
	blob_buf_put_string(&buf, "name", "value"); 
	// get the raw buffer
	char *data = malloc(blob_buf_size(&buf)); 
	memcpy(data, blob_buf_head(&buf), blob_buf_size(&buf)); 

Blobmsg Extension
-----------------

On top of this structure, blobpack also provides a blobmsg class. It helps you
serialize arbitrary json data into binary blobs. Blobmsg uses the extended id
for packing it's attributes and further extends the basic blob\_buf with the
following attributes: 

	BLOBMSG_TYPE_ARRAY: an array of elements (like javascript array)
	BLOBMSG_TYPE_TABLE: a map of elements (like javascript object)
	BLOBMSG_TYPE_STRING: a named string
	BLOBMSG_TYPE_INT64: a named int64
	BLOBMSG_TYPE_INT32: a named int32
	BLOBMSG_TYPE_INT16: a named int16
	BLOBMSG_TYPE_INT8: a named int8

Note that all attributes in the blobmsg structure are named. If in the simple blob\_buf you were only required to give an element id and type, in blobmsg structure you can also name your attributes using arbitrary names. This makes it very easy to serialize complex data structures just like you would serialize json data. 

A blobmsg buffer looks like this: 

    |                        blobmsg    | blobmsg_hdr     | blobmsg_data | 
	+-----------------------------------+----------+------+--------------+
	| BLOBMSG_TYPE | EXTENDED + TOT_LEN | name_len | name |  blob_attr   |
	+-----------------------------------+----------+------+--------------+

Note: each field is padded using BLOBMSG\_PADDING(len) macro. 

Classes
-------

	* blob\_buf - a cross platform blob buffer implementation that allows packing structured data into binary blobs
	* blob\_attr - a building block of which blob\_buf is made. 
	* blobmsg - a blob buffer that represents an array of named fields (and can represent nested objects) which can be easily mapped to json
	* blobmsg\_json - a set of utility functions for converting blobmsg objects to and from json. 

Binary Blob Buffer
------------------

This is a class that allocates a binary buffer where you can then pack data. You can reuse the same blob buffer to pack data without reallocating memory by using blobbuf\_reinit() method. 
	
	blob_buf_init(struct blob_buf *buf, const char *data, size_t size) - initialize the buffer
		
		This function zeroes the blob_buf structure and allocates a default 256
		byte buffer.
		
		Optionally initialize the new buffer with the provided data. The data
		is copied into the newly allocated buffer.  
			
		usage: blob_buf_init(&buf); 
		return: returns -ENOMEM if the buffer could not be allocated. 

	blob_buf_reset(struct blob_buf *buf) - reset the buffer to point to the begining 

		Does not deallocate any memory, just resets internal pointers and total buffer length. 
		
		Reinitializes the buffer and rewinds all offsets. Use this function
		when you want to reuse a previously allocated buffer without having to
		allocate memory. 
		
		Id parameter is for usage by higher level code for giving the buffer an id. Default is 0. 

	blob_buf_grow(struct blob_buf *buf, int len) - resize the buffer
		
		Resizes the allocated buffer to the specified size.  

	TODO: finish the documentation

LICENSE
-------

This software is distributed under the terms of GPLv2 general public license. 
