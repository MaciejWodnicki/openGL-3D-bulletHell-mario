#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>

#include "shaders.h"
#include "PlayerClass.h"
#include "EnemyClass.h"
#include "gun.h"

#include <iostream>
#include <deque>
#include <vector>
#include <memory>
#include <conio.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void getMousePos(GLFWwindow* window, double& xpos, double& ypos);

void spawnEnemies();
void ResetScene();
void PlayerDied();
void updateScene();
void onEnemyDeath(int enemyIndex);


const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;

//global variables
int score = 0;

double deltaT;

glm::vec4 BackgroundColor = glm::vec4(0.7f, 0.7f, 0.9f, 1.0f);
bool reset = false;

Player player;
float currentY = 0.0f;
const float max_g = -2.98f;
float g = max_g;
bool shootingIsEnabled = true;

unsigned int defaultEnemyCount = 5;
unsigned int enemyCount = defaultEnemyCount;
std::vector<std::unique_ptr<Enemy>> enemies(enemyCount);
std::deque<std::unique_ptr<Bullet>> bullets;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Mario Survivors", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    ///////////////////////////////////////////
    // MAIN SHADERS
    // build and compile our shader program
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link main shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ///////////////////////////////////////////
// SHADOW SHADERS
// vertex shader
    unsigned int depthVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(depthVertexShader, 1, &depthVertexShaderSource, NULL);
    glCompileShader(depthVertexShader);

    glGetShaderiv(depthVertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(depthVertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int depthFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(depthFragmentShader, 1, &depthFragmentShaderSource, NULL);
    glCompileShader(depthFragmentShader);

    glGetShaderiv(depthFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(depthFragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link depth shaders
    unsigned int depthShaderProgram = glCreateProgram();
    glAttachShader(depthShaderProgram, depthVertexShader);
    glAttachShader(depthShaderProgram, depthFragmentShader);
    glLinkProgram(depthShaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(depthVertexShader);
    glDeleteShader(depthFragmentShader);


    static const float Objects[] = {
        //ground              //normals         //UV
        -10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 0.0f,
        10.0f, -0.5f,  10.0f, 0.0f, 1.0f, 0.0f, 10.0f, 10.0f,

        10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f, 10.0f, 10.0f,
        -10.0f, -0.5f,  10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 10.0f,
        -10.0f, -0.5f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f,  0.0f,

        //cube
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, //bottom
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,

         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, //top
         0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
         0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,

         0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f, //front
         0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 1.0f,

         0.5f,  0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f, //back
         0.5f, -0.5f,  0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f,

         0.5f,  0.5f,  0.5f, 0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, //left
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

         0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f, //right
         0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 1.0f,

         0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
    };

    //VAO and VBO
    unsigned int VertexBufferId, VertexArrayId;
    glGenVertexArrays(1, &VertexArrayId);
    glGenBuffers(1, &VertexBufferId);
    glBindVertexArray(VertexArrayId);

    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Objects), Objects, GL_STATIC_DRAW);

    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);                   
                                                    
    //normals attribute                             
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);                   
                                                    
    //texture attribute                             
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //load and create grass texture 
    unsigned int textureGrassId;

    glGenTextures(1, &textureGrassId);
    glBindTexture(GL_TEXTURE_2D, textureGrassId);
    //set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image, create texture and generate mipmaps
    int tex_width, tex_height, tex_nr_Channels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("grass.jpg", &tex_width, &tex_height, &tex_nr_Channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    //load and create npc texture 
    unsigned int textureNpcId;

    glGenTextures(1, &textureNpcId);
    glBindTexture(GL_TEXTURE_2D, textureNpcId);
    //set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load("enemy.jpg", &tex_width, &tex_height, &tex_nr_Channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    //load and create rock texture 
    unsigned int textureRockId;

    glGenTextures(1, &textureRockId);
    glBindTexture(GL_TEXTURE_2D, textureRockId);
    //set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load("rock.jpg", &tex_width, &tex_height, &tex_nr_Channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    //empty texture
    unsigned int texturePlayerId;

    glGenTextures(1, &texturePlayerId);
    glBindTexture(GL_TEXTURE_2D, texturePlayerId);
    //set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    data = stbi_load("player.jpg", &tex_width, &tex_height, &tex_nr_Channels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    ///////////////////////////////////////////
    // configure depth map FBO
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ////////////////////////////////
    //shader config
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram,"diffuseTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 1);


    //lighting
    glm::vec3 lightPos(0.0f, 15.0f, -1.0f);

    //constant refreshing
    glfwSwapInterval(1);

    glfwSetTime(0);
    float prevTime = 0.0f;
    //game loop
    while (!glfwWindowShouldClose(window))
    {
        deltaT = (float)glfwGetTime() - prevTime;
        prevTime = (float)glfwGetTime();

        processInput(window);
        updateScene();

        /////////////////////////////////////
        //first pass
        // render scene from light's point of view
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(depthShaderProgram);
        glActiveTexture(depthMap);

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 20.5f;
        lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, -1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        glUniformMatrix4fv(1, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

        //model matrix
        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

        
        // scene objects
         
        //Ground
        glBindTexture(GL_TEXTURE_2D, textureGrassId);
        glBindVertexArray(VertexArrayId);
        glUseProgram(depthShaderProgram);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Player

        ModelMatrix = glm::translate(glm::mat4(1.0f), player.position);
        ModelMatrix = glm::rotate(ModelMatrix, -player.rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

        glDrawArrays(GL_TRIANGLES, 6, 36);


        for (auto& enemy : enemies)
        {
            if (!enemy)
                continue;
            ModelMatrix = glm::translate(glm::mat4(1.0f), enemy->position);
            ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
            glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

            glDrawArrays(GL_TRIANGLES, 6, 36);
        }

        //bullet pew

        for (std::unique_ptr<Bullet>& bullet : bullets)
        {
            if (!bullet)
                continue;

            ModelMatrix = glm::translate(glm::mat4(1.0f), bullet->getPos());
            ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

            glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

            glDrawArrays(GL_TRIANGLES, 6, 36);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        /////////////////////////////////////
        //second pass
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        
        //choose a background color
        glClearColor(BackgroundColor.x, BackgroundColor.y, BackgroundColor.z, BackgroundColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        //lightcolor
        float redChannel = ((((float)glfwGetTime() * 5)) + 1.5f) / 8 + 0.2f;
        float greenChannel = ((sin((float)glfwGetTime() * 5)) + 1.5f) / 8 + 0.2f;
        float blueChannel = ((cos((float)glfwGetTime() * 5)) + 1.5f) / 8 + 0.2f;
        glm::vec3 lightColor = glm::vec3(0.6f);
        glUniform3f(7, lightColor.x, lightColor.y, lightColor.z);

        //projection matrix
        glm::mat4 ProjMatrix = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01f, 100.0f);

        double mouseX, mouseY;
        getMousePos(window, mouseX, mouseY);

        mouseX = mouseX / 400;
        mouseY = mouseY / 400;

        //view matrix
        glm::vec3 camera_destination = glm::vec3(player.position.x, 0.0f, player.position.z);
        glm::vec3 camera_position = glm::vec3(player.position.x + 11.0f * cos(mouseX), 3.0f, player.position.z + 11.0f * sin(mouseX));
        glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 ViewMatrix = glm::lookAt(camera_position, camera_destination, camera_up);

        glUniformMatrix4fv(3, 1, GL_FALSE, &ProjMatrix[0][0]);
        glUniformMatrix4fv(4, 1, GL_FALSE, &ViewMatrix[0][0]);
        ModelMatrix = glm::mat4(1.0f);
        glUniformMatrix4fv(5, 1, GL_FALSE, &ModelMatrix[0][0]);
        
        // set light uniforms
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"),camera_position.x, camera_position.y, camera_position.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);


        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureGrassId);

        glUseProgram(shaderProgram);

       
        // scene objects
        
        //Ground
        ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4.0f, 1.0f, 4.0f));
        glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

        glBindTexture(GL_TEXTURE_2D, textureGrassId);
        glUseProgram(shaderProgram);
        glBindVertexArray(VertexArrayId);
        glUniform4f(6, 1.0f, 1.0f, 1.0f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        //Player

        ModelMatrix = glm::translate(glm::mat4(1.0f), player.position);
        ModelMatrix = glm::rotate(ModelMatrix, -player.rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

        glBindTexture(GL_TEXTURE_2D, texturePlayerId);
        glUseProgram(shaderProgram);
        glUniform4f(6, 0.0f, 0.2f, 0.8f, 0.0f);

        glDrawArrays(GL_TRIANGLES, 6, 36);


        //Enemies

        glBindTexture(GL_TEXTURE_2D, textureNpcId);
        glUseProgram(shaderProgram);
        glUniform4f(6, 0.6f, 0.0f, 0.0f, 0.0f);

        for (auto& enemy: enemies)
        {
            if (!enemy)
                continue;

            ModelMatrix = glm::translate(glm::mat4(1.0f), enemy->position);
            ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
            glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

            glDrawArrays(GL_TRIANGLES, 6, 36);
        }


        //bullets

        glBindTexture(GL_TEXTURE_2D, textureRockId);
        glUseProgram(shaderProgram);
        glUniform4f(6, 1.0f, 1.0f, 1.0f, 0.0f);
        for (auto& b : bullets)
        {
            if (!b)
                continue;

            ModelMatrix = glm::translate(glm::mat4(1.0f), b->getPos());
            ModelMatrix = glm::rotate(ModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
            glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(ModelMatrix));

            glDrawArrays(GL_TRIANGLES, 6, 36);
        }
      
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //clear memory
    glDeleteVertexArrays(1, &VertexArrayId);
    glDeleteBuffers(1, &VertexBufferId);
    glDeleteProgram(shaderProgram);


    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        player.rotationAngle -= glm::radians(player.rotSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        player.rotationAngle += glm::radians(player.rotSpeed);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        float x = -player.speed * cos(player.rotationAngle);
        float z = -player.speed * sin(player.rotationAngle);
        player.Move(glm::vec3(x, 0.0f, z));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        float x = player.speed * cos(player.rotationAngle);
        float z = player.speed * sin(player.rotationAngle);
        player.Move(glm::vec3(x, 0.0f, z));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !(player.jump_active) && player.canJump) {
        player.jump_speed = player.max_jump_speed;
        g = max_g;
        player.jump_active = true;
        currentY = player.position.y;
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && (player.jump_active))
    {
        g = max_g * 0.05f;
        player.jump_speed = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && player.jump_active)
        g = g * 10;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        reset = true; 
}

void getMousePos(GLFWwindow* window, double & xpos, double &ypos)
{
    glfwGetCursorPos(window, &xpos, &ypos);
}


void updateScene()
{
    player.Update(deltaT, g, currentY);

    spawnEnemies();

    for (int i = 0; i<enemies.size(); i++)
    {
        if (!enemies[i])
            continue;
        
        if (shootingIsEnabled)
        {
            std::unique_ptr<Bullet> newBullet(enemies[i]->RandomRoam(player.position, (float)glfwGetTime(), player.position.x-10, player.position.x + 10 )); // look for a spot around the player
            if(newBullet)
                bullets.push_back(std::move(newBullet));
        }

        int enemyCollision = player.CheckEnemyCollision(enemies[i]->position);

        if (enemyCollision == 2)
            onEnemyDeath(i);
        else if (enemyCollision == 1)
            PlayerDied();
    }

    for (auto it = bullets.begin(); it != bullets.end();)
    {
        if ((*it) == nullptr ||
            glm::abs((*it)->getPos().x) > player.movementBorders ||
            glm::abs((*it)->getPos().z) > player.movementBorders)
        {
            it = bullets.erase(it);
            continue;
        }
        
        (*it)->updateBulletPos();

        if (player.CheckBulletCollision((*it)->getPos()))
        {
            PlayerDied();
            break;
        }
        it++;

    }

    ResetScene();

}

void ResetScene()
{
    if (reset == true)
    {
        for (auto& b : bullets)
            b.release();

        enemies.resize(defaultEnemyCount);
        for (auto& enemy : enemies)
            enemy.release();

        bullets.erase(bullets.begin(), bullets.end());

        BackgroundColor = glm::vec4(0.7f, 0.7f, 0.9f, 1.0f);
        //mouseDeltaX = glm::radians(90.0f);
        
        player.position = glm::vec3(0.0f, 0.0f, 0.0f);
        
        player.speed = player.maxSpeed;
        player.rotationAngle = player.initRotation;
        enemyCount = defaultEnemyCount;

        reset = false;
        shootingIsEnabled = true;
        player.canJump = true;
    }
}

void PlayerDied()
{
    shootingIsEnabled = false;
    player.speed = 0.0f;
    player.position = glm::vec3(0.0f);
    player.canJump = false;

    score = 0;

    BackgroundColor = glm::vec4(0.7f, 0.1f, 0.1f, 1.0f);

    for (auto& b : bullets)
        b.release();

   //console prints
   system("CLS");
   std::cout << "Score: " << score << std::endl;
   std::cout << "Enemy count: " << enemies.size() << std::endl;
}

void spawnEnemies()
{

    for (int i =0; i<enemies.size(); i++)
    {
        if (enemies[i])
            continue;
        
        //random point in a ring
        float randfloat = glm::sqrt(glm::linearRand<float>(0, 1));
        float bigR = 25.0f;
        float smallR = 20.0f;

        float theta = randfloat * 2 * glm::pi<float>();

        float innerX = smallR * cos(theta);
        float innerY = smallR * sin(theta);
        float outerX = bigR * cos(theta);
        float outerY = bigR * sin(theta); 
        
        glm::vec3 spawnPosition = glm::linearRand(
            glm::vec3(
                player.position.x + innerX,
                0.5f,
                player.position.y + innerY
            ),
            glm::vec3(
                player.position.x + outerX,
                0.5f,
                player.position.y + outerY
            ));
        enemies[i] = std::make_unique<Enemy>(spawnPosition);
    }
}
void onEnemyDeath(int enmeyIndex)
{
    enemies[enmeyIndex].release();
    score++;
    if (score % 5 == 0)
    {
        enemyCount++;
        enemies.resize(enemies.size() + 1);
    }
    
    //console prints
    system("CLS");
    std::cout << "Score: " << score << std::endl;
    std::cout << "Enemy count: " << enemies.size() << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    
}