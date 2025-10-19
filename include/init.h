#ifndef INIT_H
#define INIT_H

#include <GLFW/glfw3.h>

#define RESOLUTION_W 1366
#define RESOLUTION_H 768

// #define LOW_RES_W 683
// #define LOW_RES_H 384

#define LOW_RES_W 1280
#define LOW_RES_H 720

// #define LOW_RES_W (RESOLUTION_W / 2)
// #define LOW_RES_H (RESOLUTION_H / 2)

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *read_ppm(int is_pgm, const char *filename_ppm, int *width, int *height);

unsigned int init_texture(const char* path);

// Renvoie un shader en c_str à partir de son fichier source
char *read_shader(const char *filename);
// Compile un shader (donc l'envoie à OpenGL) à partir du code source en c_str
unsigned int compile_shader(unsigned int type, const char *source);
// Compile deux shaders envoyés en param et renvoie un programme OpenGL
int create_program(const char *vertex_shader, const char *fragment_shader);

// Initialise le contexte OpenGL, compile les shaders et renvoie le program ID
unsigned int init(GLFWwindow** window);

// first render pass will be in a low res texture
void generateLowResBuf(unsigned int* frameBuf, unsigned int* outTexture);

#ifdef __cplusplus
}
#endif

#endif