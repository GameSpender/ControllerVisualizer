#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <random>
#include <iostream>

#include "Util.h"
#include "Init.h"

#include "Services.h"

#include "ModelRenderer.h"
#include "InputSystem.h"
#include "EventBus.h"
#include "AssetManager.h"
#include "SoundManager.h"
#include "EventHandler.h"
#include "ProjectileSystem.h"
#include "CollisionSystem3D.h"
#include "LightManager.h"

#include "BindingGenerator.h"
#include "Ship3D.h"
#include "PlayerController.h"

using namespace glm;



const GLFWvidmode* mode;
int screenWidth = 800;
int screenHeight = 800;

bool debugWeapon = false;
bool debugDamage = false;




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



    unsigned int shader3D = createShader("shaders/3dColor.vert", "shaders/3dColor.frag");

    glm::vec3 cameraPos(50, 50, -50);
    glm::vec3 target(50, 0, 50);
    glm::vec3 up(0, 1, 0);
    glm::mat4 view = glm::lookAt(cameraPos, target, up);

    glm::mat4 projection = glm::perspective(
        glm::radians(60.0f),
        (float)screenWidth / (float)screenHeight,
        0.1f,
        5000.0f
    );

    //SpriteRenderer spriteRenderer(rectShader);
    ModelRenderer modelRenderer(shader3D);


    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Postavljanje boje pozadine


    // ----------------- new stuff -------------------

    PlayerInput playerInput;


    InputDevice keyboard;
    InputDevice mouse;
    InputDevice gamepad;
    

    //GamepadInput gamepadInput;


    Services::init(
        new InputSystem,
        new EventBus,
        new AssetManager,
        new SoundManager,
        new EventHandler,
        new ProjectileSystem,
		nullptr, // 2D collision system 
        new CollisionSystem3D,
        new LightManager
    );


    // ----------------- new stuff -------------------


    Services::assets->loadModel("plane", "res/models/plane/plane.gltf");


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



    //bindKeyboardAndMouse(keyboard, mouse, playerInput, screenHeight);

    bindGamepad(gamepad, playerInput);

    //bindGamepad(gamepad2, player2Input);



    Services::inputSystem->devices.push_back(keyboard);
    Services::inputSystem->devices.push_back(mouse);
    Services::inputSystem->devices.push_back(gamepad);
    Services::inputSystem->players.push_back(playerInput);

	std::shared_ptr<Transform3D> worldOrigin = std::make_shared<Transform3D>();

    std::shared_ptr<Ship3D> playerShip = std::make_shared<Ship3D>();
    playerShip->screenMax = vec2(100, 100);
    playerShip->respawn(vec3(10, 0, 10));
    playerShip->scale = vec3(1.0f);

	std::shared_ptr<Model3D> shipModel = std::make_shared<Model3D>("plane");

    playerShip->addChild(shipModel);

    worldOrigin->addChild(playerShip);



    std::shared_ptr<Collider3D> shipCollider = std::make_shared<Collider3D>(Collider3D::ShapeType3D::Cylinder);
    shipCollider->mask = CollisionLayer::All;
    shipCollider->layer = CollisionLayer::Player;
    shipCollider->scale = vec3(0.8f);
    Services::collisions3D->addCollider(shipCollider);
    playerShip->addChild(shipCollider);

    shipCollider->onCollisionEnter = [](Collider3D*) {
        std::cout << "Ship 1 collided with!\n";
        };
    shipCollider->onCollisionExit = [](Collider3D*) {
        std::cout << "Ship 1 stopped being collided with!\n";
        };





    PlayerController playerController(&Services::inputSystem->players[0]);

    //playerController.possess(playerShip.get());
    playerController.possess(playerShip.get());


    Services::lights->ambientColor = glm::vec3(0.05f); // slightly brighter ambient light for 3D models

    PointLight2D pointLight;
    pointLight.position = vec3(screenWidth / 2, screenHeight / 2, 100);
    pointLight.color = vec3(1.0f, 0.8f, 0.6f);
    pointLight.intensity = 6.0f;
    pointLight.range = 2000.0f;
    auto light = std::make_shared<PointLight2D>(pointLight);
    Services::lights->addLight(light);




    int framerateCap = 75;
    double frameInterval = 1.0f / framerateCap;
    double nextFrameTime = 0.0f;

    double lastTime = 0.0f;

    // Somewhere in initialization, after the EventBus is ready:
    if (Services::eventBus) {
        Services::eventBus->subscribe<DamageEvent>([](const DamageEvent& e) {
            printf("Damage dealt: %.2f to target %p (team %d)\n", e.amount, e.target, e.team);
            });
    }

    // Somewhere in your initialization code, e.g., main.cpp or a setup function
    if (Services::eventBus) {
        Services::eventBus->subscribe<DeathEvent>([](const DeathEvent& e) {
            printf("DeathEvent: target=%p, team=%d\n", e.target, e.team);
            });
    }


    while (!glfwWindowShouldClose(window))
    {
        double time = glfwGetTime();
        double dt = time - lastTime;
        lastTime = time;


        Services::inputSystem->update();

        playerController.update(dt);


        playerShip->update(dt);

        Services::projectiles->update(dt);

        if(Services::collisions3D)
			Services::collisions3D->update();
		if (Services::collisions)
            Services::collisions->update();

        Services::eventHandler->processEvents();
        Services::eventBus->clear();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        double currentTime = glfwGetTime();
        if (nextFrameTime <= currentTime) {
            double renderTime = currentTime;

            //glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, screenWidth, screenHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            glEnable(GL_DEPTH_TEST);


			modelRenderer.Draw(shipModel, view, projection, cameraPos);



            glDisable(GL_DEPTH_TEST);

            glfwSwapBuffers(window);

            currentTime = glfwGetTime();
            renderTime = currentTime - renderTime;
            nextFrameTime = currentTime + frameInterval - renderTime;
        }

        glfwPollEvents();
    }

    glDeleteProgram(shader3D);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}