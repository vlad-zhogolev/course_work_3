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

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


// directional lights
vector<DirectionalLight> dirLights;
// point lights
vector<PointLight> pointLights;
// spot lights
vector<SpotLight> spotLights;
// objects
vector<Object> objects;
// models
vector<Model> models;

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D-Scene lighting simulation program", NULL, NULL);
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
    Shader shaderGeometryPass("shaders/g_buffer.vert", "shaders/g_buffer.frag");
    Shader shaderLightingPass("shaders/deferred_shading.vert", "shaders/deferred_shading.frag");
    Shader shaderLightBox("shaders/deferred_light_box.vert", "shaders/deferred_light_box.frag");
    
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

    // configure g-buffer framebuffer    
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition;
    unsigned int gNormal;
    unsigned int gAlbedoSpec;
    unsigned int gMaterialAmbient;
    unsigned int gMaterialDiffuse;
    unsigned int gMaterialSpecular;
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // material buffers (ambient, diffuse and specular properties)
    glGenTextures(1, &gMaterialAmbient);
    glBindTexture(GL_TEXTURE_2D, gMaterialAmbient);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMaterialAmbient, 0);
    glGenTextures(1, &gMaterialDiffuse);
    glBindTexture(GL_TEXTURE_2D, gMaterialDiffuse);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gMaterialDiffuse, 0);
    glGenTextures(1, &gMaterialSpecular);
    glBindTexture(GL_TEXTURE_2D, gMaterialSpecular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gMaterialSpecular, 0);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[6] =
    { 
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT5
    };
    glDrawBuffers(6, attachments);

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // lighting info   
    const unsigned int NR_LIGHTS = 32;
    
    int pointLightsNumber = min(32, (int)pointLights.size());
    int spotLightsNumber = min(32, (int)spotLights.size());
    int dirLightsNumber = min(4, (int)dirLights.size());

    // shader configuration    
    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedoSpec", 2);
    shaderLightingPass.setInt("gMaterialAmbient", 3);
    shaderLightingPass.setInt("gMaterialDiffuse", 4);
    shaderLightingPass.setInt("gMaterialSpecular", 5);
    shaderLightingPass.setInt("pointLightsNumber", pointLightsNumber);
    shaderLightingPass.setInt("spotLightsNumber", spotLightsNumber);
    shaderLightingPass.setInt("dirLightsNumber", dirLightsNumber);
    
    // set up directional lights
    for (unsigned int i = 0; i < dirLights.size(); ++i)
    {
        shaderLightingPass.setVec3("dirLights[" + to_string(i) + "].direction", dirLights[i].getDirection());
        shaderLightingPass.setVec3("dirLights[" + to_string(i) + "].color", dirLights[i].getColor());
        shaderLightingPass.setVec3("dirLights[" + to_string(i) + "].ambient", dirLights[i].getAmbient());
        shaderLightingPass.setVec3("dirLights[" + to_string(i) + "].diffuse", dirLights[i].getDiffuse());
        shaderLightingPass.setVec3("dirLights[" + to_string(i) + "].specular", dirLights[i].getSpecular());
    }

    // set up point lights
    for (unsigned int i = 0; i < pointLights.size(); ++i)
    {
        shaderLightingPass.setVec3("pointLights[" + to_string(i) + "].ambient", pointLights[i].getAmbient());
        shaderLightingPass.setVec3("pointLights[" + to_string(i) + "].diffuse", pointLights[i].getDiffuse());
        shaderLightingPass.setVec3("pointLights[" + to_string(i) + "].specular", pointLights[i].getSpecular());
        shaderLightingPass.setVec3("pointLights[" + to_string(i) + "].position", pointLights[i].getPosition());
        shaderLightingPass.setVec3("pointLights[" + to_string(i) + "].color", pointLights[i].getColor());        
        shaderLightingPass.setFloat("pointLights[" + to_string(i) + "].constant", pointLights[i].getConstant());
        shaderLightingPass.setFloat("pointLights[" + to_string(i) + "].linear", pointLights[i].getLinear());
        shaderLightingPass.setFloat("pointLights[" + to_string(i) + "].quadratic", pointLights[i].getQuadratic());
    }

    //set up spot lights
    for (unsigned int i = 0; i < spotLights.size(); ++i)
    {
        shaderLightingPass.setVec3("spotLights[" + to_string(i) + "].position", spotLights[i].getPosition());
        shaderLightingPass.setVec3("spotLights[" + to_string(i) + "].color", spotLights[i].getColor());
        shaderLightingPass.setVec3("spotLights[" + to_string(i) + "].direction", spotLights[i].getDirection());
        shaderLightingPass.setVec3("spotLights[" + to_string(i) + "].ambient", spotLights[i].getAmbient());
        shaderLightingPass.setVec3("spotLights[" + to_string(i) + "].diffuse", spotLights[i].getDiffuse());
        shaderLightingPass.setVec3("spotLights[" + to_string(i) + "].specular", spotLights[i].getSpecular());
        shaderLightingPass.setFloat("spotLights[" + to_string(i) + "].constant", spotLights[i].getConstant());
        shaderLightingPass.setFloat("spotLights[" + to_string(i) + "].linear", spotLights[i].getLinear());
        shaderLightingPass.setFloat("spotLights[" + to_string(i) + "].quadratic", spotLights[i].getQuadratic());
        shaderLightingPass.setFloat("spotLights[" + to_string(i) + "].cutOff", glm::cos(glm::radians(spotLights[i].getCutOff())));
        shaderLightingPass.setFloat("spotLights[" + to_string(i) + "].outerCutOff", glm::cos(glm::radians(spotLights[i].getOuterCutOff())));
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

        // 1. geometry pass: render scene's geometry/color data into gbuffer        
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        /*glm::mat4 model;*/
        shaderGeometryPass.use();
        shaderGeometryPass.setMat4("projection", projection);
        shaderGeometryPass.setMat4("view", view);
        for (unsigned int i = 0; i < objects.size(); i++)
        {
            glm::mat4 model = objects[i].getModelMatrix();
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            shaderGeometryPass.setMat3("normalMatrix", normalMatrix);
            shaderGeometryPass.setMat4("model", model);
            objects[i].getModel().Draw(shaderGeometryPass);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gMaterialAmbient);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gMaterialDiffuse);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, gMaterialSpecular);
        
        // update point lights
        for (unsigned int i = 0; i < pointLights.size(); ++i)                              
            shaderLightingPass.setVec3("pointLights[" + to_string(i) + "].position", pointLights[i].getPosition());                            

        // update spot lights
        for (unsigned int i = 0; i < spotLights.size(); ++i)        
            shaderLightingPass.setVec3("spotLights[" + to_string(i) + "].position", spotLights[i].getPosition());
            
        shaderLightingPass.setVec3("viewPos", camera.Position);
        // finally render quad
        renderQuad();

        // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer        
        // write to default framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                                                   
        // blit to default framebuffer.                                           
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. render lights on top of scene        
        shaderLightBox.use();            
        shaderLightBox.setMat4("projection", projection);
        shaderLightBox.setMat4("view", view);
        for (unsigned int i = 0; i < pointLights.size(); ++i)
        {
            glm::mat4 model = glm::mat4();
            model = glm::translate(model, pointLights[i].getPosition());
            model = glm::scale(model, glm::vec3(0.125f));
            shaderLightBox.setMat4("model", model);
            shaderLightBox.setVec3("lightColor", pointLights[i].getColor());
            renderCube();
        }

        for (unsigned int i = 0; i < spotLights.size(); ++i)
        {
            glm::mat4 model = glm::mat4();
            model = glm::translate(model, spotLights[i].getPosition());
            glm::quat rotation;
            rotation = glm::rotation(glm::vec3(0.0f, -1.0f, 0.0f), glm::normalize(spotLights[i].getDirection()));
            model *= glm::toMat4(rotation);
            model = glm::scale(model, glm::vec3(0.25f));
            shaderLightBox.setMat4("model", model);
            shaderLightBox.setVec3("lightColor", spotLights[i].getColor());
            renderPyramid();           
        }

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