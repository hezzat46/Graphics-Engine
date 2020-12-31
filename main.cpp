#include <application.hpp>
#include <iostream>
#include <fstream>
#include <cassert>
#include "input/keyboard.hpp"
#include "input/mouse.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <shader.hpp>
#include <memory>
#include <utility>
#include <texture/texture-utils.h>
#include <glm/gtx/euler_angles.hpp>
#include <json/json.hpp>
#include <mesh/common-vertex-types.hpp>
#include <mesh/common-vertex-attributes.hpp>
#include "../ECS/Entity/Entity.h"
#include "../ECS/Component/TransformComponent.h"
#include "../ECS/Component/CameraComponent.h"
#include "../ECS/Component/CameraControllerComponent.h"
#include "../ECS/Component/MeshRenderer.h"
#include "../ECS/Component/RenderState.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include <vector>
#include "../ECS/light/light.h"
#pragma region helper_functions

std::string read_file(const char *filename)
{
    std::ifstream fin(filename);
    if (fin.fail())
    {
        std::cerr << "Unable to open shader file: " << filename << std::endl;
        std::exit(-1);
    }
    return std::string(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());
}
void checkShaderCompilationErrors(GLuint shader)
{
    //Check and log for any error in the compilation process.
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status); // Takes a shader and returns the status of this shader program.
    if (!status)
    { // If there is a status (status != 0):
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length); // Get the error length (char array length).
        char *logStr = new char[length];
        glGetShaderInfoLog(shader, length, nullptr, logStr); // Get the error char array.
        std::cerr << "ERROR:" << logStr << std::endl;        // print the char array of the log error.
        delete[] logStr;
        std::exit(-1);
    }
}
void attachShader(GLuint program, const char *filename, GLenum shader_type)
{

    // 1. Reads the shader from file, compiles it,
    std::string source_code = read_file(filename);
    const char *source_code_as_c_str = source_code.c_str();

    // 2. Pass the program as a string to the GPU and then compile it.
    GLuint shader = glCreateShader(shader_type);
    // Function parameter:
    // shader (GLuint): shader object name.
    // count (GLsizei): number of strings passed in the third parameter. We only have one string here.
    // string (const GLchar**): an array of source code strings.
    // lengths (const GLint*): an array of string lengths for each string in the third parameter. if null is passed,
    //                          then the function will deduce the lengths automatically by searching for '\0'.
    glShaderSource(shader, 1, &source_code_as_c_str, nullptr);
    glCompileShader(shader);
    

    // 3. Check for any Compilation errors.
    checkShaderCompilationErrors(shader);

    // 4. Attach this shader to the program if no errors found in shader.
    glAttachShader(program, shader);

    // 5. Delete the shader as it is already attached to the program.
    glDeleteShader(shader);
}
// Reads a file using a path and returns the file text in a string.
void checkProgramLinkingErrors(GLuint program)
{
    //Check and log for any error in the linking process.
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status); // Takes a shader program (vertex & fragment) and returns the status of this shader program.
    if (!status)                                      // If there is a status (status != 0):
    {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length); // Get the error length (char array length).
        char *logStr = new char[length];
        glGetProgramInfoLog(program, length, nullptr, logStr); // Get the error char array.
        std::cerr << "LINKING ERROR: " << logStr << std::endl; // print the char array of the log error.
        delete[] logStr;
        std::exit(-1);
    }
}

#pragma endregion

GLuint program = 0, vertex_array = 0;
int keyboard = 2;
glm::vec2 mouse = glm::vec2(0, 0);
GameState *switchState(int state);

class ShaderIntroductionApplication : public our::Application
{

    // These unsigned integers represent the way we communicate with the GPU.
    // They act like a name (or, in other words, an ID).
    // These uint are passed to the GL header functions to tell the GL which OpenGL object
    // we are referring to. Also the values are set by the GL functions when passed by reference.

    // We need a window with title: "Shader Introduction", size: {1280, 720}, not full screen.
};

our::Application *app = new ShaderIntroductionApplication();

class PlayState : public GameState
{

    our::ShaderProgram shader;
    our::ShaderProgram *ptrShader;
    our::Mesh *ptrModel;
    our::Mesh *ptrModel2;
    our::Mesh model;
    our::Mesh model2;
   

    std::vector<Entity *> Entities;
    std::vector<Entity *> bullets;

    glm::mat4 cameraMatrix;
    Entity *Quad = new Entity();
    Entity *Sphere = new Entity();
    Entity *CameraEntity = new Entity();
    int y=1;
    int x=1;
        
    TransformComponent *transCamera = new TransformComponent();
    CameraComponent *cameraComponent = new CameraComponent();
    CameraControllerComponent *cameraController = new CameraControllerComponent();
    TransformComponent *transObj = new TransformComponent();
    TransformComponent *transObj2 = new TransformComponent();
    MeshRendererComponent *meshObj = new MeshRendererComponent();
    MeshRendererComponent *meshObj2 = new MeshRendererComponent();
    RenderState * renderStateObj = new RenderState();
    RenderState * renderStateObj2 = new RenderState();
    glm::mat4 transformationMatrix;
    glm::mat4 transformationMatrix2;
    Light light;

