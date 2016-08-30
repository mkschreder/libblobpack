#pragma once

#define STR(x) #x                       
#define TEST(x) if(!(x)){ printf("test failed at %d, %s: %s\n", __LINE__, __FILE__, STR(x)); exit(-1); } else { printf("[OK] %s\n", STR(x)); }
