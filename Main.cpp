#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <iostream>
#include "Util.h"
#include "SpriteRenderer.h"
#include <glm/gtc/type_ptr.hpp>
#include "GamepadInput.h"
#include "ButtonObject.h"
#include "AnalogStickObject.h"
#include "GamepadObject.h"


#include "glm/ext.hpp"

using namespace glm;

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

int screenWidth = 800;
int screenHeight = 800;



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




void onButtonEvent(int action) {
    if (action == GLFW_PRESS) {
        std::cout << "Button Pressed!" << std::endl;
    }
    else if (action == GLFW_RELEASE) {
        std::cout << "Button Released!" << std::endl;
    }
}

void onStickEvent(vec2 position) {
    std::cout << "Stick moved to position: (" << position.x << ", " << position.y << ")" << std::endl;
}


int main()
{
    // Inicijalizacija GLFW i postavljanje na verziju 3 sa programabilnim pajplajnom
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

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
    };

	GamepadObject gamepad(gamepadTex);
	gamepad.position = vec2(1000.0f, 400.0f);
	gamepad.scale = vec2(800.0f);
	gamepad.markDirty();

	gamepad.rightStick.onStickEvent = onStickEvent;


	analogStick.position = vec2(500.0f, 300.0f);
	analogStick.scale = vec2(100.0f, 100.0f);
    analogStick.markDirty();

	button.onButtonEvent = onButtonEvent;
	button.position = vec2(400.0f, 300.0f);
	button.scale = vec2(50.0f);
    analogStick.markDirty();

    

    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Postavljanje boje pozadine
    

	double rotationAngle = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

        if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
            // store previous state
            bool previousA = gamepadInput.buttonA;

            // update input
            if (gamepadInput.updateFromGLFW(GLFW_JOYSTICK_1)) {
                // check if state changed
                analogStick.setStickPosition(gamepadInput.thumbstickRight);
                if (gamepadInput.buttonA != previousA) {
                    button.setPressed(gamepadInput.buttonA);

                    // print only on change
                    printf("Button A: %s\n", gamepadInput.buttonA ? "Pressed" : "Released");
                }
            }
        } else {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);
                if (button.hitTest(vec2((float)mouseX, (float)mouseY))) {
					button.onMouseInput(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
                }

				Interactive* hitGamepad = gamepad.hitTest(vec2((float)mouseX, (float)mouseY));
                if (hitGamepad) {
					hitGamepad->onMouseInput(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
                }

            }
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
				button.onMouseInput(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
				gamepad.onMouseInput(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
            }

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                double mouseX, mouseY;
                glfwGetCursorPos(window, &mouseX, &mouseY);
                if (analogStick.hitTest(vec2((float)mouseX, (float)mouseY))) {
                    analogStick.onMouseInput(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
                }
				Interactive* hitGamepad = gamepad.hitTest(vec2((float)mouseX, (float)mouseY));
                if (hitGamepad) {
                    hitGamepad->onMouseInput(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
                }
                analogStick.onMouseMove(vec2((float)mouseX, (float)mouseY));
				gamepad.onMouseMove(vec2((float)mouseX, (float)mouseY));
				
			}
            else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
				analogStick.onMouseInput(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);
				gamepad.onMouseInput(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);
            }
        }


        analogStick.update(0.016);
        gamepad.update(0.016);




        /*position = position + 1;
        if (position > 1200)
			position = 1000;
		gamepad.position.x = (float)position;
		gamepad.markDirty();*/

        glClear(GL_COLOR_BUFFER_BIT); // Bojenje pozadine, potrebno kako pomerajući objekti ne bi ostavljali otisak

		//spriteRenderer.Draw(spriteTexture, vec2(800.0f, 540.0f), vec2(40.0f, 40.0f), radians(90.0f));

        analogStick.Draw(spriteRenderer);
		button.Draw(spriteRenderer);
		gamepad.Draw(spriteRenderer);
        glfwSwapBuffers(window); // Zamena bafera - prednji i zadnji bafer se menjaju kao štafeta; dok jedan procesuje, drugi se prikazuje.
        glfwPollEvents(); // Sinhronizacija pristiglih događaja
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}