    GameState *handleEvents()
    {
        if (app->getKeyboard().justPressed(GLFW_KEY_ESCAPE))
        {
            return switchState(1);
        }
        return NULL;
    }

    void onEnter() override
    {
        
        light.diffuse = {1,1,1};
        light.specular = {1,1,1};
        light.ambient = {0.1f, 0.1f, 0.1f};
        light.direction = {-1, -1, -1};
        light.position = {0, 1, 2};
        light.attenuation = {0, 0, 1};
        light.spot_angle = {glm::pi<float>()/4, glm::pi<float>()/2};


        int width, height;
        glfwGetFramebufferSize(app->window, &width, &height);

        transCamera->init(app,{0, 0, 0}, {0, 0, 0}, {width, height, 1});

        CameraEntity->addComponent(cameraComponent, "camera");
        CameraEntity->addComponent(transCamera, "transform");
        CameraEntity->addComponent(cameraController, "controller");

        transObj->init(app,{0, -1, 0}, {0, 0, 0}, {5, 5, 5});
        transObj2->init(app,{-3, 8, 3}, {0, 0, 0}, {5, 5, 5});
    
        Quad->addComponent(transObj, "transform");
        Sphere->addComponent(transObj2, "transform");

        //cameraComponent->setDirection({-3,-3,-3});

        // transformationMatrix = transObj->to_mat4();
        ///  transformationMatrix2 = transObj2->to_mat4();
        cameraMatrix = cameraComponent->getcameraMatrix();
        //std::cout<<glm::to_string(cameraMatrix)<<std::endl;

        //Quad->addComponent(transObj, "transform");
        // Quad->addComponent(transObj2, "transform");
        Quad->addComponent(meshObj, "mesh");
        Sphere->addComponent(meshObj2, "mesh");
        //Quad->update();

        renderStateObj2->setDepthTesting(false,GL_LEQUAL);
        renderStateObj2->setCulling(true,GL_BACK,GL_CCW);
        renderStateObj2->setBlending(false);
        Sphere->addComponent(renderStateObj2,"renderState");


        renderStateObj->setDepthTesting(false,GL_LEQUAL);
        renderStateObj->setCulling(false,GL_BACK,GL_CCW);
        renderStateObj->setBlending(false);
        Quad->addComponent(renderStateObj,"renderState");

        

        shader.create();
        shader.attach("assets/shaders/ex11_transformation/transform.vert", GL_VERTEX_SHADER);
        shader.attach("assets/shaders/ex11_transformation/tint.frag", GL_FRAGMENT_SHADER);
        shader.link();

        our::mesh_utils::Cuboid(model, true);
        our::mesh_utils::Sphere(model2, {32, 16}, true);
        glUseProgram(shader);

        ptrShader = &shader;
        ptrModel = &model;
        ptrModel2 = &model2;
        meshObj->init(ptrShader, ptrModel);
        meshObj2->init(ptrShader, ptrModel2);

        cameraComponent->setEyePosition({10, 10, 10});
        cameraComponent->setTarget({0, 0, 0});
        cameraComponent->setUp({0, 1, 0});
        cameraComponent->setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        cameraController->initialize(app, cameraComponent);

        glClearColor(0, 0, 0, 0);
    
        Entities.push_back(CameraEntity);
        Entities.push_back(Sphere);
        Entities.push_back(Quad);
    }

