#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

//using namespace std;

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;   
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

const int BLINN_PHONG = 4;

class Mesh {
public:
    /*  Mesh Data  */
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    //unsigned int VAO;

    /*  Functions  */
    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    // render the mesh
    void Draw(Shader shader);
    void setAmbient(glm::vec3 ambient) { this->ambient = ambient; }
    void setDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
    void setSpecular(glm::vec3 specular) { this->specular = specular; }
    void setShininess(float shininess) { this->shininess = shininess; }
    glm::vec3 getAmbient() { return ambient; }
    glm::vec3 getDiffuse() { return diffuse; }
    glm::vec3 getSpecular() { return specular; }
    float getShininess() { return shininess; }

private:
    /*  Render data  */
    unsigned int VAO, VBO, EBO;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    /*  Functions    */
    // initializes all the buffer objects/arrays
    void setupMesh();
};
#endif

