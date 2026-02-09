#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL

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
#include "RenderSystem.h"
#include "UpdateSystem.h"

#include "BindingGenerator.h"
#include "Ship3D.h"
#include "ParticleEmitter.h"
#include "PlayerController.h"
#include "UprightActor3D.h"
#include "Guns3D.h"
#include "Weapon3D.h"
#include "GamepadObject.h"



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
    unsigned int spriteShader = createShader("shaders/sprite.vert", "shaders/sprite.frag");

    vec2 mapSize(480, 270);

    glm::vec3 cameraPos(mapSize.x/2, 280, mapSize.y/2+1);
    //glm::vec3 cameraPos(mapSize.x/2-20, 20, mapSize.y/2-20);
    glm::vec3 target(mapSize.x / 2, 0, mapSize.y / 2);
    glm::vec3 up(0, 0, -1);
    //glm::vec3 up(0, 1, 0);
    glm::mat4 view = glm::lookAt(cameraPos, target, up);

    glm::mat4 mainProjection = glm::perspective(
        //glm::radians(80.0f),
        glm::radians(55.0f),
        (float)screenWidth / (float)screenHeight,
        0.1f,
        5000.0f
    );

    glm::mat4 cameraProjection = glm::perspective(
        //glm::radians(80.0f),
        glm::radians(70.0f),
        (float)screenWidth / (float)screenHeight,
        0.1f,
        5000.0f
    );

    glm::mat4 projection = mainProjection;

    //SpriteRenderer spriteRenderer(rectShader);
    ModelRenderer modelRenderer(shader3D);

    SpriteRenderer spriteRenderer(spriteShader);


    // ----------------- new stuff -------------------

    auto playerInput = std::make_shared<PlayerInput>();
    auto cameraInput = std::make_shared<PlayerInput>();
    auto gamepadInput = std::make_shared<PlayerInput>();


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
        nullptr, // projectile system
        nullptr, // 2D collision system 
        new CollisionSystem3D,
        new LightManager,
        new RenderSystem(&modelRenderer, &spriteRenderer),
        new UpdateSystem
    );


    // ----------------- new stuff -------------------



    
    Services::assets->loadModel("coordinate", "res/models/coordinate.glb");
    Services::assets->loadModel("plane", "res/models/plane/plane.gltf");
    Services::assets->loadModel("drone", "res/models/drone/Drone.gltf");
    Services::assets->loadModel("map", "res/models/map/controller_visualiser_map.gltf");
    Services::assets->loadModel("gamepad", "res/models/controller/ctrl.gltf");

	Services::assets->loadTexture("smoke", "res/sprites/smoke.png");
    Services::assets->loadTexture("laser_shot", "res/sprites/enemy_projectile.png");

    Services::sound->loadSound("laser_shot", "res/audio/shoot.wav");


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



	bindGamepad(gamepad, cameraInput);
    bindGamepad(gamepad, playerInput);
    bindGamepadFull(gamepad, gamepadInput);
	

    Services::inputSystem->devices.push_back(keyboard);
    Services::inputSystem->devices.push_back(mouse);
    Services::inputSystem->devices.push_back(gamepad);
    Services::inputSystem->players.push_back(playerInput);
	Services::inputSystem->players.push_back(cameraInput);
    Services::inputSystem->players.push_back(gamepadInput);

	std::shared_ptr<Transform3D> worldOrigin = std::make_shared<Transform3D>();

	Services::updateSystem->addNode(worldOrigin);


	std::shared_ptr<UprightActor3D> camera = std::make_shared<UprightActor3D>();
	camera->position = cameraPos;
	camera->lookAt(target, up);
	worldOrigin->addChild(camera);

	Services::updateSystem->addNode(camera);

    /*std::shared_ptr<Model3D> coordinateModel = std::make_shared<Model3D>("coordinate");
    coordinateModel->scale = vec3(5.0f);
    coordinateModel->position = vec3(mapSize.x / 2, 0, mapSize.y / 2);
    Services::renderSystem->submit(coordinateModel);*/


    auto gamepadObject = std::make_shared<GamepadObject>(gamepadInput);
    gamepadObject->init();
    gamepadObject->rotation = glm::quat(vec3(-glm::half_pi<float>(), 0, 0));
    gamepadObject->position = vec3(mapSize.x / 2, -40, mapSize.y / 2 - 30);
    gamepadObject->scale = vec3(30.0f);
    
    Services::updateSystem->addNode(gamepadObject);

    std::shared_ptr<Ship3D> playerShip = std::make_shared<Ship3D>();
    playerShip->screenMax = mapSize;
    playerShip->respawn(vec3(mapSize.x/2, 0, mapSize.y/2));
    playerShip->scale = vec3(1.3f);

	std::shared_ptr<Model3D> shipModel = std::make_shared<Model3D>("plane");

    auto hardpoint = std::make_shared<Hardpoint3D>();
    hardpoint->position = vec3(0, 0, -8);
    playerShip->addHardpoint(hardpoint, 0);

    auto laserGun = std::make_shared<LaserGun>();
    laserGun->scale = vec3(5.0f);
    laserGun->shotInterval = 0.3f;
    hardpoint->attachWeapon(laserGun);


    
	Services::updateSystem->addNode(playerShip);
    Services::updateSystem->addNode(hardpoint);
    Services::updateSystem->addNode(laserGun);
	Services::renderSystem->submit(shipModel);

    playerShip->addChild(shipModel);

    worldOrigin->addChild(playerShip);

    playerShip->initialize();


    /*auto cord = std::make_shared<Model3D>("coordinate");
    laserGun->addChild(cord);
	Services::renderSystem->submit(cord);*/


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

    Services::eventBus->subscribe<ShootEvent>([](const ShootEvent& e) {
        // This code runs every time a ShootEvent is emitted
        std::cout << "Shoot event received!\n";
        std::cout << "Position: " << e.position.x << ", " << e.position.y << ", " << e.position.z << "\n";
        std::cout << "Direction: " << e.direction.x << ", " << e.direction.y << ", " << e.direction.z << "\n";
        std::cout << "Projectile Type: " << e.projectileType << "\n";
        std::cout << "Sound: " << e.soundName << ", Effect: " << e.effectName << "\n";

        // Example: spawn projectile
        // spawnProjectile(e.position, e.direction, e.projectileType);

        // Example: play sound
        // playSound(e.soundName);

        // Example: play muzzle flash effect
        // spawnEffect(e.effectName, e.position, e.direction);
    });
 
    PlayerController playerController(playerInput);

    //playerController.possess(playerShip.get());
    playerController.possess(playerShip.get());


	glm::vec3 ambientLight(0.05f); // default ambient light for 3D models
    Services::lights->ambientColor = ambientLight; // slightly brighter ambient light for 3D models
    glClearColor(ambientLight.x, ambientLight.y, ambientLight.z, 1.0f); // Postavljanje boje pozadine

    PointLight3D pointLight;
    pointLight.position = vec3(mapSize.x/2, 100, mapSize.y/2 - 50);
    pointLight.color = vec3(0.7f, 0.8f, 1.0f);
    pointLight.intensity = 10.0f;
    pointLight.range = 150.0f;
    auto light = std::make_shared<PointLight3D>(pointLight);
    Services::lights->addLight(light);

   



    PointLight3D pl;
    pl.color = vec3(0.9f, 0.5f, 0);
    pl.intensity = 3.0f;
    pl.range = 300.0f;

    pl.position = vec3(mapSize.x / 2, 40, 5.0f);
	auto plPtr1 = std::make_shared<PointLight3D>(pl);
    pl.position = vec3(mapSize.x / 2, 40, mapSize.y - 5.0f);
    auto plPtr2 = std::make_shared<PointLight3D>(pl);
	pl.position = vec3(5.0f, 40, mapSize.y / 2);
	auto plPtr3 = std::make_shared<PointLight3D>(pl);
    pl.position = vec3(mapSize.x - 5.0f, 40, mapSize.y / 2);
    auto plPtr4 = std::make_shared<PointLight3D>(pl);
    Services::lights->addLight(plPtr1);
    Services::lights->addLight(plPtr2);
    Services::lights->addLight(plPtr3);
	Services::lights->addLight(plPtr4);




	auto map = std::make_shared<Model3D>("map");

	map->position = vec3(mapSize.x / 2, 10.0f, mapSize.y / 2);
	map->rotation = quat(vec3(-glm::half_pi<float>(), 0, 0));
	//map->lookAt(map->position + vec3(0, -1, 0), vec3(0, 0, -1));
	map->scale = vec3(10.3f);
    worldOrigin->addChild(map);

	Services::renderSystem->submit(map);
	Services::updateSystem->addNode(map);

    for (int i = 0; i < 4; ++i) {
        PointLight3D pl;

        float offset = 22.75f * i;
        pl.position = vec3(-23.0f, -10.0f, -23.7f - offset);
        pl.color = vec3(1.0, 0.2f, 0.2f);
        pl.intensity = 3.0f;
        pl.range = 100.0f;

        PointLight3D pr;
        pr.position = vec3(23.0f, -10.0f, -23.7f - offset);
        pr.color = vec3(1.0, 0.2f, 0.2f);
        pr.intensity = 3.0f;
        pr.range = 100.0f;

		auto plPtr = std::make_shared<PointLight3D>(pl);
		auto prPtr = std::make_shared<PointLight3D>(pr);

        Services::lights->addLight(plPtr);
        Services::lights->addLight(prPtr);

		map->addChild(plPtr);
		map->addChild(prPtr);

        pl.markDirty();
        pr.markDirty();
    }



    int framerateCap = 75;
    double frameInterval = 1.0f / framerateCap;
    double nextFrameTime = 0.0f;
	double nextDebugTime = 0.0f;
	double debugInterval = 1.0f;

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

	bool cameraPressed = false;
	bool cameraPossessed = false;

    while (!glfwWindowShouldClose(window))
    {
        double time = glfwGetTime();
        double dt = time - lastTime;
        lastTime = time;


        Services::inputSystem->update();

        playerController.update(dt);


		Services::updateSystem->update(dt);


        if(Services::collisions3D)
			Services::collisions3D->update();


        Services::eventHandler->processEvents();
        Services::eventBus->clear();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
            if (!cameraPressed) {
				
                playerController.unpossess();
                if (!cameraPossessed) {
                    projection = cameraProjection;
                    playerController.possess(camera.get());
                }
                else {
                    projection = mainProjection;
					playerController.possess(playerShip.get());

                    camera->position = cameraPos;
                    camera->lookAt(target, up);
                }
				cameraPossessed = !cameraPossessed;
                std::cout << "Switching input mode. Camera control: " << cameraPossessed << std::endl;
                cameraPressed = true;
            }
        }
        else {
            cameraPressed = false;
        }

        double currentTime = glfwGetTime();
        if (nextFrameTime <= currentTime) {
            double renderTime = currentTime;

            //glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, screenWidth, screenHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            glEnable(GL_DEPTH_TEST);


            glm::vec3 eye = camera->getWorldPosition();

            Services::renderSystem->renderAll(
                glm::lookAt(
                    eye,
                    eye + camera->forward(),
                    camera->upWorld()
                ),
                projection,
                eye
            );




            glDisable(GL_DEPTH_TEST);

            glfwSwapBuffers(window);

            currentTime = glfwGetTime();
            renderTime = currentTime - renderTime;
            nextFrameTime = currentTime + frameInterval - renderTime;
        }

        if(nextDebugTime <= currentTime) {
            // Output debug info to console every second
            //printf("Player Pos: (%.2f, %.2f, %.2f) Vel: (%.2f, %.2f, %.2f)\n",
            //    playerShip->position.x, playerShip->position.y, playerShip->position.z,
            //    playerShip->physics.velocity.x, playerShip->physics.velocity.y, playerShip->physics.velocity.z);

			//printf("World origin children count: %d\n", worldOrigin->children.size());
			//std::cout << glm::to_string(Services::inputSystem->players[0].getPosition(Action::MousePositionHorizontal, Action::MousePositionVertical)) << std::endl;
            nextDebugTime = currentTime + debugInterval;
		}

        glfwPollEvents();
    }

    glDeleteProgram(shader3D);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}