Binary Blob Library
-------------------

This small library allows you to pack objects into binary blobs in a cross
platform fashion. 

Blobpack is based on blobmsg code from openwrt libubox, however there were
inconsistencies in the original code that were not easily fixable because many
applications depend on the public contract of libubox. Therefore, libblobpack
was created where the original interface has been refactored and cleaned up
without modifying the underlying functionality of the library. 

How does it work?
-----------------

Blobpack packs attributes into a blob buffer. Each attribute has an id which
identifies the type of attribute inside the blob buffer. The data inside the
buffer is packed as a list of blob\_attr structures like this: 

	+-----------------+-----------------+
	| id + len | data | id + len | data |  
	+-----------------+-----------------+

The id is packed with length as a single 32 bit integer. The limitation is that
you can only have up to 127 field types with maximum data length of around 16M
per field. 

The bits of the id + len field are like this: 

	[ ettt tttt ssssssss ssssssss ssssssss ]

	- e: extended bit
	- i: id bits
	- s: size bits 

ID can be one of the following: 

	BLOB_ATTR_UNSPEC: unspecified (0)
	BLOB_ATTR_NESTED: this attribute is a nested blob buffer
	BLOB_ATTR_BINARY: the attribute contains binary data
	BLOB_ATTR_STRING: the attribute is a null terminated string
	BLOB_ATTR_INT8: the attribute is a char
	BLOB_ATTR_INT16: the attribute is a short integer
	BLOB_ATTR_INT32: the attribute is a 32 bit integer
	BLOB_ATTR_INT64: the attribute is a 64 bit integer

If you logical "or" your ID with BLOB\_ATTR\_EXTENDED then you can use your own
set of blob types. This is how blobmsg extension can pack it's own types using
the same underlying structure as blob buf without running the risk of getting
blob data mixed up. 

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

	+-----------------------------------+----------+------+------+
	| BLOBMSG_TYPE | EXTENDED + TOT_LEN | name_len | name | data |
	+-----------------------------------+----------+------+------+

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
