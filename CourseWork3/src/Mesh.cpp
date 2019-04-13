#include "Mesh.h"

using namespace std;

std::string to_string(TextureType type)
{
    string result;
    switch (type)
    {
        case TextureType::Albedo:
            return "texture_albedo";
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

Mesh::Mesh(const vector<Vertex>& vertices, const vector<unsigned int>& indices, const vector<Texture>& textures):
    _vertices(vertices),
    _indices(indices),
    _textures(textures)
{
    // Set the vertex buffers and it's attribute pointers.
    setupMesh();
}

void Mesh::Draw(Shader shader)
{
    // Bind appropriate textures

    unsigned int diffuseNr = 0;   
    unsigned int normalNr = 0;
    unsigned int metallicNr = 0;
    unsigned int ambientOcclusionNr = 0;
    unsigned int roughnessNr = 0;

    // Shader configuration    
    // Set conformity between variable name in shader and OpenGL texture.
    // Number corresponds to OpenGL texture number.
    // e.g. 0 - GL_TEXTURE0
    //      1 - GL_TEXTURE1
    //      and so on...
     for (unsigned int i = 0; i < _textures.size(); i++)
     {
         glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
                                           // retrieve texture number (the N in diffuse_textureN)
         unsigned int number;       
        
         switch (_textures[i].type)
         {
         case TextureType::Albedo:
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

         // Set the sampler to the correct texture unit           
         shader.setInt(to_string(_textures[i].type) + to_string(number), i);
         // Bind the texture
         glBindTexture(GL_TEXTURE_2D, _textures[i].id);
     }
    
    //bind material properties
    //PBR shader doesn't have them
    // shader.setVec3("ambient", ambient);
    // shader.setVec3("diffuse", diffuse);
    // shader.setVec3("specular", specular);
    // shader.setFloat("shininess", BLINN_PHONG * shininess);
    shader.setFloat("opacityRatio", _opacity);
    shader.setFloat("refractionRatio", _refraction);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // set textures to default
    for (unsigned int i = 0; i < _textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    //set material properties to default
    // shader.setVec3("ambient", glm::vec3(0));
    // shader.setVec3("diffuse", glm::vec3(0));
    // shader.setVec3("specular", glm::vec3(0));
    // shader.setFloat("shininess", 0);
    shader.setFloat("opacityRatio", 0.0);
    shader.setFloat("refractionRatio", 0.0);

    glActiveTexture(GL_TEXTURE0); //set active texture to default
}

void Mesh::setupMesh()
{
    // Create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // Load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that a pointer can be simply passed to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), &_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // Texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}