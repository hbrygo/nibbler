#ifndef STB_IMAGE_STUB_H
#define STB_IMAGE_STUB_H

#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

static inline void stbi_set_flip_vertically_on_load(int) {}
static inline unsigned char* stbi_load(const char* filename, int* x, int* y, int* comp, int req_comp) {
	(void)filename;
    (void)x;
    (void)y;
    (void)comp;
    (void)req_comp;
	
    return (unsigned char*)malloc(1);
}
static inline void stbi_image_free(void* data) { free(data); }

#ifdef __cplusplus
}
#endif

#endif
