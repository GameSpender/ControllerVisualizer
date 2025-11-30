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

#include "GamepadInput.h"
#include "GamepadObject.h"

#include "ship.h"
#include "LineVisualizer.h"

#include "Enemy.h"

#include "irrKlang/irrKlang.h"
#include "glm/ext.hpp"

#include "TextRenderer.h"

#include <format>

using namespace glm;

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

const GLFWvidmode* mode;
int screenWidth = 800;
int screenHeight = 800;


irrklang::ISoundEngine* soundEngine = irrklang::createIrrKlangDevice();



GamepadObject* gamepad = nullptr;
Ship* ship = nullptr;


void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath); // Učitavanje teksture
    glBindTexture(GL_TEXTURE_2D, texture); // Vezujemo se za teksturu kako bismo je podesili

    // Generisanje mipmapa - predefinisani različiti formati za lakše skaliranje po potrebi (npr. da postoji 32 x 32 verzija slike, ali i 16 x 16, 256 x 256...)
    //glGenerateMipmap(GL_TEXTURE_2D);
    

    // Podešavanje strategija za wrap-ovanje - šta da radi kada se dimenzije teksture i poligona ne poklapaju
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S - tekseli po x-osi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T - tekseli po y-osi

    // Podešavanje algoritma za smanjivanje i povećavanje rezolucije: nearest - bira najbliži piksel, linear - usrednjava okolne piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

unsigned preprocessTexture(const char* filepath) {
    unsigned texture;
    preprocessTexture(texture, filepath);
    return texture;
}


std::vector<Enemy> enemies;
std::vector<Projectile> enemyProjectiles;
unsigned enemyTex;
unsigned int enemyProjTex;

double nextSpawn = 10.0f;
double difficulty = 1.0f;
double spawning = false;
int killCount = 0;


void onMouseClick(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        vec2 mousePos = vec2((float)xpos, (float)ypos);
        if (gamepad) {
            Interactive* hitObject = gamepad->hitTest(mousePos);
            if (hitObject) {
				hitObject->onMouseInput(button, action);
            }
        }
	}
    if (action == GLFW_RELEASE) {
        if (gamepad)
            gamepad->onMouseInput(button, action);
    }
}


void onMouseMove(GLFWwindow* window, double xpos, double ypos) {
	static vec2 lastMousePos = vec2(0.0f);
    vec2 mousePos = vec2((float)xpos, (float)ypos);
    if(lastMousePos == mousePos)
		return;
    if (gamepad) {
		gamepad->onMouseMove(mousePos);
    }
	lastMousePos = mousePos;
}

std::vector<std::string> musicTracks = {
    "audio/track1.ogg",
    "audio/track2.ogg",
    "audio/track3.ogg",
    "audio/track4.ogg"
};

std::vector<std::string> musicQueue;
irrklang::ISound* currentMusic = nullptr;

void shuffleMusic() {
    musicQueue = musicTracks;
    std::shuffle(musicQueue.begin(), musicQueue.end(), std::mt19937(std::random_device{}()));
}

void playNextTrack(irrklang::ISoundEngine* engine) {
    if (musicQueue.empty()) {
        shuffleMusic(); // reshuffle when all tracks are used
    }

    std::string track = musicQueue.back();
    musicQueue.pop_back();

    if (currentMusic) {
        currentMusic->drop();
    }

    currentMusic = engine->play2D(track.c_str(), false, false, true);
}

void updateMusic(irrklang::ISoundEngine* engine) {
    if (!currentMusic || currentMusic->isFinished()) {
        playNextTrack(engine);
    }
}

void destroySound() {
    soundEngine->play2D("assets/explosion.wav");
}

void shootSound() {
    soundEngine->play2D("assets/shoot.wav");
}

void enemyShootSound() {
    soundEngine->play2D("assets/enemy_shoot.wav");
}

void enemyDeathSound() {
    soundEngine->play2D("assets/enemy_death.wav");
}

void onButtonEvent(int action) {
    if (action == GLFW_PRESS) {
        std::cout << "Button Pressed!" << std::endl;
    }
    else if (action == GLFW_RELEASE) {
        std::cout << "Button Released!" << std::endl;
    }
}

void onStickEvent(vec2 position) {
}

void onLeftThumbstick(vec2 position) {
    if (ship) {
        ship->setThrust(position);
    }
}

void onRightThumbstick(vec2 position) {
    if (ship) {
        ship->setDirection(position);
    }
}


bool shooting = false;
void onShootEvent(int action) {
    
    if (action == GLFW_PRESS) {
        shooting = true;
    }
    else if (action == GLFW_RELEASE) {
        shooting = false;
    }
}

