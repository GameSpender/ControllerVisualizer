#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <random>
#include <iostream>
#include "Util.h"

#include "SpriteRenderer.h"
#include "ModelRenderer.h"
#include "LightManager.h"

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
#include "InputSystem.h"
#include "EventBus.h"
#include "ProjectileSystem.h"
#include "PlayerController.h"

#include "GamepadObject.h"
#include "ship.h"
#include "BindingGenerator.h"
#include "CollisionSystem.h"
#include "Guns.h"

#include "SimpleShootingAi.h"
#include "ShipFactory.h"

#include "Services.h"


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



    unsigned int rectShader = createShader("shaders/rect.vert", "shaders/rect.frag");
	unsigned int pulseShader = createShader("shaders/passthrough.vert", "shaders/pulse_effect.frag");
    unsigned int debugShader = createShader("shaders/color.vert", "shaders/color.frag");
	unsigned int Shader3D = createShader("shaders/3dColor.vert", "shaders/3dColor.frag");

    glm::mat4 projection = glm::ortho(0.0f, (float)mode->width, 0.0f, (float)mode->height, -100.0f, 1.0f);


    glm::vec3 cameraPos(screenWidth / 2, screenHeight / 2, 500.0f);
    glm::vec3 target(screenWidth / 2, screenHeight / 2, 0.0f);
    glm::vec3 up(0, 1, 0);

    glm::mat4 view = glm::lookAt(cameraPos, target, up);




    glUseProgram(rectShader);
    glUniformMatrix4fv(glGetUniformLocation(rectShader, "uProjection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(pulseShader);
    glUniform2f(glGetUniformLocation(pulseShader, "uScreenSize"), screenWidth, screenHeight);


	//PulseEffectRenderer pulseRenderer(pulseShader);

	//unsigned spriteTexture;
	//preprocessTexture(spriteTexture, "res/cursor.png");
	SpriteRenderer spriteRenderer(rectShader);
	ModelRenderer modelRenderer(Shader3D);

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

    PlayerInput playerInput;
    PlayerInput player2Input;
    ShipFactory factory;


    InputDevice keyboard;
    InputDevice gamepad;
    InputDevice gamepad2;
    InputDevice mouse;

	GamepadInput gamepadInput;
    
    
    Services::init(
        new InputSystem,
        new EventBus,
        new AssetManager,
		new SoundManager,
        new EventHandler,
		new ProjectileSystem,
        new CollisionSystem,
        new LightManager
    );


    // ----------------- new stuff -------------------

	Services::assets->loadTexture("grass", "res/grass.png");
	Services::assets->loadTexture("ship", "res/ship.png");
    Services::assets->loadTexture("enemy_ship_basic", "res/enemy.png");
	Services::assets->loadTexture("laser_shot", "res/projectile.png");
	Services::assets->loadTexture("bullet_shot", "res/bullet.png");
	Services::assets->loadTexture("enemy_shot", "res/enemy_projectile.png");

	//Services::assets->loadModel("test_box", "res/models/box01.glb");
    //Services::assets->loadModel("plane", "res/models/plane.glb");
    Services::assets->loadModel("plane", "res/models/plane/plane.gltf");

	Services::sound->loadSound("laser_shot", "assets/shoot.wav");
	Services::sound->loadSound("minigun_spool", "assets/minigun_spool.wav");
	Services::sound->loadSound("minigun_shoot", "assets/minigun_shoot.wav");
	Services::sound->loadSound("minigun_stop", "assets/minigun_stop.wav");
    Services::sound->loadSound("enemy_shot", "assets/enemy_shoot.wav");
    Services::sound->loadSound("enemy_death", "assets/enemy_death.wav");


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

    //bindGamepad(gamepad2, player2Input);

    

	Services::inputSystem->devices.push_back(keyboard);
	Services::inputSystem->devices.push_back(mouse);
	Services::inputSystem->devices.push_back(gamepad);
	Services::inputSystem->players.push_back(playerInput);
	Services::inputSystem->players.push_back(player2Input);



    std::shared_ptr<Ship> playerShip = std::make_shared<Ship>();
    playerShip->spriteName = "ship";
    playerShip->screenMax = vec2(screenWidth, screenHeight);
    playerShip->respawn(vec2(500, 500));
    playerShip->scale = vec2(50.0f);

    std::shared_ptr<Collider2D> shipCollider = std::make_shared<Collider2D>(Collider2D::ShapeType::Circle);
    shipCollider->mask = CollisionLayer::All;
    shipCollider->layer = CollisionLayer::Player;
    shipCollider->scale = vec2(0.8f);
    Services::collisions->addCollider(shipCollider);
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
    primaryHP->attachWeapon(laserGun);

    // Add hardpoint to ship and bind to action
    playerShip->addHardpoint(primaryHP, 0);


    std::shared_ptr<Ship> player2Ship = std::make_shared<Ship>();
    player2Ship->spriteName = "ship";
    player2Ship->screenMax = vec2(screenWidth, screenHeight);
    player2Ship->respawn(vec2(500, 500));
    player2Ship->scale = vec2(50.0f);

    std::shared_ptr<Collider2D> ship2Collider = std::make_shared<Collider2D>(Collider2D::ShapeType::Circle);
    ship2Collider->mask = CollisionLayer::All;
    ship2Collider->layer = CollisionLayer::Player;
    ship2Collider->scale = vec2(0.8f);
    Services::collisions->addCollider(ship2Collider);
    player2Ship->addChild(ship2Collider);

    // Create primary hardpoint and attach a weapon
    auto primaryHP2 = std::make_shared<Hardpoint>();
	primaryHP2->position = vec2(0, -1.0f);
    auto laserMinigun = std::make_shared<LaserMinigun>();
    primaryHP2->attachWeapon(laserMinigun);

    // Add hardpoint to ship and bind to action
    player2Ship->addHardpoint(primaryHP2, 0);

    auto enemyship = factory.spawnEnemy(vec2(1000, 700), 0);

    PlayerController playerController(&Services::inputSystem->players[0]);
    PlayerController player2Controller(&Services::inputSystem->players[1]);
	SimpleShootingAi aiController;

    //playerController.possess(playerShip.get());
	playerController.possess(playerShip.get());
	player2Controller.possess(player2Ship.get());
    aiController.possess(enemyship.get());


    Services::lights->ambientColor = glm::vec3(0.05f); // slightly brighter ambient light for 3D models

	PointLight2D pointLight;
	pointLight.position = vec3(screenWidth/2, screenHeight/2, 100);
	pointLight.color = vec3(1.0f, 0.8f, 0.6f);
    pointLight.intensity = 6.0f;
	pointLight.range = 2000.0f;
	auto light = std::make_shared<PointLight2D>(pointLight);
	Services::lights->addLight(light);

    


    std::shared_ptr<GamepadObject> gamepadVisualizer = std::make_shared<GamepadObject>();
    gamepadVisualizer->initHiearchy();
    gamepadVisualizer->position = vec2(screenWidth / 2, screenHeight * 0.3);

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

	float debugRotation = 0.0f;
    LineVisualizer line(vec2(0), vec2(0), vec3(255, 255, 0));

    while (!glfwWindowShouldClose(window))
    {
        double time = glfwGetTime();
        double dt = time - lastTime;
        lastTime = time;


        Services::inputSystem->update();
        
		playerController.update(dt);
		player2Controller.update(dt);

		aiController.setTarget(player2Ship->getWorldPosition(), player2Ship->getVelocity());
        //aiController.setTarget(enemyship->getWorldPosition(), enemyship->velocity);
		aiController.update(dt);

		//printf("Player 1 Pos: (%.2f, %.2f) Vel: (%.2f, %.2f)\n", playerShip->position.x, playerShip->position.y, playerShip->physics.velocity.x, playerShip->physics.velocity.y);
        
        playerShip->update(dt);
        player2Ship->update(dt);
        enemyship->update(dt);

        Services::projectiles->update(dt);

        
        gamepadInput.updateFromGLFW(gamepad.id);
        gamepadVisualizer->updateFromInput(gamepadInput);
        gamepadVisualizer->update(dt);

        Services::collisions->update();


        Services::eventHandler->processEvents();
        Services::eventBus->clear();

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
        
        debugRotation += static_cast<float>(dt) * 0.1f;


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
            

            spriteRenderer.Draw(Services::assets->getTexture(playerShip->spriteName)->id, playerShip->getWorldMatrix());

            spriteRenderer.Draw(Services::assets->getTexture(player2Ship->spriteName)->id, player2Ship->getWorldMatrix());

            spriteRenderer.Draw(Services::assets->getTexture(enemyship->spriteName)->id, enemyship->getWorldMatrix());

            Services::projectiles->render(spriteRenderer, *Services::assets);
            
            if (debugWeapon) {
                line.start = laserMinigun->getWorldPosition();
                line.end = line.start + laserMinigun->forwardWorld() * 1000.0f;
                line.Draw(debugShader, screenWidth, screenHeight);
            }

            glEnable(GL_DEPTH_TEST);


            glm::mat3 worldMat3 = playerShip->getWorldMatrix();

            // Convert to mat4
            glm::mat4 modelMat(1.0f);

            // Copy rotation + scale (upper-left 2x2)  
            modelMat[0][0] = worldMat3[0][0];
            modelMat[0][1] = worldMat3[0][1];
            modelMat[1][0] = worldMat3[1][0];
            modelMat[1][1] = worldMat3[1][1];

            // Add translation (third row of mat3 is translation in x/y)
            modelMat[3][0] = worldMat3[2][0]; // X
            modelMat[3][1] = worldMat3[2][1]; // Y

            // Set some depth (Z)
            modelMat[3][2] = 0.0f; // or whatever Z you want

			modelMat = glm::rotate(modelMat, glm::degrees(-90.0f), glm::vec3(1, 0, 0));
			//modelMat = glm::rotate(modelMat, debugRotation, glm::vec3(1, 0, 1));
			//modelMat = glm::scale(modelMat, glm::vec3(0.12f));
            modelMat = glm::scale(modelMat, glm::vec3(0.2f));
			//modelMat = glm::translate(modelMat, glm::vec3(1.0f, 1.0f, 0.0f));

            //// Now draw
            //modelRenderer.Draw(
            //    Services::assets->getModel("test_box"),
            //    modelMat,
            //    mat4(1.0f),
            //    projection
            //);

            modelRenderer.Draw(
                Services::assets->getModel("plane"),
                modelMat,
                mat4(1.0f),
                projection,
                cameraPos
			);



			glDisable(GL_DEPTH_TEST);



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