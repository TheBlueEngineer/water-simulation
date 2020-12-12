#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"

void processKeyboardInput ();

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool ok = true; //test mouse movement
float lastX = 0.0f;
float lastY = 0.0f;

Window window("Wave simulation - Alexandru-Cosmin GRIGORE & Stefan SURDU", 800, 800);
Camera camera;

//CONTROL VARIABLES FOR LIGHT - THE POSITION, THE SPEED AND COLOR OF THE SUN'S LIGHT
glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-0.0f, 100.0f, -0.0f);
float sunAmbientStrength = 0.0f;
float sunPosition = 0.0f;
float sunHeight = 400.0f;
float sunDelay = 8.0f;

//CONTROL VARIABLES FOR DIRECTIONAL WAVE - THESE VARIABLES CONTROL THE BEHAVIOUR OF THE DIRECTIONAL WAVE
float amplitude_directional = 8.0f;
float frequency_directional = 1.2f; 
float wavespeed_directional = 2.2f;
float phase_directional = wavespeed_directional * frequency_directional;
float wavedir_x = 0.5f;
float wavedir_y = 0.0f;
float wavedir_z = 0.5f;
glm::vec3 waveDirection = glm::vec3(wavedir_x, wavedir_y, wavedir_z);

//CONTROL VARIABLES FOR CIRCULAR WAVE - THESE VARIABLES CONTROL THE BEHAVIOUR OF THE CIRCULAR WAVE
float amplitude_circular = 8.0f;
float frequency_circular = 1.2f;
float wavespeed_circular = 2.2f;
float phase_circular = wavespeed_circular * frequency_circular;
glm::vec3 center = glm::vec3(1.0f, 0.0f, 1.0f);

//CONTROL VARIABLES FOR BOTH TYPE OF WAVES - THESE VARIABLES CONTROL THE BEHAVIOUR OF BOTH WAVES
float sharpcontrol = 0.17f;
float stepness = 0.17f;
int enableDirectional = 0;
int enableCircular = 0;

//CONTROL VARIABLES FOR DOLPHIN - CONTROL THE ANGLE, ANGLE DIRECTION FOR THE DOLPHIN OBJECTS
float dolphin_angle = 0.0f;
float dolphin_angle_y = 90;
float dolphin_angle_direction = -1.0f;

//VARIABLES FOR BOXES - THE NUMBER OF BOXES IN THE SCENE, THE ARRAY WITH BOXES AND THEIR POSITIONS;
int const noBoxes = 8;
Mesh boxes[noBoxes];
glm::vec3 boxesPosition[noBoxes];

//RADIAN FUNCTION FOR SINE, COSINE FUNCTIONS
float radian(float angle) {
	return (angle * 3.14f) / 180;
}

//GERSTNER FUNCTION FOR WAVES
glm::vec3 Gerstner(float Amplitude, float Frequency, float Phase, float Time, glm::vec3 Position, glm::vec3 Direction, float SharpControl, float Stepness) {
	glm::vec3 result = glm::vec3(0.0f, 0.0f, 0.0f);

	float dotProduct = glm::dot(glm::vec2(Direction.x, Direction.z), glm::vec2(Position.x, Position.z));
	float innerFunction = Frequency * SharpControl * dotProduct + Phase * Time;
	float cosine = cos(innerFunction);
	float sine = sin(innerFunction);

	result.x = Position.x + Stepness * Amplitude * Direction.x * cosine;
	result.y = Amplitude * sine;
	result.z = Position.z + Stepness * Amplitude * Direction.z * cosine;
	return result;
}

//CIRCULAR DIRECTION
glm::vec3 CircularDirection(glm::vec3 Position, glm::vec3 Center) {

	glm::vec3 Direction = glm::vec3(Center.x, 0.0f, Center.z) - glm::vec3(Position.x, 0.0f, Position.z);
	float DirectionMagnitude = sqrt(pow(Direction.x, 2) + pow(Direction.z, 2));

	glm::vec3 resultDirection = Direction / DirectionMagnitude;

	return resultDirection;
}

