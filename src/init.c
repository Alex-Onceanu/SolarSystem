#include <glad.h>

#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NB_VERTEX 4
#define ATTR_PER_VERTEX 3
#define NB_INDEX 6


unsigned int compile_shader(unsigned int type, const char *source)
{
    unsigned int id = glCreateShader(type);

    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    return id;
}

int create_program(const char *vertex_shader, const char *fragment_shader)
{
    unsigned int program = glCreateProgram();

    unsigned int vs = compile_shader(GL_VERTEX_SHADER, vertex_shader);
    unsigned int fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

char *read_shader(const char *filename)
{
    FILE *shader_file = fopen(filename, "r");

    int maxbuf = 200;
    char *res = malloc((1 + maxbuf) * sizeof(char));
    int i = 0;

    while (!feof(shader_file))
    {
        if (i >= maxbuf)
        {
            maxbuf *= 2;
            res = realloc(res, (1 + maxbuf) * sizeof(char));
        }
        res[i++] = fgetc(shader_file);
    }
    res[i - 1] = '\0';

    fclose(shader_file);

    return res;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void setupMesh()
{
    float vertex[NB_VERTEX * ATTR_PER_VERTEX] = {
    // x,    y,   z
    -1.0, -1.0, 0.0,
     1.0, -1.0, 0.0,
    -1.0,  1.0, 0.0,
     1.0,  1.0, 0.0};

    // Vertex buffer, on envoie à OpenGL les données du triangle
    unsigned int triang_buf;
    glGenBuffers(1, &triang_buf);
    glBindBuffer(GL_ARRAY_BUFFER, triang_buf);
    glBufferData(GL_ARRAY_BUFFER, NB_VERTEX * ATTR_PER_VERTEX * sizeof(float), vertex, GL_STATIC_DRAW);

    // Puis on lui dit comment interpréter ces données
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, ATTR_PER_VERTEX * sizeof(float), 0);

    // Index buffer, pour éviter les doublons de vertex
    unsigned int indices[NB_INDEX] = {
        0, 1, 2, 1, 3, 2};

    // Bind et interprétation du index_buffer
    unsigned int ind_buf;
    glGenBuffers(1, &ind_buf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ind_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, NB_INDEX * sizeof(unsigned int), indices, GL_STATIC_DRAW);
}

unsigned int init(GLFWwindow** window)
{
    if (!glfwInit()) return 0;

    *window = glfwCreateWindow(RESOLUTION_W, RESOLUTION_H, "This is not outer wilds", NULL, NULL);
    if (window == NULL)
    {
        printf("Error in glfwCreateWindow !\n");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return 0;
    }

    int w, h;
    glfwGetFramebufferSize(*window, &w, &h);
    glViewport(0, 0, w, h);

    // Vertex Shader, pour la position de chaque vertex
    char *vs_source = read_shader("../shaders/main.vert");

    // Fragment Shader, pour chaque pixel (gère la couleur en outre)
    char *fs_source = read_shader("../shaders/main.frag");

    unsigned int shader = create_program(vs_source, fs_source);
    glUseProgram(shader);

    free(vs_source);
    free(fs_source);

    setupMesh();

    return shader;
}