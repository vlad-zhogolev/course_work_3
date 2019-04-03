#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Camera.h"
#include "SceneLoader.h"
#include "LightManager.h"
#include "Model.h"
#include "Object.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <vector>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, LightManager& lightManager);
void renderCube();
void renderPyramid();
void renderQuad();
unsigned int loadTexture(char const * path);
void renderSphere();

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

using DirectionalLights = vector<DirectionalLight>;
using PointLights = vector<PointLight>;
using SpotLights = vector<SpotLight>;
using Objects = vector<Object>;
using Models = vector<Model>;

// Max number of lights   
const PointLights::size_type MAX_NUMBER_OF_POINT_LIGHTS = 32;

// Scene contents
DirectionalLights dirLights;
PointLights pointLights;
SpotLights spotLights;
Objects objects;
Models models; 

int main()
{        
    // set russian locale
    setlocale(LC_ALL, "Russian");

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CourseWork3", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // load all OpenGL function pointers   
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }   

    // compile shaders       
    Shader shader("shaders/pbr.vert", "shaders/pbr.frag");   
    
    // load scene   
    SceneLoader sceneLoader;
    sceneLoader.loadScene("LightData.txt", "ModelData.txt", dirLights, pointLights, spotLights, models, objects);             

    // setup light manager and key callbacks for lights controls
    LightManager lightManager(pointLights, spotLights);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowUserPointer(window, &lightManager);

    // configure global opengl state    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);   
    
    PointLights::size_type pointLightsNumber = min(MAX_NUMBER_OF_POINT_LIGHTS, pointLights.size());   

    // shader configuration    
    // Set conformity between variable name in shader and OpenGL texture.
    // Number corresponds to OpenGL texture number.
    // e.g. 0 - GL_TEXTURE0
    //      1 - GL_TEXTURE1
    //      and so on...
    shader.use();
    shader.setInt("pointLightsNumber", pointLightsNumber);        

    // set up point lights
    for (unsigned int i = 0; i < pointLights.size(); ++i)
    {
        pointLights[i].setColor(pointLights[i].getColor() * glm::vec3(100));
        shader.setVec3("pointLights[" + to_string(i) + "].ambient", pointLights[i].getAmbient());
        shader.setVec3("pointLights[" + to_string(i) + "].diffuse", pointLights[i].getDiffuse());
        shader.setVec3("pointLights[" + to_string(i) + "].specular", pointLights[i].getSpecular());
        shader.setVec3("pointLights[" + to_string(i) + "].position", pointLights[i].getPosition());
        shader.setVec3("pointLights[" + to_string(i) + "].color", pointLights[i].getColor());        
        shader.setFloat("pointLights[" + to_string(i) + "].constant", pointLights[i].getConstant());
        shader.setFloat("pointLights[" + to_string(i) + "].linear", pointLights[i].getLinear());
        shader.setFloat("pointLights[" + to_string(i) + "].quadratic", pointLights[i].getQuadratic());
    }    
   
    // render loop    
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic        
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        lightManager.updateDeltaTime(deltaTime);

        //// input
        //processInput(window, lightManager);

        // render        
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        /*glm::mat4 model;*/       
        
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // Render objects
        for (unsigned int i = 0; i < objects.size(); i++)
        {
            glm::mat4 model = objects[i].getModelMatrix();           
            shader.setMat4("model", model);          
              
            objects[i].getModel().Draw(shader);
        }                

        // update point lights
        for (unsigned int i = 0; i < pointLights.size(); ++i)                              
            shader.setVec3("pointLights[" + to_string(i) + "].position", pointLights[i].getPosition());                            

        // update spot lights
        for (unsigned int i = 0; i < spotLights.size(); ++i)        
            shader.setVec3("spotLights[" + to_string(i) + "].position", spotLights[i].getPosition());
            
        shader.setVec3("cameraPos", camera.Position);
        
        // input
        processInput(window, lightManager);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {        
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,   // bottom-left
            1.0f,  1.0f, -1.0f,   // top-right
            1.0f, -1.0f, -1.0f,   // bottom-right         
            1.0f,  1.0f, -1.0f,   // top-right
            -1.0f, -1.0f, -1.0f,  // bottom-left
            -1.0f,  1.0f, -1.0f,   // top-left
           // front face
           -1.0f, -1.0f,  1.0f,  // bottom-left
           1.0f, -1.0f,  1.0f,   // bottom-right
           1.0f,  1.0f,  1.0f,   // top-right
           1.0f,  1.0f,  1.0f,  // top-right
           -1.0f,  1.0f,  1.0f,   // top-left
           -1.0f, -1.0f,  1.0f,   // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f,  // top-right
            -1.0f,  1.0f, -1.0f,  // top-left
            -1.0f, -1.0f, -1.0f,  // bottom-left
            -1.0f, -1.0f, -1.0f,  // bottom-left
            -1.0f, -1.0f,  1.0f,  // bottom-right
            -1.0f,  1.0f,  1.0f,  // top-right
            // right face
            1.0f,  1.0f,  1.0f,   // top-left
            1.0f, -1.0f, -1.0f,   // bottom-right
            1.0f,  1.0f, -1.0f,   // top-right         
            1.0f, -1.0f, -1.0f,   // bottom-right
            1.0f,  1.0f,  1.0f,   // top-left
            1.0f, -1.0f,  1.0f,   // bottom-left     
             // bottom face
            -1.0f, -1.0f, -1.0f,  // top-right
            1.0f, -1.0f, -1.0f,  // top-left
            1.0f, -1.0f,  1.0f,   // bottom-left
            1.0f, -1.0f,  1.0f,   // bottom-left
            -1.0f, -1.0f,  1.0f,   // bottom-right
            -1.0f, -1.0f, -1.0f,   // top-right
             // top face
             -1.0f,  1.0f, -1.0f, // top-left
             1.0f,  1.0f , 1.0f,   // bottom-right
             1.0f,  1.0f, -1.0f,  // top-right     
             1.0f,  1.0f,  1.0f,  // bottom-right
             -1.0f,  1.0f, -1.0f, // top-left
             -1.0f,  1.0f,  1.0f,  // bottom-left                                                                         
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

unsigned int pyramidVAO = 0;
unsigned int pyramidVBO = 0;

void renderPyramid()
{
    if (pyramidVAO == 0)
    {
        float vertices[] =
        {
            //front face
             0.5f, -0.5f,  0.5f,//2
             0.0f,  0.5f,  0.0f,//5
            -0.5f, -0.5f,  0.5f,//1              
            //rigth face
             0.5f, -0.5f, -0.5f,//3
             0.0f,  0.5f,  0.0f,//5
             0.5f, -0.5f,  0.5f,
            //left face
            -0.5f, -0.5f,  0.5f,
             0.0f,  0.5f,  0.0f,
            -0.5f, -0.5f, -0.5f,//4
            //back face
            -0.5f, -0.5f, -0.5f,
             0.0f,  0.5f,  0.0f,
             0.5f, -0.5f, -0.5f,
            //bottom face
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            
             0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,
        };

        glGenVertexArrays(1, &pyramidVAO);
        glGenBuffers(1, &pyramidVBO);
        glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(pyramidVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);        
    }
    glBindVertexArray(pyramidVAO);
    glDrawArrays(GL_TRIANGLES, 0, 18);
    glBindVertexArray(0);
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, LightManager& lightManager)
{
    //glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // camera control
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);      
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    void* obj = glfwGetWindowUserPointer(window);
    LightManager* lightManager = static_cast<LightManager*>(obj);
    if (lightManager)            
        lightManager->key_callback(window, key, scancode, action, mods);    
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        ofstream vertFout("vert.data");
        ofstream textureFout("texture.data");
        ofstream normFout("norm.data");
        ofstream indicesFout("indices.data");

        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                vertFout << xPos << " " << yPos << " " << zPos << "\n";
                textureFout << xSegment << " " << ySegment << "\n";
                normFout << xPos << " " << yPos << " " << zPos << "\n";

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indicesFout << indices[indices.size() - 2] << " " << indices[indices.size() - 1] << "\n";
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indicesFout << indices[indices.size() - 2] << " " << indices[indices.size() - 1] << "\n";
                }
            }
            oddRow = !oddRow;
        }
        indexCount = indices.size();

        std::vector<float> data;
        for (int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        float stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}