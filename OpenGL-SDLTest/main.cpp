#include <SFML/Window.hpp>
//#include <SFML/OpenGL.hpp>
#include <iostream>

#include "stb_image.h"
#include "Camera.h"
#include "Model.h"

using namespace std;

unsigned int loadTexture(const char *path);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 1920.0f / 2.0f;
float lastY = 1080.0f / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool isFullscreen = true;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

bool depthBufferMode = false;

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

	//stbi_set_flip_vertically_on_load(true);

	//Set up Shaders
	Shader shader("shader.vert", "shader.frag");
	Shader depthShader("shader.vert", "depthShader.frag");
	Shader screenShader("framebufferShader.vert", "framebufferShader.frag");

	Shader *currentShader = &shader;

	float offset = 0.01f; 

	float cubeVertices[] = {
		// positions          // texture Coords
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	float planeVertices[] = {
		// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};
	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates. NOTE that this plane is now much smaller and at the top of the screen
		// positions   // texCoords
		-0.3f,  1.0f,  0.0f, 1.0f,
		-0.3f,  0.7f,  0.0f, 0.0f,
		 0.3f,  0.7f,  1.0f, 0.0f,

		-0.3f,  1.0f,  0.0f, 1.0f,
		 0.3f,  0.7f,  1.0f, 0.0f,
		 0.3f,  1.0f,  1.0f, 1.0f
	};
	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	// plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	// screen quad VAO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// load textures
	// -------------
	unsigned int cubeTexture = loadTexture("container.jpg");
	unsigned int floorTexture = loadTexture("metal.png");

	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("texture1", 0);

	screenShader.use();
	screenShader.setInt("screenTexture", 0);

	// framebuffer configuration
	// -------------------------
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
				else if (event.key.code == sf::Keyboard::X) {
					depthBufferMode = !depthBufferMode;

					if (depthBufferMode)
						currentShader = &depthShader;
					else
						currentShader = &shader;
				}
				break;
			default:
				break;
			}

		}
		//handle mouse/keyboard input
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

		// render
		// ------
		// bind to framebuffer and draw scene as we normally would to color texture 
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		currentShader->use();
		glm::mat4 model = glm::mat4(1.0f);
		camera.Yaw += 180.0f;
		camera.ProcessCameraRotation(0, 0, false);
		glm::mat4 view = camera.GetViewMatrix();
		camera.Yaw -= 180.0f;
		camera.ProcessCameraRotation(0, 0, true);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window.getSize().x / (float)window.getSize().y, 0.1f, 100.0f);
		currentShader->setMat4("view", view);
		currentShader->setMat4("projection", projection);
		// cubes
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		currentShader->setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		currentShader->setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// floor
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		currentShader->setMat4("model", glm::mat4(1.0f));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// second render pass: draw as normal
		// ----------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		shader.setMat4("view", view);

		// cubes
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// floor
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		shader.setMat4("model", glm::mat4(1.0f));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// now draw the mirror quad with screen texture
		// --------------------------------------------
		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.

		screenShader.use();
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
		glDrawArrays(GL_TRIANGLES, 0, 6);

		window.display();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &quadVBO);


	return 0;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const *path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
