#ifndef INIT_H
#define INIT_H

#include <GLFW/glfw3.h>

#define RESOLUTION_W 1366
#define RESOLUTION_H 768

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *read_ppm(const char *filename_ppm, int *width, int *height);

unsigned int init_texture(const char* path);

// Renvoie un shader en c_str à partir de son fichier source
char *read_shader(const char *filename);
// Compile un shader (donc l'envoie à OpenGL) à partir du code source en c_str
unsigned int compile_shader(unsigned int type, const char *source);
// Compile deux shaders envoyés en param et renvoie un programme OpenGL
int create_program(const char *vertex_shader, const char *fragment_shader);

// Initialise le contexte OpenGL, compile les shaders et renvoie le program ID
unsigned int init(GLFWwindow** window);

#ifdef __cplusplus
}
#endif

#endif