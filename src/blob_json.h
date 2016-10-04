/*
	Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
 	Copyright (C) 2015 Martin Schröder <mkschreder.uk@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

struct json_object;

#include <stdbool.h>
#include "blob.h"
/*
bool blob_put_json_object(struct blob *b, struct json_object *obj);
bool blob_put_json_element(struct blob *b, struct json_object *obj);
bool blob_put_json_from_string(struct blob *b, const char *str);
bool blob_put_json_from_file(struct blob *b, const char *file);
char *blob_format_json(struct blob_field *attr, bool list); 
char *blob_format_json_indent(struct blob_field *attr, bool list, int indent); 
*/

void blob_field_dump_json(const struct blob_field *self); 
void blob_field_dump_json_pretty(const struct blob_field *self); 

static inline void blob_dump_json(const struct blob *self){ blob_field_dump_json(blob_head_const(self)); }

char *blob_field_to_json(const struct blob_field *self); 
static inline char *blob_to_json(const struct blob *self){ return blob_field_to_json(blob_head_const(self)); }

bool blob_init_from_json(struct blob *self, const char *json); 

bool blob_put_json(struct blob *self, const char *json); 
bool blob_put_json_from_file(struct blob *self, const char *file); 

