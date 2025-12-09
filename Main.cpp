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
#include "SoundManager.h"
#include "EventHandler.h"

#include "GamepadObject.h"
#include "ship.h"
#include "BindingGenerator.h"
#include "CollisionSystem.h"

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
    unsigned int debugShader = createShader("shaders/color.vert", "shaders/color.frag");

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
    EventBus eventBus;
    AssetManager assetManager;
    SoundManager soundManager;
    EventHandler eventHandler(&eventBus);
    ProjectileSystem projectileSystem;
    CollisionSystem collisionSystem;

    PlayerInput playerInput;
    PlayerInput player2Input;

    InputDevice keyboard;
    InputDevice gamepad;
    InputDevice gamepad2;
    InputDevice mouse;

    GamepadInput gamepadInput;
    

    // ----------------- new stuff -------------------

    bool debugWeapon = false;

    assetManager.loadTexture("grass", "res/grass.png");
    assetManager.loadTexture("ship", "res/ship.png");
    assetManager.loadTexture("laser_shot", "res/projectile.png");
    assetManager.loadTexture("bullet_shot", "res/bullet.png");

    assetManager.loadTexture("gamepad_body", "res/body.png");
    assetManager.loadTexture("button_A", "res/a_idle.png");
    assetManager.loadTexture("button_A_pressed", "res/a_pressed.png");
    assetManager.loadTexture("button_B", "res/B_idle.png");
    assetManager.loadTexture("button_B_pressed", "res/B_pressed.png");
    assetManager.loadTexture("button_X", "res/X_idle.png");
    assetManager.loadTexture("button_X_pressed", "res/X_pressed.png");
    assetManager.loadTexture("button_Y", "res/Y_idle.png");
    assetManager.loadTexture("button_Y_pressed", "res/Y_pressed.png");
    assetManager.loadTexture("stick_head", "res/stick_head.png");
    assetManager.loadTexture("stick_head_pressed", "res/stick_head_pressed.png");
    assetManager.loadTexture("dpad", "res/dpad_idle.png");
    assetManager.loadTexture("dpad_pressed", "res/dpad_pressed.png");
    assetManager.loadTexture("bumper", "res/bumper.png");
    assetManager.loadTexture("bumper_pressed", "res/bumper_pressed.png");


    soundManager.loadSound("laser_shot", "assets/shoot.wav");
    soundManager.loadSound("minigun_spool", "assets/minigun_spool.wav");
    soundManager.loadSound("minigun_shoot", "assets/minigun_shoot.wav");
	soundManager.loadSound("minigun_stop", "assets/minigun_stop.wav");

    eventHandler.init(&soundManager, &projectileSystem);

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
    gamepad2 = {
        .type = DeviceType::Gamepad,
        .id = 1
    };


    bindKeyboardAndMouse(keyboard, mouse, playerInput, screenHeight);

    bindGamepad(gamepad, player2Input);

    bindGamepad(gamepad2, player2Input);

    

    inputSystem.devices.push_back(keyboard);
    inputSystem.devices.push_back(mouse);
    inputSystem.devices.push_back(gamepad);
    inputSystem.devices.push_back(gamepad2);
    inputSystem.players.push_back(playerInput); 
    inputSystem.players.push_back(player2Input);


    std::shared_ptr<Ship> playerShip = std::make_shared<Ship>();
    playerShip->spriteName = "ship";
    playerShip->screenMax = vec2(screenWidth, screenHeight);
    playerShip->init(&inputSystem.players.front(), &eventBus);
    playerShip->respawn(vec2(500, 500));
    playerShip->scale = vec2(50.0f);

    std::shared_ptr<Collider2D> shipCollider = std::make_shared<Collider2D>(Collider2D::ShapeType::Circle);
    shipCollider->mask = CollisionLayer::All;
    shipCollider->layer = CollisionLayer::Player;
    shipCollider->scale = vec2(0.8f);
    collisionSystem.addCollider(shipCollider);
    playerShip->addChild(shipCollider);

    shipCollider->onCollisionEnter = [](Collider2D*) {
        std::cout << "Ship 1 collided with!\n";
        };
    shipCollider->onCollisionExit = [](Collider2D*) {
        std::cout << "Ship 1 stopped being collided with!\n";
        };


    // Create primary hardpoint and attach a weapon
    auto primaryHP = std::make_shared<Hardpoint>();
	primaryHP->position = vec2(0, -0.9f);
    auto laserGun = std::make_shared<LaserGun>();
    laserGun->initWeapon(&projectileSystem, &eventBus, &collisionSystem);
    primaryHP->attachWeapon(laserGun);

    // Add hardpoint to ship and bind to action
    playerShip->addHardpoint(primaryHP, Action::Shoot);


    std::shared_ptr<Ship> player2Ship = std::make_shared<Ship>();
    player2Ship->spriteName = "ship";
    player2Ship->screenMax = vec2(screenWidth, screenHeight);
    player2Ship->init(&inputSystem.players[1], &eventBus);
    player2Ship->respawn(vec2(500, 500));
    player2Ship->scale = vec2(50.0f);

    std::shared_ptr<Collider2D> ship2Collider = std::make_shared<Collider2D>(Collider2D::ShapeType::Circle);
    ship2Collider->mask = CollisionLayer::All;
    ship2Collider->layer = CollisionLayer::Player;
    ship2Collider->scale = vec2(0.8f);
    collisionSystem.addCollider(ship2Collider);
    player2Ship->addChild(ship2Collider);

    // Create primary hardpoint and attach a weapon
    auto primaryHP2 = std::make_shared<Hardpoint>();
	primaryHP2->position = vec2(0, -1.0f);
    auto laserMinigun = std::make_shared<LaserMinigun>();
    laserMinigun->initWeapon(&projectileSystem, &eventBus, &collisionSystem);
    primaryHP2->attachWeapon(laserMinigun);

    // Add hardpoint to ship and bind to action
    player2Ship->addHardpoint(primaryHP2, Action::Shoot);

    TestActor testActor("grass");
    testActor.init(&inputSystem.players.front(), &eventBus);
    testActor.position = vec2(100, 100);
    testActor.scale = vec2(50);


    std::shared_ptr<GamepadObject> gamepadVisualizer = std::make_shared<GamepadObject>();
    gamepadVisualizer->initHiearchy();
    gamepadVisualizer->position = vec2(screenWidth / 2, screenHeight * 0.3);

    int framerateCap = 75;
    double frameInterval = 1.0f / framerateCap;
    double nextFrameTime = 0.0f;

    double lastTime = 0.0f;

    LineVisualizer line(vec2(0), vec2(0), vec3(255, 255, 0));

    while (!glfwWindowShouldClose(window))
    {
        double time = glfwGetTime();
        double dt = time - lastTime;
        lastTime = time;


        inputSystem.update();
        
        testActor.update(dt);
        playerShip->update(dt);
        player2Ship->update(dt);

        projectileSystem.update(dt);

        gamepadInput.updateFromGLFW(gamepad.id);
        gamepadVisualizer->updateFromInput(gamepadInput);
        gamepadVisualizer->update(dt);

        collisionSystem.update();

        eventHandler.processEvents();

        eventBus.clear();

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);



        double currentTime = glfwGetTime();
        if (nextFrameTime <= currentTime) {
            double renderTime = currentTime;

            //glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, screenWidth, screenHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            titleText.DrawText(spriteRenderer, "Controller Visualizer");

            {
                auto size = vec2(100, 100);
                auto pos = vec2(screenWidth - 60, 60);
                spriteRenderer.Draw(signatureTexture, pos, size);
            }
            
            spriteRenderer.Draw(assetManager.getTexture(gamepadVisualizer->spriteName)->id, gamepadVisualizer->getWorldMatrix());
            for (auto& child : gamepadVisualizer->children)
            {
                if (auto actor = std::dynamic_pointer_cast<Actor2D>(child))
                {
                    spriteRenderer.Draw(
                        assetManager.getTexture(actor->spriteName)->id,
                        actor->getWorldMatrix()
                    );
                }
            }


            spriteRenderer.Draw(assetManager.getTexture(testActor.spriteName)->id, testActor.getWorldMatrix());

            spriteRenderer.Draw(assetManager.getTexture(playerShip->spriteName)->id, playerShip->getWorldMatrix());

            spriteRenderer.Draw(assetManager.getTexture(player2Ship->spriteName)->id, player2Ship->getWorldMatrix());

            if (debugWeapon) {
                line.start = laserMinigun->getWorldPosition();
                line.end = line.start + laserMinigun->forwardWorld() * 1000.0f;
                line.Draw(debugShader, screenWidth, screenHeight);
            }
            


            projectileSystem.render(spriteRenderer, assetManager);
            

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