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
#include "Actor.h"
#include "AssetManager.h"

#include "ship.h"

using namespace glm;



const GLFWvidmode* mode;
int screenWidth = 800;
int screenHeight = 800;



class TestActor : public Actor2D {


public: 
    float speed = 100.0f;

    TestActor(std::string spriteName){
        this->spriteName = spriteName;
    }

    void update(double dt) override {
        if (hasInput()) {
            float movement = input->getAnalog(Action::MoveHorizontal);
            vec2 velocity(speed * movement * dt, 0);
            position += velocity;

            if (hasEventBus())
                events->emit(MoveEvent{ position, velocity, 1.0f });

            markDirty();
        }
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


	//PulseEffectRenderer pulseRenderer(pulseShader);

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
    InputDevice gamepad;
    InputDevice mouse;
    

    // ----------------- new stuff -------------------

    assetManager.loadTexture("grass", "res/grass.png");
    assetManager.loadTexture("ship", "res/ship.png");

    keyboard = {
        .type = DeviceType::Keyboard,
        .id = 0
    };
    mouse = {
        .type = DeviceType::Mouse,
        .id = 0
    };
    gamepad = {
        .type = DeviceType::Gamepad,
        .id = 0
    };

    playerInput.devices.push_back(keyboard);
    playerInput.devices.push_back(mouse);
    playerInput.devices.push_back(gamepad);

    {
        // WASD for keyboard movement
        InputBinding moveRight = {
            .device = keyboard,
            .code = GLFW_KEY_D,
            .scale = 1.0f
        };

        InputBinding moveLeft = {
            .device = keyboard,
            .code = GLFW_KEY_A,
            .scale = -1.0f
        };

        InputBinding moveUp = {
            .device = keyboard,
            .code = GLFW_KEY_W,
            .scale = 1.0f
        };

        InputBinding moveDown = {
            .device = keyboard,
            .code = GLFW_KEY_S,
            .scale = -1.0f
        };

        InputBinding leftStickX = {
            .device = gamepad,
            .axisCode = GLFW_GAMEPAD_AXIS_LEFT_X,
            .scale = 1.0f,
            .deadzone = 0.05f
        };

        InputBinding leftStickY = {
            .device = gamepad,
            .axisCode = GLFW_GAMEPAD_AXIS_LEFT_Y,
            .scale = -1.0f,
            .deadzone = 0.05f
        };

        // Right stick for aiming / secondary axes
        InputBinding rightStickX = {
            .device = gamepad,
            .axisCode = GLFW_GAMEPAD_AXIS_RIGHT_X,
            .scale = 1.0f,
            .deadzone = 0.05f
        };

        InputBinding rightStickY = {
            .device = gamepad,
            .axisCode = GLFW_GAMEPAD_AXIS_RIGHT_Y,
            .scale = -1.0f,
            .deadzone = 0.05f
        };

        InputBinding mouseX = {
            .device = mouse,
            .axisCode = 0,
            .scale = 1.0f
        };

        InputBinding mouseY = {
            .device = mouse,
            .axisCode = 1,
            .scale = -1.0f,
            .offset = (float)screenHeight
        };

        InputBinding mouseRight = {
            .device = mouse,
            .code = GLFW_MOUSE_BUTTON_2
        };

        InputBinding mouseLeft = {
            .device = mouse,
            .code = GLFW_MOUSE_BUTTON_1
        };

        InputBinding rightBumper = {
            .device = gamepad,
            .code = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER
        };

        // ---------------- Horizontal movement ----------------
        playerInput.bindings[Action::MoveHorizontal].push_back(moveRight);
        playerInput.bindings[Action::MoveHorizontal].push_back(moveLeft);
        playerInput.bindings[Action::MoveHorizontal].push_back(leftStickX);

        // ---------------- Vertical movement ----------------
        playerInput.bindings[Action::MoveVertical].push_back(moveUp);
        playerInput.bindings[Action::MoveVertical].push_back(moveDown);
        playerInput.bindings[Action::MoveVertical].push_back(leftStickY);

        // ---------------- Mouse position ----------------
        playerInput.bindings[Action::MousePositionHorizontal].push_back(mouseX);
        playerInput.bindings[Action::MousePositionVertical].push_back(mouseY);


        // ---------------- Aim ----------------
        playerInput.bindings[Action::AimHorizontal].push_back(rightStickX);
        playerInput.bindings[Action::AimVertical].push_back(rightStickY);

        playerInput.bindings[Action::Aim].push_back(mouseRight);


        // ---------------- Shooting ----------------
        playerInput.bindings[Action::Shoot].push_back(mouseLeft);
        playerInput.bindings[Action::Shoot].push_back(rightBumper);

    }


    inputSystem.devices.push_back(keyboard);
    inputSystem.devices.push_back(mouse);
    inputSystem.devices.push_back(gamepad);
    inputSystem.players.push_back(playerInput);


    Ship playerShip;
    playerShip.spriteName = "ship";
    playerShip.screenMax = vec2(screenWidth, screenHeight);
    playerShip.init(&inputSystem.players.front(), &mainBus);
    playerShip.respawn(vec2(500, 500));
    playerShip.scale = vec2(50);

    TestActor testActor("grass");
    testActor.init(&inputSystem.players.front(), &mainBus);
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
        playerShip.update(dt);

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);



		//directionLine.start = ship->getWorldPosition();
		//directionLine.end = ship->getWorldPosition() + ship->forward() * 50.0f;


        double currentTime = glfwGetTime();
        if (nextFrameTime < currentTime) {
            double renderTime = currentTime;

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

            spriteRenderer.Draw(assetManager.getTexture(playerShip.spriteName)->id, playerShip.getWorldMatrix());
            

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