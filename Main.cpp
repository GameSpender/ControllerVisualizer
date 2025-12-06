#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <random>
#include <iostream>
#include "Util.h"
#include "SpriteRenderer.h"
#include <glm/gtc/type_ptr.hpp>

#include "LineVisualizer.h"

#include "irrKlang/irrKlang.h"
#include "glm/ext.hpp"

#include "TextRenderer.h"
#include <format>
#include "Init.h"
#include "PulseEffectRenderer.h"
#include "Actor.h"
#include "AssetManager.h"

using namespace glm;



const GLFWvidmode* mode;
int screenWidth = 800;
int screenHeight = 800;



class TestActor : public Actor2D {


public: 
    float speed = 100.0f;

    TestActor(std::string spriteName, PlayerInput &input, EventBus &event) : Actor2D(input, event) {
        this->spriteName = spriteName;
    }

    void update(double dt) override {
        float movement = input.isDown(Action::MoveRight) ? 1.0f : 0 + input.isDown(Action::MoveLeft) ? -1.0f : 0.0f;
        vec2 velocity(speed * movement * dt, 0);
        position += velocity;

        events.emit(MoveEvent{ position, velocity, 1.0f });
        markDirty();
    }

};



int main()
{
    // Inicijalizacija GLFW i postavljanje na verziju 3 sa programabilnim pajplajnom
    GLFWwindow* window = initGLFW();


    GLFWcursor* cursor = loadImageToCursor("res/grass_cursor.png");
    glfwSetCursor(window, cursor);

    // Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW failed to initialize");


    {
        mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        screenWidth = mode->width;
        screenHeight = mode->height;

    }


    CreateSceneFramebuffer(screenWidth, screenHeight);


    // Potrebno naglasiti da program koristi alfa kanal za providnost
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



    unsigned int rectShader = createShader("shaders/rect.vert", "shaders/rect.frag");
	unsigned int pulseShader = createShader("shaders/passthrough.vert", "shaders/pulse_effect.frag");

    glm::mat4 projection = glm::ortho(0.0f, (float)mode->width, 0.0f, (float)mode->height, -1.0f, 1.0f);
    glUseProgram(rectShader);
    glUniformMatrix4fv(glGetUniformLocation(rectShader, "uProjection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(pulseShader);
    glUniform2f(glGetUniformLocation(pulseShader, "uScreenSize"), screenWidth, screenHeight);


	PulseEffectRenderer pulseRenderer(pulseShader);

	//unsigned spriteTexture;
	//preprocessTexture(spriteTexture, "res/cursor.png");
	SpriteRenderer spriteRenderer(rectShader);   

    LineVisualizer directionLine(
        glm::vec2(0, 0),
        glm::vec2(1, 0),
        glm::vec3(1, 0, 0)   // red
    );





    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Postavljanje boje pozadine

    TextRenderer titleText;
    titleText.LoadFont("fonts/font.otf", 80, glm::vec3(0.8f, 0.5f, 0.1f)); // baked orange
    titleText.position = vec2(mode->width * 0.22f, mode->height * 0.7f);

    unsigned int signatureTexture = preprocessTexture("res/signature.png");

    
    // ----------------- new stuff -------------------

    InputSystem inputSystem;
    PlayerInput playerInput;
    EventBus mainBus;
    AssetManager assetManager;
    InputDevice keyboard;
    

    // ----------------- new stuff -------------------

    assetManager.loadTexture("grass", "res/grass.png");

    keyboard = {
        .type = DeviceType::Keyboard,
        .id = 0
    };

    playerInput.devices.push_back(keyboard);

    InputBinding moveRightBinding;
    InputBinding moveLeftBinding;
    InputBinding moveRightArrowBinding;

    moveRightBinding.device = keyboard;
    moveRightBinding.code = GLFW_KEY_D;
    moveLeftBinding.device = keyboard;
    moveLeftBinding.code = GLFW_KEY_A;
    moveRightArrowBinding.device = keyboard;
    moveRightArrowBinding.code = GLFW_KEY_RIGHT;
    playerInput.bindings[Action::MoveRight].push_back(moveRightBinding);
    playerInput.bindings[Action::MoveRight].push_back(moveRightArrowBinding);
    playerInput.bindings[Action::MoveLeft].push_back(moveLeftBinding);


    inputSystem.devices.push_back(keyboard);
    inputSystem.players.push_back(playerInput);




    TestActor testActor("grass", inputSystem.players.front(), mainBus);
    testActor.position = vec2(100, 100);
    testActor.scale = vec2(50);


    int framerateCap = 75;
    double frameInterval = 1.0f / framerateCap;
    double nextFrameTime = 0.0f;

    double lastTime = 0.0f;


    while (!glfwWindowShouldClose(window))
    {
        double time = glfwGetTime();
        double dt = time - lastTime;
        lastTime = time;


        inputSystem.update();
        
        testActor.update(dt);

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);



		//directionLine.start = ship->getWorldPosition();
		//directionLine.end = ship->getWorldPosition() + ship->forward() * 50.0f;


        double currentTime = glfwGetTime();
        if (nextFrameTime < currentTime) {
            float renderTime = currentTime;

            //glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, screenWidth, screenHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            titleText.DrawText(spriteRenderer, "Controller Visualizer");

            {
                auto size = vec2(100, 100);
                auto pos = vec2(screenWidth - 60, 30);
                spriteRenderer.Draw(signatureTexture, pos, size);
            }
            
            spriteRenderer.Draw(assetManager.getTexture(testActor.spriteName)->id, testActor.getWorldMatrix());

            

            //glBindFramebuffer(GL_FRAMEBUFFER, 0);

            //pulseRenderer.Render(sceneColorTex);
			//testRenderer.Render(sceneColorTex);
            
            //directionLine.Draw(colorShader, mode->width, mode->height);
            glfwSwapBuffers(window);

            currentTime = glfwGetTime();
            renderTime = currentTime - renderTime;
            nextFrameTime = currentTime + frameInterval - renderTime ;
        }

        glfwPollEvents(); 
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(pulseShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}