void onEnableSpawning(int action) {
    if (action == GLFW_PRESS) {
        if (!spawning) {
            difficulty = 1.2f;
            spawning = true;
            nextSpawn = glfwGetTime() + 2.0f;
            killCount = 0;
        }
        if (spawning && ship->destroyed) {
            difficulty = 1.2f;
            spawning = false;
            nextSpawn = glfwGetTime() + 2.0f;


            ship->setRepaired();
            ship->position = vec2(mode->width * 0.5f, mode->height * 0.8f);
            ship->rotation = 0;
            ship->inertiaAngular = 0;
            ship->inertiaLinear = vec2(0);

            enemies.clear();
            enemyProjectiles.clear();

            enemies.shrink_to_fit();
            enemyProjectiles.shrink_to_fit();
        }
    }
}

int main()
{
    // Inicijalizacija GLFW i postavljanje na verziju 3 sa programabilnim pajplajnom
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    mode = glfwGetVideoMode(primaryMonitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Controller Visualizer", primaryMonitor, NULL);
    if (window == NULL) return endProgram("Failed to create window");
    glfwMakeContextCurrent(window);

    // Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW failed to initialize");

    // Potrebno naglasiti da program koristi alfa kanal za providnost
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    unsigned int colorShader = createShader("color.vert", "color.frag");

    glm::mat4 projection = glm::ortho(0.0f, (float)mode->width, (float)mode->height, 0.0f, -1.0f, 1.0f);
    glUseProgram(rectShader);
    glUniformMatrix4fv(glGetUniformLocation(rectShader, "uProjection"), 1, GL_FALSE, glm::value_ptr(projection));


	//unsigned spriteTexture;
	//preprocessTexture(spriteTexture, "res/cursor.png");
	SpriteRenderer spriteRenderer(rectShader);

	GamepadInput gamepadInput;

    ButtonObject button(
        preprocessTexture("res/button_idle.png"),
        preprocessTexture("res/button_pressed.png"),
        RECTANGLE
    );

    AnalogStickObject analogStick(
        preprocessTexture("res/stick_head.png"),
        preprocessTexture("res/stick_head_pressed.png")
	);


    GamepadTextures gamepadTex = {
    .gamepadBody = preprocessTexture("res/body.png"),
    .stickHead = preprocessTexture("res/stick_idle.png"),
    .stickHeadPressed = preprocessTexture("res/stick_pressed.png"),
    .buttonAIdle = preprocessTexture("res/a_idle.png"),
    .buttonAPressed = preprocessTexture("res/a_pressed.png"),
    .buttonBIdle = preprocessTexture("res/b_idle.png"),
    .buttonBPressed = preprocessTexture("res/b_pressed.png"),
    .buttonXIdle = preprocessTexture("res/x_idle.png"),
    .buttonXPressed = preprocessTexture("res/x_pressed.png"),
    .buttonYIdle = preprocessTexture("res/y_idle.png"),
    .buttonYPressed = preprocessTexture("res/y_pressed.png"),
    .dpadIdle = preprocessTexture("res/dpad_idle.png"),
    .dpadPressed = preprocessTexture("res/dpad_pressed.png"),
    .bumper = preprocessTexture("res/bumper.png"),
    .bumperPressed = preprocessTexture("res/bumper_pressed.png")
    };

	gamepad = new GamepadObject(gamepadTex);
	gamepad->position = vec2(mode->width * 0.5f, mode->height * 0.3);
	gamepad->scale = vec2(800.0f);
	gamepad->markDirty();

	ship = new Ship(preprocessTexture("res/ship.png"), preprocessTexture("res/projectile.png"), preprocessTexture("res/smoke.png"),
        mode->width, mode->height);
	ship->position = vec2(mode->width * 0.5f, mode->height * 0.8f);
	ship->scale = vec2(50.0f);
    ship->destroyedCallback = destroySound;
    ship->shootCallback = shootSound;
	ship->markDirty();

    gamepad->leftStick.onStickEvent = onLeftThumbstick;
	gamepad->rightStick.onStickEvent = onRightThumbstick;
    gamepad->buttonA.onButtonEvent = onShootEvent;
    gamepad->bumperRight.onButtonEvent = onShootEvent;
    gamepad->buttonY.onButtonEvent = onEnableSpawning;

    //enemy stuff
    enemyTex = preprocessTexture("res/enemy.png");
    enemyProjTex = preprocessTexture("res/enemy_projectile.png");
    

    LineVisualizer directionLine(
        glm::vec2(0, 0),
        glm::vec2(1, 0),
        glm::vec3(1, 0, 0)   // red
    );

    shuffleMusic();


    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Postavljanje boje pozadine

	glfwSetMouseButtonCallback(window, onMouseClick);

    TextRenderer titleText;
    TextRenderer killcountText;
    titleText.LoadFont("fonts/font.otf", 80, glm::vec3(0.8f, 0.5f, 0.1f)); // baked orange
    titleText.position = vec2(mode->width * 0.22f, mode->height * 0.7f);

    killcountText.LoadFont("fonts/font.otf", 170, glm::vec3(0.8f, 0.5f, 0.1f)); // baked orange
    killcountText.position = vec2(mode->width * 0.375f, mode->height * 0.85f);

    


    
    double lastTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        double time = glfwGetTime();
        double dt = time - lastTime;
        lastTime = time;

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

        if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
            if (gamepadInput.updateFromGLFW(GLFW_JOYSTICK_1)) {
				gamepad->updateFromInput(gamepadInput);
            }
        }

        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            onMouseMove(window, xpos, ypos);
        }


        gamepad->update(dt);

        if (shooting) ship->shoot();
        ship->update(dt);

        if (spawning && time > nextSpawn) {
            int spawnCount = difficulty * difficulty;

            for (int i = 0; i < spawnCount; i++) {

                // -------------------------------
                // 1. Choose a random direction
                // -------------------------------
                float angle = glm::linearRand(0.0f, glm::two_pi<float>());
                vec2 dir = vec2(glm::cos(angle), glm::sin(angle));

                // -------------------------------
                // 2. Choose random spawn distance
                // Spawn 600–1200 units from ship
                // -------------------------------
                float distance = glm::linearRand(600.0f, 1200.0f);
                vec2 spawnPos = ship->position + dir * distance;

                // -------------------------------
                // 3. Create enemy
                // -------------------------------
                enemies.emplace_back(Enemy(enemyTex, enemyProjTex, mode->width, mode->height));
                Enemy& e = enemies.back();

                e.scale = vec2(50.0f);
                e.position = spawnPos;

                // -------------------------------
                // 4. Assign random velocity
                // speed between 50–150
                // -------------------------------
                float speed = glm::linearRand(50.0f, 150.0f);

                // 50% chance to drift roughly toward the ship
                if (glm::linearRand(0.0f, 1.0f) < 0.5f) {
                    vec2 toShip = glm::normalize(ship->position - spawnPos);
                    e.velocity = toShip * speed;
                }
                else {
                    // Random direction
                    float moveAngle = glm::linearRand(0.0f, glm::two_pi<float>());
                    e.velocity = vec2(glm::cos(moveAngle), glm::sin(moveAngle)) * speed;
                }
            }

            difficulty += 0.1f;
            nextSpawn = time + 5.0f; // optional adaptive spawn rate
        }


        for (auto it = enemies.begin(); it != enemies.end(); ) {
            it->update(dt);
            if (it->canShoot()) {
                enemyProjectiles.push_back(it->shootAt(ship->position, ship->inertiaLinear, dt));
                enemyShootSound();
            }
                
            // Check if hit by any projectile
            for (auto& proj : ship->projectiles) {
                if (it->checkHit(proj)) {
                    it->health -= 25.0f;
                    proj.lifetime = -1.0f; // mark projectile for removal
                }
            }

            // Remove dead enemies
            if (it->health <= 0.0f) {
                enemyDeathSound();
                it = enemies.erase(it);
                killCount++;
            }
            else {
                ++it;
            }
        }

        for (auto p = enemyProjectiles.begin(); p != enemyProjectiles.end(); ) {
            p->update(dt);
            //cout << "projectile: " << to_string(p->position) << endl;
            if (ship->checkHit(*p)) {
                p->lifetime = 0;
                ship->setDestroyed();
            }
            if (p->lifetime < 0) {
                p = enemyProjectiles.erase(p);
            }
            else {
                ++p;
            }
        }

		//directionLine.start = ship->getWorldPosition();
		//directionLine.end = ship->getWorldPosition() + ship->forward() * 50.0f;
        

        glClear(GL_COLOR_BUFFER_BIT); // Bojenje pozadine, potrebno kako pomerajući objekti ne bi ostavljali otisak

        if (spawning) {

            killcountText.DrawText(spriteRenderer, format("{:04}", killCount));
        }
        else {
            titleText.DrawText(spriteRenderer, "Controller Visualizer");
        }
            

		gamepad->Draw(spriteRenderer);
		ship->Draw(spriteRenderer);
        for (auto& e : enemies)
            e.Draw(spriteRenderer);
        for (auto& p : enemyProjectiles)
            p.Draw(spriteRenderer);

        if(spawning)
            updateMusic(soundEngine);

		//directionLine.Draw(colorShader, mode->width, mode->height);
        glfwSwapBuffers(window); // Zamena bafera - prednji i zadnji bafer se menjaju kao štafeta; dok jedan procesuje, drugi se prikazuje.
        glfwPollEvents(); // Sinhronizacija pristiglih događaja
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}