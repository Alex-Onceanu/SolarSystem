#include <glad.h>

#include "init.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NB_VERTEX 4
#define ATTR_PER_VERTEX 3
#define NB_INDEX 6

// written in one of my previous projects 2 years ago
// I adapted it so it can also read grey-scale images (.pgm format)
unsigned char *read_ppm(int is_pgm, const char *filename_ppm, int *width, int *height)
{
    FILE *fichier = fopen(filename_ppm, "r");
    if (fichier == NULL)
    {
        printf("Can't find %s\n", filename_ppm);
        return NULL;
    }
    if(fgetc(fichier) != 'P' || fgetc(fichier) != (is_pgm ? '5' : '6'))
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

    int nb_px = (*width) * (*height);
    unsigned char *res = malloc((is_pgm ? 1 : 3) * nb_px * sizeof(unsigned char));
    fread(res, sizeof(unsigned char), (is_pgm ? 1 : 3) * nb_px, fichier);

    fclose(fichier);
    return res;
}

unsigned int init_texture(const char* path)
{
    if(!path || !*path || !path[1]) return -1;

    static unsigned int dejavu = 0;
    int width, height;
    int g = 0; for(; path[g]; g++);
    int is_pgm = path[g - 2] == 'g';
    unsigned char *data = read_ppm(is_pgm, path, &width, &height);

    // Chaque texture chargée est associée à un uint, qu'on stocke dans un
    // unsigned int[], ici comme il y en a 1 seul on envoie &texture
    unsigned int texture_id;
    // Génération du texture object
    glGenTextures(1, &texture_id);

    glActiveTexture(GL_TEXTURE0 + dejavu++);

    glBindTexture(GL_TEXTURE_2D, texture_id);

    if(!is_pgm)
    {
        // On attache une image 2d à un texture object
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

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
unsigned int global_ui_program;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glUniform1f(glGetUniformLocation(global_program, "aspectRatio"), (float)(width) / (float)(height));
    glUseProgram(global_ui_program);
    glUniform1f(glGetUniformLocation(global_ui_program, "aspectRatio"), (float)(width) / (float)(height));
    glUseProgram(global_program);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
}

unsigned int initUI()
{
    char *vs_source = read_shader("../shaders/main.vert");
    char *fs_source = read_shader("../shaders/ui.frag");

    global_ui_program = create_program(vs_source, fs_source);

    free(vs_source);
    free(fs_source);

    return global_ui_program;
}

unsigned int init(GLFWwindow** window)
{
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit()) return 0;

    *window = glfwCreateWindow(RESOLUTION_W, RESOLUTION_H, "Welcome to my solar system !", NULL, NULL);
    if (window == NULL)
    {
        printf("Error in glfwCreateWindow !\n");
        glfwTerminate();
        return 0;
    }

    glfwMakeContextCurrent(*window);
    glfwSetWindowSizeLimits(*window, LOW_RES_W, LOW_RES_H, GLFW_DONT_CARE, GLFW_DONT_CARE);
    // glfwSwapInterval(0); // decomment this to remove the 60FPS cap

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    return global_program;
}

void generateLowResBuf(unsigned int* frameBuf, unsigned int* outTexture)
{
    glGenFramebuffers(1, frameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, *frameBuf);

    glGenTextures(1, outTexture);
    glBindTexture(GL_TEXTURE_2D, *outTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, LOW_RES_W, LOW_RES_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *outTexture, 0);
}