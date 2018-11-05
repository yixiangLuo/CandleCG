// Std. Includes
#include <string>

// glad
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL/SOIL.h>

#define NR_POINT_LIGHTS 2
#define PI 3.141592653

// Properties
GLuint screenWidth = 800, screenHeight = 600;

bool mouseMoveMode = true;
bool lightMoveMode = false;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

struct Light {
    glm::vec3 position;

    float constant;
    float linear;
    float quadratic;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

// The MAIN function, from here we start our application and run the Game loop
int main()
{
  // Init GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr); // Windowed
  glfwMakeContextCurrent(window);

  // Set the required callback functions
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  if (!gladLoadGL()) {
		printf("Failed to load OpenGL and its extensions");
		return(-1);
	}
	printf("OpenGL Version %d.%d loaded", GLVersion.major, GLVersion.minor);

  // Define the viewport dimensions
  glViewport(0, 0, screenWidth, screenHeight);

  // Setup some OpenGL options
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if(mouseMoveMode) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Setup and compile our shaders
  Shader modelShader(SHADER_DIR "model_vs.c", SHADER_DIR "model_frag.c");
  Shader lightShader(SHADER_DIR "light_vs.c", SHADER_DIR "light_frag.c");
  Shader depthShader(SHADER_DIR "shadow_vs.c", SHADER_DIR "shadow_frag.c", SHADER_DIR "shadow_geo.c");
  // Shader lampShader("../../../Path/To/Shaders/lamp.vs", "../../../Path/To/Shaders/lamp.frag");

  // Load models
  // Model Candle(MODEL_DIR "nanosuit/nanosuit.obj");
  Model Candle(MODEL_DIR "candle/candle.obj");
  Model Flame(MODEL_DIR "candle/flame.obj");
  Model Floor(MODEL_DIR "floor/floor.obj");
  // Model Candle(MODEL_DIR "candle2/3d-model.obj");
  // Used a lamp object here. Find one yourself on the internet, or create your own one ;) (or be oldschool and set the VBO and VAO yourselves)
  // Model lightBulb("../../../Path/To/Lamps/Bulb.obj");

  // Draw in wireframe
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // set model matrix
  glm::mat4 model;
  model = glm::translate(model, glm::vec3(-2.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
  model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down

  float light_theta = PI/2.0;

  // Point lights
  Light lights[NR_POINT_LIGHTS];
  glm::vec3 modelPos(model * glm::vec4(10.0711, 6.9, 0.4750, 1));
  lights[0].position = modelPos;
  lights[0].ambient = glm::vec3(0.05f, 0.05f, 0.05f);
  lights[0].diffuse = glm::vec3(1.0f, 0.77f, 0.35f);
  lights[0].specular = glm::vec3(1.0f, 0.77f, 0.35f);
  lights[0].constant = 1.0f;
  lights[0].linear = 0.009;
  lights[0].quadratic = 0.0032;
  lights[1].position = glm::vec3(cos(light_theta) * 6.0, 3.6f, sin(light_theta) * 6.0);
  lights[1].ambient = glm::vec3(0.1f, 0.1f, 0.1f);
  lights[1].diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
  lights[1].specular = glm::vec3(0.8f, 0.8f, 0.8f);
  lights[1].constant = 1.0f;
  lights[1].linear = 0.009;
  lights[1].quadratic = 0.0032;

  // configure depth map FBO
  // -----------------------
  const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
  unsigned int depthMapFBO[NR_POINT_LIGHTS];
  unsigned int depthCubemap[NR_POINT_LIGHTS];
  for(int i; i<NR_POINT_LIGHTS; i++){
    glGenFramebuffers(1, &depthMapFBO[i]);
    // create depth cubemap texture
    glGenTextures(1, &depthCubemap[i]);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[i]);
    for (unsigned int j = 0; j < 6; ++j)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap[i], 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  // Game loop
  while(!glfwWindowShouldClose(window))
  {
    // Set frame time
    GLfloat currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Check and call events
    glfwPollEvents();
    Do_Movement();

    // Clear the colorbuffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // move light position over time
    if(lightMoveMode){
      light_theta += deltaTime * 0.1;
      lights[1].position.x = cos(light_theta) * 6.0;
      lights[1].position.z = sin(light_theta) * 6.0;
    }

    // 1. render scene to depth cubemap
    // -----------------------------------------------
    float near_plane = 1.0f;
    float far_plane = 25.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
    std::vector<glm::mat4> shadowTransforms;
    for(int i=0; i<NR_POINT_LIGHTS; i++){
      shadowTransforms.clear();
      shadowTransforms.push_back(shadowProj * glm::lookAt(lights[i].position, lights[i].position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lights[i].position, lights[i].position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lights[i].position, lights[i].position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lights[i].position, lights[i].position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lights[i].position, lights[i].position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
      shadowTransforms.push_back(shadowProj * glm::lookAt(lights[i].position, lights[i].position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

      // 1. render scene to depth cubemap
      // --------------------------------
      glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
      glClear(GL_DEPTH_BUFFER_BIT);
      depthShader.use();
      for (unsigned int j = 0; j < 6; ++j){
        depthShader.setMat4("shadowMatrices[" + std::to_string(j) + "]", shadowTransforms[j]);
      }
      depthShader.setFloat("far_plane", far_plane);
      depthShader.setVec3("lightPos", lights[i].position);
      depthShader.setMat4("model", model);
      Candle.Draw(depthShader);
      Floor.Draw(depthShader);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 2. render scene as normal
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    modelShader.use();   // <-- Don't forget this one!
    // Transformation matrices
    glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    modelShader.setMat4("projection", projection);
    modelShader.setMat4("view", view);
    modelShader.setMat4("model", model);

    // Set the lighting uniforms
    modelShader.setVec3("viewPos", camera.Position);

    for(int i=0; i<NR_POINT_LIGHTS; i++){
      modelShader.setVec3("pointLights[" + std::to_string(i) + "].position", lights[i].position);
      modelShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", lights[i].ambient);
      modelShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", lights[i].diffuse);
      modelShader.setVec3("pointLights[" + std::to_string(i) + "].specular", lights[i].specular);
      modelShader.setFloat("pointLights[" + std::to_string(i) + "].constant", lights[i].constant);
      modelShader.setFloat("pointLights[" + std::to_string(i) + "].linear", lights[i].linear);
      modelShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", lights[i].quadratic);
    }

    // Draw the loaded model
    GLuint depthMapUnit;
    GLuint candle_max = Candle.MaxNumTexture();
    GLuint floor_max = Floor.MaxNumTexture();
    depthMapUnit = candle_max > floor_max? candle_max : floor_max;

    for(int i=0; i<NR_POINT_LIGHTS; i++){
      glActiveTexture(GL_TEXTURE0 + depthMapUnit + i);
      glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[i]);
      modelShader.setInt("shadowMap" + std::to_string(i+1), depthMapUnit + i);
    }
    modelShader.setFloat("far_plane", far_plane);
    Candle.Draw(modelShader);
    Floor.Draw(modelShader);

    lightShader.use();
    lightShader.setMat4("projection", projection);
    lightShader.setMat4("view", view);
    lightShader.setMat4("model", model);
    Flame.Draw(lightShader);

    // Swap the buffers
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
  // Camera controls
  if(keys[GLFW_KEY_W])
      camera.ProcessKeyboard(FORWARD, deltaTime);
  if(keys[GLFW_KEY_S])
      camera.ProcessKeyboard(BACKWARD, deltaTime);
  if(keys[GLFW_KEY_A])
      camera.ProcessKeyboard(LEFT, deltaTime);
  if(keys[GLFW_KEY_D])
      camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);
  if(key == GLFW_KEY_M && action == GLFW_PRESS){
    mouseMoveMode = !mouseMoveMode;
    if(mouseMoveMode) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
  if(key == GLFW_KEY_P && action == GLFW_PRESS){
    lightMoveMode = !lightMoveMode;
  }

  if(action == GLFW_PRESS)
      keys[key] = true;
  else if(action == GLFW_RELEASE)
      keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  if(firstMouse)
  {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
  }

  GLfloat xoffset = xpos - lastX;
  GLfloat yoffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  if(mouseMoveMode) camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(yoffset);
}

#pragma endregion
