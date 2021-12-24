#include <SFML/Window.hpp>
//#include <SFML/OpenGL.hpp>
#include <iostream>

#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"

using namespace std;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool isFullscreen = false;

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
	sf::Window window(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, settings);
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


	//Set up Shader
	Shader lightingShader("shader.vert", "shader.frag");
	Shader lightCubeShader("lightShader.vert", "lightShader.frag");
	float offset = 0.01f; 


	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	unsigned int texture1, texture2;
	unsigned int VBO, cubeVAO;
	//unsigned int EBO;

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);



	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// we only need to bind to the VBO, the container's VBO's data already contains the data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// set the vertex attribute 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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

					glGenVertexArrays(1, &cubeVAO);
					glBindVertexArray(cubeVAO);

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
			camera.ProcessKeyboard(FORWARD, deltaTime);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			camera.ProcessKeyboard(LEFT, deltaTime);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			camera.ProcessKeyboard(RIGHT, deltaTime);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			running = false;
		}
		

		sf::Vector2i center(window.getSize().x / 2, window.getSize().y / 2);
		sf::Vector2i deltaPos = sf::Mouse::getPosition(window) - center;

		if (window.hasFocus()) {
			sf::Mouse::setPosition(center, window);

			camera.ProcessMouseMovement((float)(deltaPos.x), (float)(-deltaPos.y));
		}
		
		// render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate shader when setting uniforms/drawing objects
		lightingShader.use();
		lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window.getSize().x / (float)window.getSize().y, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		// render the cube
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		// also draw the lamp object
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
		lightCubeShader.setMat4("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// end the current frame (internally swaps the front and back buffers)
		window.display();
	}

	// release resources...
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO);


	return 0;
}