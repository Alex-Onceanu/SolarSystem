#include <glad.h>

#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NB_VERTEX 4
#define ATTR_PER_VERTEX 3
#define NB_INDEX 6



// written in one of my previous projects 2 years ago
unsigned char *read_ppm(const char *filename_ppm, int *width, int *height)
{
    FILE *fichier = fopen(filename_ppm, "r");
    if (fichier == NULL)
    {
        printf("Can't find %s\n", filename_ppm);
        return NULL;
    }
    if(fgetc(fichier) != 'P' || fgetc(fichier) != '6')
    {
        return NULL;
    }
    fgetc(fichier);
    if(fgetc(fichier) == '#')
    {
        while(fgetc(fichier) != '\n');
    }
    else
    {
        fseek(fichier, -1, SEEK_CUR);
    }

    int max_color = 255;
    if (fscanf(fichier, "%d %d %d", width, height, &max_color) != 3)
    {
        printf("Probleme de lecture du header ppm...");
        fclose(fichier);
        return NULL;
    }
    if (max_color != 255)
    {
        printf("Couleur ref doit etre 255, non ?\n");
        fclose(fichier);
        return NULL;
    }

    // Tout le fichier est lu d'un seul coup avec un fread approprié

    int nb_px = (*width) * (*height); // rgb
    unsigned char *res = malloc(3 * nb_px * sizeof(unsigned char));
    fread(res, sizeof(unsigned char), 3 * nb_px, fichier);

    fclose(fichier);
    return res;
}

unsigned int init_texture(const char* path)
{
    int width, height;
    unsigned char *data = read_ppm(path, &width, &height);

    // Chaque texture chargée est associée à un uint, qu'on stocke dans un
    // unsigned int[], ici comme il y en a 1 seul on envoie &texture
    unsigned int texture_id;
    // Génération du texture object
    glGenTextures(1, &texture_id);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture_id);

    // On attache une image 2d à un texture object
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture_id;
}

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

// will only be used by framebuffer_size_callback to update aspect ratio in the shader
unsigned int global_program;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    glUniform1f(glGetUniformLocation(global_program, "aspectRatio"), (float)(width) / (float)(height));
}

unsigned int init(GLFWwindow** window)
{
    if (!glfwInit()) return 0;

    *window = glfwCreateWindow(RESOLUTION_W, RESOLUTION_H, "This is not outer wilds yet", NULL, NULL);
    if (window == NULL)
    {
        printf("Error in glfwCreateWindow !\n");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(*window);

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

    global_program = create_program(vs_source, fs_source);
    glUseProgram(global_program);

    glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);

    free(vs_source);
    free(fs_source);

    setupMesh();

    return global_program;
}