int main()
{
	glClearColor(0.2f, 0.8f, 1.0f, 1.0f);

	//BUILDING AND COMPILING SHADERS - (VERTEX SHADER NAME, FRAGMENT SHADER NAME);
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");
	Shader waterShader("Shaders/water_vertex_shader.glsl", "Shaders/water_fragment_shader.glsl");
	Shader dolphinShader("Shaders/dolphin_vertex_shader.glsl", "Shaders/dolphin_fragment_shader.glsl");
	Shader skyShader("Shaders/skybox_vertex_shader.glsl", "Shaders/skybox_fragment_shader.glsl");
	Shader boxShader("Shaders/box_vertex_shader.glsl", "Shaders/box_fragment_shader.glsl");
	
	//TEXTURES - LOAD THE BMP TEXTURES(INSERT PATH HERE);
	GLuint tex = loadBMP("Resources/Textures/wood.bmp");
	GLuint tex2 = loadBMP("Resources/Textures/water.bmp");
	GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");
	GLuint tex4 = loadBMP("Resources/Textures/oceans.bmp");
	GLuint tex5 = loadBMP("Resources/Textures/dolphin.bmp");
	GLuint tex6 = loadBMP("Resources/Textures/skybox2.bmp");
	glEnable(GL_DEPTH_TEST);

	//INSERT THE ABOVE TEXTURES IN A TEXTURE VECTOR
	std::vector<Texture> textures;
	textures.push_back(Texture());
	textures[0].id = tex;
	textures[0].type = "texture_diffuse";

	std::vector<Texture> textures2;
	textures2.push_back(Texture());
	textures2[0].id = tex2;
	textures2[0].type = "texture_diffuse";

	std::vector<Texture> textures3;
	textures3.push_back(Texture());
	textures3[0].id = tex3;
	textures3[0].type = "texture_diffuse";

	std::vector<Texture> textures4;
	textures4.push_back(Texture());
	textures4[0].id = tex4;
	textures4[0].type = "texture_diffuse";

	std::vector<Texture> textures5;
	textures5.push_back(Texture());
	textures5[0].id = tex5;
	textures5[0].type = "texture_diffuse";

	std::vector<Texture> textures6;
	textures6.push_back(Texture());
	textures6[0].id = tex6;
	textures6[0].type = "texture_diffuse";

	//WE CREATE OBJECTS USING A MESH LOADER - WE LOAD THE OBJ FILE AND THE ADJACENT TEXTURE
	MeshLoaderObj loader;

	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh plane = loader.loadObj("Resources/Models/plane1.obj", textures4);
	Mesh dolphin = loader.loadObj("Resources/Models/dolphin.obj", textures5);
	Mesh skybox = loader.loadObj("Resources/Models/skyboxv2.obj", textures6);

	//WE PLACE THE BOXES IN SCENE AND LOAD THE OBJECTS
	for ( int j = 0; j < noBoxes; j++) {
		boxesPosition[j] = glm::vec3( rand()%400 - 200, 0.0f, rand()%400 - 200);
		boxes[j] = loader.loadObj("Resources/Models/cube.obj", textures);
	}

	//CHECK IF WE CLOSE THE WINDOW
	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		lastX = window.getWidth() / 2.0f;
		lastY = window.getHeight() / 2.0f;

		//CHECK IF THERE IS A KEYBOARD INPUT
		processKeyboardInput();

		//TEST MOUSE INPUT 
		if (window.isMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
			std::cout << "Pressing mouse button" << std::endl;
		}

		//**********************
		//** CODE FOR THE SUN **
		//**********************

		//USE SPECIFIC SHADER: sunShader;
		sunShader.use();

		//DEFINE THE MVP AND PROJECTION MATRIX
		glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());
		//GET UNIFORM LOCATION OF MVP 
		GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");
		
		//DETERMINE THE POSITION OF SUN, IT WILL HAVE A CIRCULAR MOVEMENT
		lightPos = glm::vec3(sunHeight * cos(glfwGetTime() / sunDelay), sunHeight * sin(glfwGetTime() / sunDelay), 0.0f);
		
		//THE AMBIENT STRENGTH OF THE LIGHT SOURCE IS SINUSOIDAL TO REFLECT THE CIRCULAR MOVEMENT OF THE SUN
		float sunAmbientStrength = sin(glfwGetTime() / 8);

		//WHEN THE SUN IS BELOW THE WATER LEVEL IT'S AMBIENT STRENGTH IS 0
		if (sunAmbientStrength < 0) {
			sunAmbientStrength = 0;
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0); //INITIALIZE THE MODEL MATRIX  
		ModelMatrix = glm::translate(ModelMatrix, lightPos); //PLACE THE LIGHT SOURCE AT "LIGHTPOS" VECTOR
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix; //MODEL VIEW PROJECTION
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]); //pass MVP matrix as MVP uniform in sunShader

		sun.draw(sunShader); //draw the sun using the sunShader
		//---------------------------------------------------------------------------------------------------END

		//********************************************
		//** ADD BOX OBJECT with color of the light **
		//********************************************
		
		//USE SPECIFIC SHADER: boxShader;
		boxShader.use();
		
		//GENERATE A NUMBER OF BOXES AND PLACE THEM AT RANDOM POSITIONS
		for (unsigned int i = 0; i < noBoxes; i++) {
			//CONTROL THE MVP AND MODEL
			GLuint MatrixID_box = glGetUniformLocation(boxShader.getId(), "MVP");
			GLuint ModelMatrixID_box = glGetUniformLocation(boxShader.getId(), "model");

			//CONTROL VARIABLES FOR LIGHT
			GLuint lightColor_box = glGetUniformLocation(boxShader.getId(), "lightColor");
			GLuint lightPos_box = glGetUniformLocation(boxShader.getId(), "lightPos");
			GLuint viewPos_box = glGetUniformLocation(boxShader.getId(), "viewPos");
			GLuint sunAmbientStrength_box = glGetUniformLocation(boxShader.getId(), "sunAmbientStrength");

			//CONTROL VARIABLES FOR DIRECTIONAL WAVE
			GLuint amplitude_box_dir = glGetUniformLocation(boxShader.getId(), "amplitude_dir");
			GLuint frequency_box_dir = glGetUniformLocation(boxShader.getId(), "frequency_dir");
			GLuint time_box_dir = glGetUniformLocation(boxShader.getId(), "time_dir");
			GLuint phase_box_dir = glGetUniformLocation(boxShader.getId(), "phase_dir");
			GLuint direction_box_dir = glGetUniformLocation(boxShader.getId(), "direction_dir");

			//CONTROL VARIABLES FOR BOTH TYPE OF WAVES
			GLuint sharpcontrol_box = glGetUniformLocation(boxShader.getId(), "sharpcontrol");
			GLuint stepness_box = glGetUniformLocation(boxShader.getId(), "stepness");
			GLuint center_box = glGetUniformLocation(boxShader.getId(), "center");
			GLuint enableDirectional_box = glGetUniformLocation(boxShader.getId(), "enableDirectional");
			GLuint enableCircular_box = glGetUniformLocation(boxShader.getId(), "enableCircular");

			//COMPUTE THE WAVE SURF
			glm::vec3 surfDirectional = Gerstner(amplitude_directional, frequency_directional, phase_directional, glfwGetTime(), boxesPosition[i], waveDirection, stepness, sharpcontrol);
			glm::vec3 CircularDir = CircularDirection(boxesPosition[i], center);
			glm::vec3 surfCircular = Gerstner(amplitude_circular, frequency_circular, phase_circular, glfwGetTime(), boxesPosition[i], CircularDir, stepness, sharpcontrol);
			glm::vec3 resultSurfVector = boxesPosition[i] + glm::vec3(0.0f, surfDirectional.y, 0.0f) * (float)(enableDirectional) + glm::vec3(0.0f, surfCircular.y, 0.0f) * (float)(enableCircular);
			
			//MVP + MODEL INITIALIZATION
			ModelMatrix = glm::mat4(1.0); //classic 4D matrix;
			ModelMatrix = glm::translate( ModelMatrix, resultSurfVector); //set position of box 
			ModelMatrix = glm::scale( ModelMatrix, glm::vec3( 2.0f, 2.0f, 2.0f) );
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix; //MVP matrix

			//PASS UNIFORM MVP AND MODEL MATRICES
			glUniformMatrix4fv(MatrixID_box, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID_box, 1, GL_FALSE, &ModelMatrix[0][0]);

			//PASS UNIFORM VARIABLES FOR LIGHT PROCESSING ON THIS OBJECT
			glUniform3f(lightColor_box, lightColor.x, lightColor.y, lightColor.z);
			glUniform3f(lightPos_box, lightPos.x, lightPos.y, lightPos.z);
			glUniform3f(viewPos_box, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
			glUniform1f(sunAmbientStrength_box, sunAmbientStrength);

			//PASS THE UNIFORM VARIABLES TO THE SHADER FOR DIRECTIONAL WAVE SURF OF THE BOXES
			glUniform1f(amplitude_box_dir, amplitude_directional);
			glUniform1f(frequency_box_dir, frequency_directional);
			glUniform1f(time_box_dir, glfwGetTime());
			glUniform1f(phase_box_dir, phase_directional);

			//PASS UNIFORM VARIABLES FOR BOTH TYPES OF WAVE SURF PROCESSING
			glUniform3f(direction_box_dir, wavedir_x, wavedir_y, wavedir_z);

			glUniform1f(sharpcontrol_box, sharpcontrol);
			glUniform1f(stepness_box, stepness);
			glUniform1i(enableDirectional_box, enableDirectional);
			glUniform1i(enableCircular_box, enableCircular);

			boxes[i].draw(boxShader); //draw the box using the shader
		}
		//---------------------------------------------------------------------------------------------------END

		//***************************
		//** CREATE OBJECT DOLPHIN **
		//***************************

		//USE SPECIFIC SHADER: dolphinShader;
		dolphinShader.use();

		//GET THE UNIFORM LOCATION FOR THE MVP + MODEL
		GLuint MatrixID_dolphin = glGetUniformLocation(dolphinShader.getId(), "MVP");
		GLuint ModelMatrixID_dolphin = glGetUniformLocation(dolphinShader.getId(), "model");
		//GET THE UNIFORM LOCATION OF THE LIGHT VARIABLES
		GLuint lightColor_dolphin = glGetUniformLocation(dolphinShader.getId(), "lightColor");
		GLuint lightPos_dolphin = glGetUniformLocation(dolphinShader.getId(), "lightPos");
		GLuint viewPos_dolphin = glGetUniformLocation(dolphinShader.getId(), "viewPos");
		GLuint sunAmbientStrength_dolphin = glGetUniformLocation(dolphinShader.getId(), "sunAmbientStrength");

		//COMPUTE THE DOLPHIN'S POSITION ON THE WATER GRID
		//CONTROL VARIABLES
		float dolphin_jump_freq = 6.0f; //how many jumps he does on a full XZ plane rotation
		float dolphin_angle_increment = 0.02f; //angle incrementation value for XZ rotation
		float dolphin_circle_radius = 100.0f; //radius of the XZ rotation circle
		float dolphin_jump_height = 25.0f; //height of the Y jump
		float dolphin_jump_depth = 10.0f; //depth of the Y dive

		//COMPUTE THE DIRECTION OF THE DOLPHIN TO SIMULATE A SINUSOIDAL TRAJECTORY FOR THE DOLPHIN
		if (dolphin_angle > 360)
			dolphin_angle = 0.0f;
		else
			dolphin_angle += dolphin_angle_increment;
		
		if (dolphin_angle_y > 90) {
			dolphin_angle_direction = -1.0f;
			dolphin_angle_y = 90;
		}
		if (dolphin_angle_y < -90) {
			dolphin_angle_direction = 1.0f;
			dolphin_angle_y = -90;
		}
		//COMPUTE THE DOLPHIN'S POSITION AND ANGLE IN THE SCENE
		dolphin_angle_y += dolphin_angle_direction * dolphin_angle_increment * dolphin_jump_freq;
		
		float dolphin_position_x = dolphin_circle_radius * cos( radian(dolphin_angle) );
		float dolphin_position_y = -dolphin_jump_depth + dolphin_jump_height * sin( radian(dolphin_jump_freq * dolphin_angle) );
		float dolphin_position_z = dolphin_circle_radius * sin( radian(dolphin_angle) );

		glm::vec3 axisY = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 axisX = glm::vec3(-1.0f, 0.0f, 0.0f);
		glm::vec3 dolphin_position = glm::vec3(dolphin_position_x, dolphin_position_y, dolphin_position_z);
		
		ModelMatrix = glm::mat4(1.0); //classic 4D matrix;
		ModelMatrix = glm::translate(ModelMatrix, dolphin_position); //set position of the dolphin 
		ModelMatrix = glm::rotate(ModelMatrix, dolphin_angle, axisY); //rotate in XZ plane, Y axis
		ModelMatrix = glm::rotate(ModelMatrix, dolphin_angle_y, axisX); //rotate in YZ plane, X axis
		ModelMatrix = glm::scale(ModelMatrix, 15.0f, 15.0f, 15.0f); //scale the original model 15 times
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix; //MVP matrix

		//PASS UNIFORM MATRICES FOR MVP AND MODEL
		glUniformMatrix4fv(MatrixID_dolphin, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID_dolphin, 1, GL_FALSE, &ModelMatrix[0][0]);
		//PASS THE UNIFORM VARIABLES FOR THIS OBJECT
		glUniform3f(lightColor_dolphin, lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(lightPos_dolphin, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewPos_dolphin, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		glUniform1f(sunAmbientStrength_dolphin, sunAmbientStrength);

		dolphin.draw(dolphinShader); //DRAW THE DOLPHIN USING HIS OWN SHADER

		//************************
		//** WATER OBJECT/PLANE **
		//************************

		//USE SPECIFIC SHADER: waterShader;
		waterShader.use();

		//MVP AND MODEL FOR WATER
		GLuint MatrixID_water = glGetUniformLocation(waterShader.getId(), "MVP");
		GLuint ModelMatrixID_water = glGetUniformLocation(waterShader.getId(), "model");
		//LIGHT VARIABLES FOR WATER
		GLuint lightColor_water = glGetUniformLocation(waterShader.getId(), "lightColor");
		GLuint lightPos_water = glGetUniformLocation(waterShader.getId(), "lightPos");
		GLuint viewPos_water = glGetUniformLocation(waterShader.getId(), "viewPos");
		GLuint sunAmbientStrength_water = glGetUniformLocation(waterShader.getId(), "sunAmbientStrength");
		//CONTROL  VARIABLES FOR DIRECTIONAL WAVE
		GLuint amplitude_water_dir = glGetUniformLocation(waterShader.getId(), "amplitude_dir");
		GLuint frequency_water_dir = glGetUniformLocation(waterShader.getId(), "frequency_dir");
		GLuint time_water_dir = glGetUniformLocation(waterShader.getId(), "time_dir");
		GLuint phase_water_dir = glGetUniformLocation(waterShader.getId(), "phase_dir");
		GLuint direction_water_dir = glGetUniformLocation(waterShader.getId(), "direction_dir");
		//CONTROL  VARIABLES FOR CIRCULAR WAVE
		GLuint amplitude_water_cir = glGetUniformLocation(waterShader.getId(), "amplitude_cir");
		GLuint frequency_water_cir = glGetUniformLocation(waterShader.getId(), "frequency_cir");
		GLuint time_water_cir = glGetUniformLocation(waterShader.getId(), "time_cir");
		GLuint phase_water_cir = glGetUniformLocation(waterShader.getId(), "phase_cir");
		//CONTROL  VARIABLES FOR BOTH TYPE OF WAVES
		GLuint sharpcontrol_water = glGetUniformLocation(waterShader.getId(), "sharpcontrol");
		GLuint stepness_water = glGetUniformLocation(waterShader.getId(), "stepness");
		GLuint center_water = glGetUniformLocation(waterShader.getId(), "center");
		GLuint enableDirectional_water = glGetUniformLocation(waterShader.getId(), "enableDirectional");
		GLuint enableCircular_water = glGetUniformLocation(waterShader.getId(), "enableCircular");
		//MVP AND MODEL
		ModelMatrix = glm::mat4(1.0); //classic 4D matrix
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); //set position of water plane
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix; //Model View Projection

		//PASS UNIFORM MATRICES FOR MVP AND MODEL
		glUniformMatrix4fv(MatrixID_water, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID_water, 1, GL_FALSE, &ModelMatrix[0][0]);
		//PASS THE UNIFORM VARIABLES TO THE SHADER FOR DIRECTIONAL WAVE
		glUniform1f(amplitude_water_dir, amplitude_directional);
		glUniform1f(frequency_water_dir, frequency_directional);
		glUniform1f(time_water_dir, glfwGetTime());
		glUniform1f(phase_water_dir, phase_directional);
		//PASS THE UNIFORM VARIABLES TO THE SHADER FOR CIRCULAR WAVE
		glUniform1f(amplitude_water_cir, amplitude_circular);
		glUniform1f(frequency_water_cir, frequency_circular);
		glUniform1f(time_water_cir, glfwGetTime());
		glUniform1f(phase_water_cir, phase_circular);
		//PASS UNIFORM VARIABLES FOR LIGHT
		glUniform3f(lightColor_water, lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(lightPos_water, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewPos_water, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		//PASS UNIFORM VARIABLES FOR DIRECTION OF THE DIRECTIONAL WAVE AND THE CENTER OF CIRCULAR WAVE
		glUniform3f(direction_water_dir, wavedir_x, wavedir_y, wavedir_z);
		glUniform3f(center_water, center.x, center.y, center.z);
		//PASS UNIFORM VARIABLES TO CONTROL THE BEHAVIOUR OF BOTH WAVES FROM KEYBOARD INPUTS
		glUniform1f(sharpcontrol_water, sharpcontrol);
		glUniform1f(stepness_water, stepness);
		glUniform1f(sunAmbientStrength_water, sunAmbientStrength);
		glUniform1i(enableDirectional_water, enableDirectional);
		glUniform1i(enableCircular_water, enableCircular);
		plane.draw(waterShader); //DRAW THE WATER PLANE USING ITS OWN SHADER

		//*******************
		//** CREATE SKYBOX **
		//*******************

		//USE SPECIFIC SHADER: waterShader;
		skyShader.use();

		//GET THE UNIFORM LOCATION OF MVP, SUN STRENGTH AND LIGHT COLOR
		GLuint MatrixID_skybox = glGetUniformLocation(skyShader.getId(), "MVP");
		GLuint sunAmbientStrength_skybox = glGetUniformLocation(skyShader.getId(), "sunAmbientStrength");
		GLuint lightColor_skybox = glGetUniformLocation(skyShader.getId(), "lightColor");
		
		//MVP AND MODEL
		ModelMatrix = glm::mat4(1.0); //classic 4D matrix;
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2000.0f, 600.f, 2000.0f));
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix; //MVP matrix

		//PASS THE UNIFORM MATRIX AND VARIABLES 
		glUniformMatrix4fv(MatrixID_skybox, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(sunAmbientStrength_skybox, sunAmbientStrength);
		glUniform3f(lightColor_skybox, lightColor.x, lightColor.y, lightColor.z);

		skybox.draw(skyShader); //DRAW THE SKYBOX USING ITS OWN SHADER

		window.update(); //update the frames
	}
}

//PROCESS THE INPUT FROM THE KEYBOARD
void processKeyboardInput()
{
	//SPEED OF THE CAMERA, ROTATION AND TRANSLATION
	float cameraSpeed = 90 * deltaTime;

	//CONTROL THE VARIABLES
	//CHANGE THE SHARP CONTROL VARIABLE, SHARP CONTROL = THE FREQUENCY AND SHARPNESS OF THE WAVE PEAKS
	if (window.isPressed(GLFW_KEY_1)) {
		sharpcontrol -= 0.0001f;
		std::cout << "Sharp Control:" << sharpcontrol << std::endl;
	}
	if (window.isPressed(GLFW_KEY_2)) {
		sharpcontrol += 0.0001f;
		std::cout << "Sharp Control:" << sharpcontrol << std::endl;
	}

	//CHANGE THE WAVE DIRECTION ON THE X AXIS
	if (window.isPressed(GLFW_KEY_3)) {
		wavedir_x -= 0.0001f;
		std::cout <<"wave x:"<< wavedir_x << std::endl;
	}
	if (window.isPressed(GLFW_KEY_4)) {
		wavedir_x += 0.0001f;
		std::cout << "wave x:" << wavedir_x << std::endl;
	}

	//CHANGE THE WAVE DIRECTION ON THE Z AXIS
	if (window.isPressed(GLFW_KEY_5)) {
		wavedir_z -= 0.0001f;
		std::cout << "wave z:" << wavedir_z << std::endl;
	}
	if (window.isPressed(GLFW_KEY_6)) {
		wavedir_z += 0.0001f;
		std::cout << "wave z:" << wavedir_z << std::endl;
	}

	//CHANGE THE STEPNESS OF THE WAVES, STEPNESS = THE DISTANCE BETWEEN WAVE PEAKS
	if (window.isPressed(GLFW_KEY_7)) {
		stepness -= 0.0001f;
		std::cout << "Stepness:" << stepness << std::endl;
	}
	if (window.isPressed(GLFW_KEY_8)) {
		stepness += 0.0001f;
		std::cout << "Stepness:" << stepness << std::endl;
	}

	//USE "C" TO ACTIVATE THE CIRCULAR WAVES AND "V" TO DISABLE THEM
	if (window.isPressed(GLFW_KEY_C))
		enableCircular = 1;
	if (window.isPressed(GLFW_KEY_V))
		enableCircular = 0;
	//USE "B" TO ACTIVATE THE DIRECTIONAL WAVES AND "N" TO DISABLE THEM
	if (window.isPressed(GLFW_KEY_B))
		enableDirectional = 1;
	if (window.isPressed(GLFW_KEY_N))
		enableDirectional = 0;

	//CAMERA TRANSLATION
	if (window.isPressed(GLFW_KEY_W))
		camera.keyboardMoveFront(cameraSpeed);
	if (window.isPressed(GLFW_KEY_S))
		camera.keyboardMoveBack(cameraSpeed);
	if (window.isPressed(GLFW_KEY_A))
		camera.keyboardMoveLeft(cameraSpeed);
	if (window.isPressed(GLFW_KEY_D))
		camera.keyboardMoveRight(cameraSpeed);
	if (window.isPressed(GLFW_KEY_R))
		camera.keyboardMoveUp(cameraSpeed);
	if (window.isPressed(GLFW_KEY_F))
		camera.keyboardMoveDown(cameraSpeed);

	//CAMERA ROTATION
	if (window.isPressed(GLFW_KEY_LEFT))
		camera.rotateOy(cameraSpeed);
	if (window.isPressed(GLFW_KEY_RIGHT))
		camera.rotateOy(-cameraSpeed);
	if (window.isPressed(GLFW_KEY_UP))
		camera.rotateOx(cameraSpeed);
	if (window.isPressed(GLFW_KEY_DOWN))
		camera.rotateOx(-cameraSpeed);
}