    void onDraw(double deltaTime) override
    {
        // shader.set("light.diffuse", light.diffuse);
        // shader.set("light.specular", light.specular);
        // shader .set("light.ambient", light.ambient);
        // shader.set("light.direction", glm::normalize(light.direction));

        //std::cout<<glm::to_string(cameraMatrix)<<std::endl;
        cameraController->update(deltaTime);

        


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        cameraMatrix = cameraComponent->getcameraMatrix();

        for (unsigned int i = 0; i < Entities.size(); i++)
        {
            if(Entities[i]->getComponent("renderState"))
            {
                RenderState * state= ((RenderState *)Entities[i]->getComponent("renderState"));
                if(!state->getDepthTesting())
                {
                    glDisable(GL_DEPTH_TEST);
                }
                else
                {
                    glEnable(GL_DEPTH_TEST);
                }
                if(!state->getCulling())
                {
                    glDisable(GL_CULL_FACE);
                }
                else
                {
                    glEnable(GL_CULL_FACE);
                }
            }
            if (Entities[i]->getComponent("mesh"))
            {
                TransformComponent* transComponent = ((TransformComponent *)Entities[i]->getComponent("transform"));
                glm::mat4 transformationMatrix = transComponent->to_mat4();
                ((MeshRendererComponent *)Entities[i]->getComponent("mesh"))->draw(transformationMatrix, cameraMatrix);       
            }
            
           
        }
        for (unsigned int i = 0; i < bullets.size(); i++)
        {
            TransformComponent* transComponent =(TransformComponent *) bullets[i]->getComponent ("transform");
             transComponent->fall(deltaTime,4);
        }
        

        if (app->getKeyboard().justPressed(GLFW_KEY_SPACE))
        {
            
            if (x+2<Entities.size())
            {
                x++;

            } 
            else
            {
            
                x=0;
            }
            
        
        }
        
        if (app->getKeyboard().justPressed(GLFW_KEY_1))
        {
            createBullet();
        }
         TransformComponent* transComponent = ((TransformComponent *)Entities[x+y]->getComponent("transform"));
         transComponent->update(deltaTime);
        //     meshObj->draw(transformationMatrix,cameraMatrix);
        //     meshObj2->draw(transformationMatrix2,cameraMatrix);
    }
    void onExit() override
    {
        shader.destroy();
        model.destroy();
        cameraController->release();
    }
    void createBullet()
    {
        our::Mesh *ptr;
        TransformComponent *transObj = new TransformComponent();
        MeshRendererComponent *meshObj = new MeshRendererComponent();
        Entity *bullet  = new Entity();
        meshObj->init(ptrShader, ptr);
        transObj->init(app,{0, -4, 0}, {0, 0, 0}, {1, 0.5, 1});
        bullet->addComponent(transObj, "transform");
        bullet->addComponent(meshObj2, "mesh");
        Entities.push_back(bullet);
        bullets.push_back(bullet);
        
    }

};
PlayState *play = new PlayState();

class MenuState : public GameState
{

    //private:
public:
    //using GameState::GameState;
    //MenuState(){

    //GameState();
    // }
    GameState *handleEvents()
    {

        if (app->getKeyboard().justPressed(GLFW_KEY_ESCAPE))
        {
            return switchState(2);
        }
    }
    void onEnter() override
    {
        program = glCreateProgram(); // We ask GL to create a program for us and return a uint that we will use it by.
                                     // (act as a pointer to the created program).

        attachShader(program, "assets/shaders/PH1.vert", GL_VERTEX_SHADER);   // read the vertex shader and attach it to the program.
        attachShader(program, "assets/shaders/PH1.frag", GL_FRAGMENT_SHADER); // read the fragment shader and attach it to the program.

        glLinkProgram(program);             // Link the vertex and fragment shader together.
        checkProgramLinkingErrors(program); // Check if there is any link errors between the fragment shader and vertex shader.

        glGenVertexArrays(1, &vertex_array); // Ask GL to create a vertex array to easily create a triangle.

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set the clear color
    }
    void onDraw(double deltaTime) override
    {

        glClear(GL_COLOR_BUFFER_BIT);    // Clear the frame buffer (back buffer of the window)
        glUseProgram(program);           // Ask GL to use this program for the upcoming operations.
                                         // Every shader and rendering call after glUseProgram will now use this program object (and the shaders).
        glBindVertexArray(vertex_array); // Binding is like selecting which object to use.
                                         // Note that we need to bind a vertex array to draw
                                         // Even if that vertex array does not send any data down the pipeline

        if (app->getKeyboard().justPressed(GLFW_KEY_1))
            keyboard = 1;
        else if (app->getKeyboard().justPressed(GLFW_KEY_2))
            keyboard = 2;
        /*else if (app->getKeyboard().justPressed(GLFW_KEY_3))
            keyboard =3 ;
        else if (app->getKeyboard().justPressed(GLFW_KEY_4))
            keyboard =4 ;*/

        GLuint keyboard_uniform_location = glGetUniformLocation(program, "keyboard");
        glUniform1i(keyboard_uniform_location, keyboard);

        mouse = app->getMouse().getMousePosition();
        GLuint mouse_uniform_location = glGetUniformLocation(program, "mouse");
        glUniform2f(mouse_uniform_location, mouse.x, mouse.y);
        // Sends vertices down the pipeline for drawing
        // Parameters:
        // mode (GLenum): what primitives to draw. GL_TRIANGLES will combine each 3 vertices into a triangle.
        // first (GLint): the index of the first vertex to draw. It is useless here since we are not receiving data through the vertex array.
        // count (GLsizei): How many vertices to send to the pipeline. Since we are sending 3 vertices only, only one triangle will be drawn.
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0); // Unbind the buffer.
    }
    void onExit() override
    {
        glDeleteProgram(program);               // Cleaning up the program we compiled and saved.
        glDeleteVertexArrays(1, &vertex_array); // Clean up the array we allocated Params: (n: number of vertex array objects, array containing the n addresses of the objects)
    }
};

MenuState *menu = new MenuState();

GameState *switchState(int state)
{

    if (state == 1)
    {
        return menu;
    }
    else if (state == 2)
    {
        return play;
    }
}


int main(int argc, char **argv)
{
    // ShaderIntroductionApplication().gotoState(menu);
    app->gotoState(menu);

    // return ShaderIntroductionApplication().run();
    return app->run();
}