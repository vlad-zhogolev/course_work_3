#include "Mesh.h"

using namespace std;

std::string to_string(TextureType type)
{
    string result;
    switch (type)
    {
        case TextureType::Diffuse:
            return "texture_diffuse";
        case TextureType::Normal:
            return "texture_normal";
        case TextureType::Metallic:
            return "texture_metallic";
        case TextureType::AmbientOcclusion:
            return "texture_ao";
        case TextureType::Roughness:
            return "texture_roughness";    
    }
}

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    setupMesh();
}

void Mesh::Draw(Shader shader)
{
    // bind appropriate textures
    unsigned int diffuseNr = 0;   
    unsigned int normalNr = 0;
    unsigned int metallicNr = 0;
    unsigned int ambientOcclusionNr = 0;
    unsigned int roughnessNr = 0;

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
                                          // retrieve texture number (the N in diffuse_textureN)
        unsigned int number;       
        
        switch (textures[i].type)
        {
        case TextureType::Diffuse:
            number = ++diffuseNr;
            break;       
        case TextureType::Normal:
            number = ++normalNr;
            break;
        case TextureType::Metallic:
            number = ++metallicNr;
            break;
        case TextureType::AmbientOcclusion:
            number = ++ambientOcclusionNr;
            break;
        case TextureType::Roughness:
            number = ++roughnessNr;
            break;
        }        

        // now set the sampler to the correct texture unit           
        shader.setInt(to_string(textures[i].type) + to_string(number), i);
        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    //bind material properties
    shader.setVec3("ambient", ambient);
    shader.setVec3("diffuse", diffuse);
    shader.setVec3("specular", specular);
    shader.setFloat("shininess", BLINN_PHONG * shininess);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // set textures to default
    for (unsigned int i = 0; i < textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    //set material properties to default
    shader.setVec3("ambient", glm::vec3(0));
    shader.setVec3("diffuse", glm::vec3(0));
    shader.setVec3("specular", glm::vec3(0));
    shader.setFloat("shininess", 0);
    glActiveTexture(GL_TEXTURE0);//set active texture to default
}

void Mesh::setupMesh()
{
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}