#include <SFML/Window.hpp>
//#include <SFML/OpenGL.hpp>
#include <iostream>

#include "stb_image.h"
#include "Camera.h"
#include "Model.h"

using namespace std;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool isFullscreen = true;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24; // Request a 24 bits depth buffer
	settings.stencilBits = 8;  // Request a 8 bits stencil buffer
	settings.antialiasingLevel = 2;  // Request 2 levels of antialiasing
	settings.majorVersion = 4;
	settings.minorVersion = 6;

	// create the window
	sf::Window window(sf::VideoMode(1920, 1080), "OpenGL", sf::Style::Fullscreen, settings);
	window.setVerticalSyncEnabled(true);
	glewInit();

	//hide & capture the mouse
	//window.setMouseCursorGrabbed(true);
	window.setMouseCursorVisible(false);

	// activate the window
	window.setActive(true);

	glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);
	//glDepthFunc(GL_EQUAL);
	//glDepthRange(1.0f, 0.0f);

	sf::Clock clock;
	sf::Time time;

	stbi_set_flip_vertically_on_load(true);

	//Set up Shader
	Shader ourShader("shader.vert", "shader.frag");
	float offset = 0.01f; 

	Model ourModel("backpack/backpack.obj");

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// run the main loop
	bool running = true;
	while (running)
	{
		//get delta time
		time = clock.getElapsedTime();
		float currentFrame = (float)(time.asMilliseconds()) / 1000.f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type) {
			case sf::Event::Closed:
				//end the program
				running = false;
				break;
			case sf::Event::Resized:
				// adjust the viewport when the window is resized
				glViewport(0, 0, event.size.width, event.size.height);
				break;
			case sf::Event::MouseWheelScrolled:
				camera.ProcessMouseScroll((float)event.mouseWheelScroll.delta);
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::F11) {
					window.setActive(false);
					if (!isFullscreen)
						window.create(sf::VideoMode(1920, 1080), "OpenGL", sf::Style::Fullscreen, settings);
					else
						window.create(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, settings);

					window.setVerticalSyncEnabled(true);

					//hide & capture the mouse
					//window.setMouseCursorGrabbed(true);
					window.setMouseCursorVisible(false);

					// activate the window
					window.setActive(true);

					isFullscreen = !isFullscreen;

					glViewport(0, 0, window.getSize().x, window.getSize().y);
				}
				break;
			default:
				break;
			}

		}
		//handle mouse/keyboard events
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			camera.ProcessCameraMovement(FORWARD, deltaTime, 1.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			camera.ProcessCameraMovement(FORWARD, deltaTime, -1.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			camera.ProcessCameraMovement(RIGHT, deltaTime, -1.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			camera.ProcessCameraMovement(RIGHT, deltaTime, 1.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			running = false;
		}
		
		sf::Vector2i center(window.getSize().x / 2, window.getSize().y / 2);
		sf::Vector2i deltaPos = sf::Mouse::getPosition(window) - center;

		if (window.hasFocus()) {
			sf::Mouse::setPosition(center, window);

			camera.ProcessCameraRotation((float)(deltaPos.x), (float)(-deltaPos.y));
		}

		if (sf::Joystick::isConnected(0)) {

			glm::vec2 camOffset(0.0f);
			camOffset.x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
			camOffset.y = -sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);

			if (glm::length(camOffset) / 100.0f >= 0.1f)
			{
				camera.ProcessCameraMovement(camOffset / 100.0f, deltaTime);
			}

			camOffset = { 0.0f, 0.0f };
			camOffset.x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::U);
			camOffset.y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::V);
			if (glm::length(camOffset) >= 1.0f)
			{
				camera.ProcessCameraRotation(camOffset.x / 10.0f, -camOffset.y / 10.0f);
			}

		}

		//lightPos.x = 1.0f + sin(currentFrame) * 2.0f;
		//lightPos.y = sin(currentFrame / 2.0f) * 1.0f;
		
		// render
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)(window.getSize().x) / (float)(window.getSize().y), 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);

		// end the current frame (internally swaps the front and back buffers)
		window.display();
	}

	// release resources...
	//glDeleteBuffers(1, &EBO);


	return 0;
}