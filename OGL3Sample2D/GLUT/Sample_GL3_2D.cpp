#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <math.h>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

int width = 600;
int height = 600;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

struct GLbucket {
	struct VAO* bucketimg;
	glm::mat4 transvector;
	glm::mat4 rotvector;
};

struct GLcannon {
	struct VAO* cannonimg;
	glm::mat4 transvector;
	glm::mat4 rotvector;
	float cannon_rotation;
};

struct GLlaser {
	struct VAO* laserimg;
	glm::mat4 transvector;
	glm::mat4 rotvector;
	float laser_rotation;
	int mirror1;
	int mirror2;
	int mirror3;
};

struct GLbrick {
	struct VAO* brickimg;
	glm::mat4 transvector;
	float yco;
	float xco;
	int col;
	int os;
};

struct GLbrick brick[100000];
int brickcount=0;
struct GLbucket buck[2];
struct GLcannon cannon;
struct GLlaser laser;
struct VAO* mirror;
glm::mat4 mirrortransvector;
struct VAO* mirror1;
float brickspeed=-0.01f;
float zoom;
int score=0;
int lives=5;

void draw();

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed */
void keyboardDown (unsigned char key, int x, int y)
{

	if (key=='n')
	{
		if (brickspeed>-0.04f)
			brickspeed=brickspeed-0.01f;
	}

	if (key=='m')
	{
		if (brickspeed<-0.01f)
			brickspeed=brickspeed+0.01f;
	}
	if (key=='a')
	{
		if (cannon.transvector[3][1]<3.5)
		{
			laser.transvector *= glm::translate (glm::vec3(0, 0.1, 0));  // rotate about vector (-1,1,1)
			cannon.transvector *= glm::translate (glm::vec3(0, 0.1, 0)); 
		}
	}

	if (key=='d')
	{
		if (cannon.transvector[3][1]>-2.0)
		{
			laser.transvector *= glm::translate (glm::vec3(0, -0.1, 0));  // rotate about vector (-1,1,1)
			cannon.transvector *= glm::translate (glm::vec3(0, -0.1, 0)); 
		}
	}

    if (key=='s')
	{
		if (cannon.cannon_rotation<75)
		{
			cannon.cannon_rotation=((int)cannon.cannon_rotation%90+3)%90;
			laser.laser_rotation=((int)laser.laser_rotation%90+3)%90;
			cannon.rotvector = glm::rotate((float)(cannon.cannon_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)fs
			laser.rotvector = glm::rotate((float)(laser.laser_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		}
	}
	if (key=='f')
	{
		if (cannon.cannon_rotation>-75)
		{
			cannon.cannon_rotation=(((int)cannon.cannon_rotation%90-3)%90);
			laser.laser_rotation=(((int)laser.laser_rotation%90-3)%90);
			cannon.rotvector = glm::rotate((float)(cannon.cannon_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			laser.rotvector = glm::rotate((float)(laser.laser_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		}
	}
	if (key==32)
	{
		system("aplay -q cannon.wav &");
		//PlaySound("cannon.wav", NULL, SND_ASYNC|SND_FILENAME|SND_LOOP);
		int count=0;
		float m1=-1.0f;
	    float c1=1.0f;
		float m2=tan((float)laser.laser_rotation*M_PI/180.0f);
	    float c2=3.2*m2+laser.transvector[3][1];
		//cout << c2 << " ";
		float intx=(c2-c1)/(m1-m2);
		float inty=(m1*intx)+c1;
		while (count<100)
		{
			laser.transvector *= glm::translate (glm::vec3(0.1*cos((float)(laser.laser_rotation*M_PI/180.0f)), 0.1*sin((float)(laser.laser_rotation*M_PI/180.0f)), 0));
			if (laser.transvector[3][0]+0.6>3 && laser.transvector[3][0]<3.1 && laser.transvector[3][1]>0 && laser.transvector[3][1]<1 && laser.mirror1==0)
			 {
			 	 count=0;
				 //cout << "Mirror\n";
				 laser.mirror1=1;
				 laser.laser_rotation+=180-(2*laser.laser_rotation);
				 laser.rotvector = glm::rotate((float)(laser.laser_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			 }
			 //cout << intx << " " << inty << "\n";
			 if (intx>-1/sqrt(2) && intx<0 && intx>laser.transvector[3][0] && intx<laser.transvector[3][0]+0.6 && laser.mirror2==0)
			 {
			 	//cout << "Mirror will collide\n";
			 	laser.mirror2=1;
			 	laser.laser_rotation+=270-(2*laser.laser_rotation);
				 laser.rotvector = glm::rotate((float)(laser.laser_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			 }

			 count++;
			 draw();
		}
		laser.transvector=glm::mat4(1.0f);
		laser.transvector *= glm::translate (glm::vec3(-3.5f, cannon.transvector[3][1], 0));
		laser.mirror1=0;
		laser.mirror2=0;
		laser.laser_rotation=cannon.cannon_rotation;
		laser.rotvector=cannon.rotvector;
		draw();
	}
}

/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
    switch (key) {
        case 'c':
        case 'C':
            rectangle_rot_status = !rectangle_rot_status;
            break;
        case 'p':
        case 'P':
            triangle_rot_status = !triangle_rot_status;
            break;
        case 'x':
            // do something
            break;
        default:
            break;
    }
}

/* Executed when a special key is pressed */
float pan;
void keyboardSpecialDown (int key, int x, int y)
{
	if (key==GLUT_KEY_UP)
	{
		//Matrices.model=glm::mat4(1.0f); 
		if (zoom<1.5)
			zoom=zoom+0.4;
		Matrices.projection = glm::ortho(-4.0f+zoom, 4.0f-zoom, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
	}
	if (key==GLUT_KEY_DOWN)
	{
		if (zoom>0.1)
			zoom=zoom-0.4;
		Matrices.projection = glm::ortho(-4.0f+zoom, 4.0f-zoom, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
	}
	if (key==GLUT_KEY_RIGHT)
	{
			if (zoom>0.1 && pan>-zoom+0.1)
				pan=pan-0.4;
			Matrices.projection = glm::ortho(-4.0f+zoom-pan, 4.0f-zoom-pan, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
	}
	if (key==GLUT_KEY_LEFT)
	{
		if (zoom<1.5 && pan<zoom)
			pan=pan+0.4;
		Matrices.projection = glm::ortho(-4.0f+zoom-pan, 4.0f-zoom-pan, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
	}
	if (key==GLUT_KEY_LEFT && (glutGetModifiers()==GLUT_ACTIVE_ALT))
	{
		if (buck[0].transvector[3][0]>-2.5)
			buck[0].transvector *= glm::translate (glm::vec3(-0.1, 0, 0));        // glTranslatef
	}
	if (key==GLUT_KEY_RIGHT && (glutGetModifiers()==GLUT_ACTIVE_ALT))
	{
		if (buck[0].transvector[3][0]<2.5)
			buck[0].transvector *= glm::translate (glm::vec3(0.1, 0, 0));        // glTranslatef
	}
	if (key==GLUT_KEY_LEFT && (glutGetModifiers()==GLUT_ACTIVE_CTRL))
	{
		if (buck[1].transvector[3][0]>-2.5)
			buck[1].transvector *= glm::translate (glm::vec3(-0.1, 0, 0));        // glTranslatef
	}
	if (key==GLUT_KEY_RIGHT && (glutGetModifiers()==GLUT_ACTIVE_CTRL))
	{
		if (buck[1].transvector[3][0]<2.5)
			buck[1].transvector *= glm::translate (glm::vec3(0.1, 0, 0));        // glTranslatef
	}
}


/* Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{
}

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */
void mouseClick (int button, int state, int x, int y)
{
	//cout << x;
    /*switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
                cout << "hello\n";
            break;
        case GLUT_RIGHT_BUTTON:
            if (state == GLUT_UP) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }*/
    if (button==3)
	{
		//Matrices.model=glm::mat4(1.0f); 
		if (zoom<1.5)
			zoom=zoom+0.4;
		Matrices.projection = glm::ortho(-4.0f+zoom, 4.0f-zoom, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
	}
	if (button==4)
	{
		if (zoom>0.1)
			zoom=zoom-0.4;
		Matrices.projection = glm::ortho(-4.0f+zoom, 4.0f-zoom, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
	}
    if (button==GLUT_LEFT_BUTTON && state==GLUT_DOWN)
    {
    	//cout << x;
    	cannon.rotvector=glm::mat4(1.0f);
    	cannon.cannon_rotation=0;
    	laser.rotvector=glm::mat4(1.0f);
    	laser.laser_rotation=0;
    	float mouseX = -1.0 + 2.0 * x / 600 ;
    	float mouseY = 1.0 - 2.0 * y / 600;
    	if (atan2(4*mouseY-cannon.transvector[3][1],4*mouseX+3.2) * 180 / M_PI<75 && atan2(4*mouseY-cannon.transvector[3][1],4*mouseX+3.2) * 180 / M_PI>-75)
    	{
	    	cannon.cannon_rotation=atan2(4*mouseY-cannon.transvector[3][1],4*mouseX+3.2) * 180 / M_PI;
	    	laser.laser_rotation=atan2(4*mouseY-laser.transvector[3][1],4*mouseX+3.2) * 180 / M_PI;
	    	cannon.rotvector = glm::rotate((float)(cannon.cannon_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			laser.rotvector = glm::rotate((float)(laser.laser_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		}
	}
}

/* Executed when the mouse moves to position ('x', 'y') */
int bucksel=0;
void mouseMotion (int x, int y)
{
	glm::vec2 pos(0.0, 0.0);
	float mouseX = 4*(-1.0 + 2.0 * x / 600);
    float mouseY = 4*(1.0 - 2.0 * y / 600);
    if (mouseX<-3 && mouseX>-3.5 && mouseY>-0.3+cannon.transvector[3][1] && mouseY<0.3+cannon.transvector[3][1])
    {
    	if (mouseY<3.5 && mouseY>-2)
    	{
	    	cannon.transvector = glm::translate (glm::vec3(-3.6, mouseY, 0));
	    	laser.transvector = glm::translate (glm::vec3(-3.5, mouseY, 0));
    	}
    }

    else if (mouseY>-4 && mouseY<-3 && mouseX>buck[0].transvector[3][0]-0.8 && mouseX<buck[0].transvector[3][0]+0.8)
    {
    	if (mouseX>-2.5 && mouseX<2.5)
    		buck[0].transvector=glm::translate(glm::vec3(mouseX,0,0));
    }
    else if (mouseY>-4 && mouseY<-3 && mouseX>buck[1].transvector[3][0]-0.8 && mouseX<buck[1].transvector[3][0]+0.8)
    {
    	if (mouseX>-2.5 && mouseX<2.5)
    		buck[1].transvector=glm::translate(glm::vec3(mouseX,0,0));
    }
    //float angle = 90 + atan2(pos.y-mouseY, pos.x-mouseX) * 180 / 3.1415926;

    //std::cout << mouseX << ", " << mouseY << std::endl;
    //std::cout << x << ", " << y << std::endl;
    //std::cout << "hello" << mouseX << std::endl;
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.2,-2.5,0, // vertex 1
    0.6,-2.5,0, // vertex 2
    0.6, 4,0, // vertex 3

    0.6, 4,0, // vertex 3
    -0.2, 4,0, // vertex 4
    -0.2,-2.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createMirror ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.05,0,0, // vertex 1
    0.05,0,0, // vertex 2
    0.05, 1,0, // vertex 3

    0.05, 1,0, // vertex 3
    -0.05, 1,0, // vertex 4
    -0.05,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.44,0.65,1, // color 1
    0.44,0.65,1, // color 2
    0.44,0.65,1, // color 3

    0.44,0.65,1, // color 3
    0.44,0.65,1, // color 4
    0.44,0.65,1,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  mirror = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createMirror1 ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    2.8,1.5,0, // vertex 1
    2.9,1.5,0, // vertex 2
    2.9, 2.5,0, // vertex 3

    2.9, 2.5,0, // vertex 3
    2.8, 2.5,0, // vertex 4
    2.8, 1.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  mirror1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createMirror2 ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    3,0,0, // vertex 1
    3.1,0,0, // vertex 2
    3.1, 1,0, // vertex 3

    3.1, 1,0, // vertex 3
    3, 1,0, // vertex 4
    3,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  mirror = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCannon ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.3,-0.3,0, // vertex 1
    0.3,-0.3,0, // vertex 2
    0.3, 0.3,0, // vertex 3

    0.3, 0.3,0, // vertex 3
    -0.3, 0.3,0, // vertex 4
    -0.3,-0.3,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,1, // color 1
    1,0,1, // color 2
    1,0,1, // color 3

    1,0,1, // color 3
    1,0,1, // color 4
    1,0,1,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  cannon.cannonimg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createLaser ()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    0.0,-0.1,0, // vertex 1
    0.6,-0.1,0, // vertex 2
    0.6, 0.1,0, // vertex 3

    0.6, 0.1,0, // vertex 3
    0.0, 0.1,0, // vertex 4
    0.0,-0.1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,1,0, // color 1
    1,1,0, // color 2
    1,1,0, // color 3

    1,1,0, // color 3
    1,1,0, // color 4
    1,1,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  laser.laserimg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBrick(int no)
{
	const GLfloat vertex_buffer_data [] = {
    -0.1,3.5,0, // vertex 1
    0.1,3.5,0, // vertex 2
    0.1, 3.7,0, // vertex 3

    0.1, 3.7,0, // vertex 3
    -0.1, 3.7,0, // vertex 4
    -0.1,3.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  brick[no].brickimg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}

void createBrick1(int no)
{
	const GLfloat vertex_buffer_data [] = {
    -0.1,3.5,0, // vertex 1
    0.1,3.5,0, // vertex 2
    0.1, 3.7,0, // vertex 3

    0.1, 3.7,0, // vertex 3
    -0.1, 3.7,0, // vertex 4
    -0.1,3.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  brick[no].brickimg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}
void createBrick2(int no)
{
	const GLfloat vertex_buffer_data [] = {
    -0.1,3.5,0, // vertex 1
    0.1,3.5,0, // vertex 2
    0.1, 3.7,0, // vertex 3

    0.1, 3.7,0, // vertex 3
    -0.1, 3.7,0, // vertex 4
    -0.1,3.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  brick[no].brickimg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}

void createBrick3(int no)
{
	const GLfloat vertex_buffer_data [] = {
    -0.1,3.5,0, // vertex 1
    0.1,3.5,0, // vertex 2
    0.1, 3.7,0, // vertex 3

    0.1, 3.7,0, // vertex 3
    -0.1, 3.7,0, // vertex 4
    -0.1,3.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0,  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  brick[no].brickimg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}

void createBucket1()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.4,-4,0, // vertex 1
    0.4,-4,0, // vertex 2
    0.8, -3,0, // vertex 3

    0.8, -3,0, // vertex 3
    -0.8, -3,0, // vertex 4
    -0.4,-4,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  buck[0].bucketimg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBucket2()
{
  // GL3 accepts only Triangles. Quads are not supported static
  const GLfloat vertex_buffer_data [] = {
    -0.4,-4,0, // vertex 1
    0.4,-4,0, // vertex 2
    0.8, -3,0, // vertex 3

    0.8, -3,0, // vertex 3
    -0.8, -3,0, // vertex 4
    -0.4,-4,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1, // color 2
    0,0,1, // color 3

    0,0,1, // color 3
    0,0,1, // color 4
    0,0,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  buck[1].bucketimg = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(2.0f);


  /* Render your scene */

  glm::mat4 translateTriangle = glm::translate (glm::vec3(0.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatemirror1 = glm::translate (glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 rotatemirror1 = glm::rotate((float)(45*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  Matrices.model *= (translatemirror1*rotatemirror1); 
  MVP = VP * Matrices.model; // MVP = p * V * M
  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror);

  // draw3DObject draws the VAO given to it using current MVP matrix
  //draw3DObject(triangle);

  Matrices.model = glm::mat4(1.0f);

       // glTranslatef
  
  Matrices.model *= buck[0].transvector;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(buck[0].bucketimg);

  Matrices.model = glm::mat4(1.0f);

  
  Matrices.model *= buck[1].transvector;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);


  draw3DObject(buck[1].bucketimg);

  Matrices.model = glm::mat4(1.0f);

  Matrices.model *= (laser.transvector*laser.rotvector);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(laser.laserimg);
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(-4.0f, 0.0f, 0.0f));
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);
  // draw3DObject draws the VAO given to it using current MVP matrix
  

  Matrices.model = glm::mat4(1.0f);

  /*glm::mat4 translateRectangle = glm::translate (glm::vec3(-4.0f, 0.0f, 0.0f));
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);*/

  // draw3DObject draws the VAO given to it using current MVP matrix
  mirrortransvector = glm::translate (glm::vec3(3.05f, 0.0f, 0.0f));
  Matrices.model *= mirrortransvector;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror);
  Matrices.model = glm::mat4(1.0f);

  //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (cannon.transvector*cannon.rotvector);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(cannon.cannonimg);
  Matrices.model = glm::mat4(1.0f);
  int i;
  for (i=0;i<=brickcount;i++)
  {
  	  Matrices.model = glm::mat4(1.0f);
  	  if (i%50==0 && i==brickcount)
  	  {
  	  	  srand(time(NULL));
  	  	  int x=rand()%4;
  	  	  if (x==0)
  	  	  {
		  	createBrick(i);
		  	brick[i/50].col=0;
		  }
		  else if (x==1)
  	  	  {
		  	createBrick1(i);
		  	brick[i/50].col=1;
		  	brick[i/50].os=0;
		  }
		  else if (x==2)
  	  	  {
		  	createBrick2(i);
		  	brick[i/50].col=2;
		  }
		  else
		  {
		  	createBrick3(i);
		  	brick[i/50].col=3;
		  }
		  int y=rand()%9;
		  if (y==0)
		  	brick[i/50].xco=-4;
		  if (y==1)
		  	brick[i/50].xco=-3;
		  if (y==2)
		  	brick[i/50].xco=-2;
		  if (y==3)
		  	brick[i/50].xco=-1;
		  if (y==4)
		  	brick[i/50].xco=0;
		  if (y==5)
		  	brick[i/50].xco=1;
		  if (y==6)
		  	brick[i/50].xco=2;
		  if (y==7)
		  	brick[i/50].xco=3;
		  if (y==8)
		  	brick[i/50].xco=-4;
		  brick[i/50].transvector = glm::translate (glm::vec3(1.0f*brick[i/50].xco, -0.01*brick[i/50].yco, 0.0f));
		  brick[i/50].yco++;
		  Matrices.model *= brick[i/50].transvector;
		  MVP = VP * Matrices.model;
		  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		  draw3DObject(brick[i].brickimg);
		  Matrices.model = glm::mat4(1.0f);
	 }
	 if (i%50==0 && i<brickcount)
	 {
	 	  if (brick[i/50].yco>800)
	 	  	brick[i/50].os=1;
	 	  if (brick[i/50].transvector[3][1]<-6.5)
	 	  {
	 			brick[i/50].yco=1000;
	 	  }
	 	  if (brick[i/50].yco>800 && brick[i/50].os==0 && brick[i/50].col==1 && brick[i/50].transvector[3][0]>buck[0].transvector[3][0]-0.8 && brick[i/50].transvector[3][0]<buck[0].transvector[3][0]+0.8)
	 	  {
	 	  		cout << "\nGame over\n";
	 	  		exit(1);
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].yco>800 && brick[i/50].os==0 && brick[i/50].col==1 && brick[i/50].transvector[3][0]>buck[1].transvector[3][0]-0.8 && brick[i/50].transvector[3][0]<buck[1].transvector[3][0]+0.8)
	 	  {
	 	  		cout << "\nGame over\n";
	 	  		exit(1);
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].yco>800 && brick[i/50].os==0 && brick[i/50].col==0 && brick[i/50].transvector[3][0]>buck[0].transvector[3][0]-0.8 && brick[i/50].transvector[3][0]<buck[0].transvector[3][0]+0.8)
	 	  {
	 	  		score+=10;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;  
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].yco>800 && brick[i/50].os==0 && brick[i/50].col==2 && brick[i/50].transvector[3][0]>buck[1].transvector[3][0]-0.8 && brick[i/50].transvector[3][0]<buck[1].transvector[3][0]+0.8)
	 	  {
	 	  		score+=10;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;    
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.6<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.6>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==1)
	 	  {
	 	  		system("aplay -q brick.wav &");
	 	  		score+=10;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;   
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.3<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.3>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==1)
	 	  {
	 	  		system("aplay -q brick.wav &");
	 	  		score+=10;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;  
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.0<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.0>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==1)
	 	  {
	 	  	system("aplay -q brick.wav &");
	 	  		score+=10;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;  
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.6<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.6>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==3)
	 	  {
	 	  	
	 	  		score+=50;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;
				laser.laser_rotation+=180-(2*laser.laser_rotation);
				laser.rotvector = glm::rotate((float)(laser.laser_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)   
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.3<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.3>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==3)
	 	  {
	 	  		score+=50;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;  
	 	  		laser.laser_rotation+=180-(2*laser.laser_rotation);
				laser.rotvector = glm::rotate((float)(laser.laser_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.0<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.0>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==3)
	 	  {
	 	  		score+=50;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;  
	 	  		laser.laser_rotation+=180-(2*laser.laser_rotation);
				laser.rotvector = glm::rotate((float)(laser.laser_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.6<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.6>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==2)
	 	  {
	 	  		if (score>0)
	 	  			score-=10;
	 	  		lives--;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush; 
	 	  		if (lives==0)
	 	  		{
	 	  			cout << "\nGame over\n";
	 	  			exit(1);
	 	  		}  
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.3<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.3>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==2)
	 	  {
	 	  		if (score>0)
	 	  			score-=10;
	 	  		lives--;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush; 
	 	  		if (lives==0)
	 	  		{
	 	  			cout << "\nGame over\n";
	 	  			exit(1);
	 	  		} 
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.0<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.0>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==2)
	 	  {
	 	  		if (score>0)
	 	  			score-=10;
	 	  		lives--;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush; 
	 	  		if (lives==0)
	 	  		{
	 	  			cout << "\nGame over\n";
	 	  			exit(1);
	 	  		} 
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.6<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.6>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==0)
	 	  {
	 	  		if (score>0)
	 	  			score-=10;
	 	  		lives--;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;
	 	  		if (lives==0)
	 	  		{
	 	  			cout << "\nGame over\n";
	 	  			exit(1);
	 	  		}   
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.3<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.3>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==0)
	 	  {
	 	  		if (score>0)
	 	  			score-=10;
	 	  		lives--;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;
	 	  		if (lives==0)
	 	  		{
	 	  			cout << "\nGame over\n";
	 	  			exit(1);
	 	  		}
	 	  		  
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  if (brick[i/50].os==0 && laser.transvector[3][0]+0.0<brick[i/50].transvector[3][0]+0.1 && laser.transvector[3][0]+0.0>brick[i/50].transvector[3][0]-0.1 && laser.transvector[3][1]+0.0>brick[i/50].transvector[3][1]+3.5 && laser.transvector[3][1]+0.0<brick[i/50].transvector[3][1]+3.7 && brick[i/50].col==0)
	 	  {
	 	  		if (score>0)
	 	  			score-=10;
	 	  		lives--;
	 	  		cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;
	 	  		if (lives==0)
	 	  		{
	 	  			cout << "\nGame over\n";
	 	  			exit(1);
	 	  		}  
	 	  		brick[i/50].yco=1000;
	 	  		brick[i/50].os=1;
	 	  }
	 	  brick[i/50].transvector = glm::translate (glm::vec3(0.5f*brick[i/50].xco, brickspeed*brick[i/50].yco, 0.0f));
		  brick[i/50].yco++;
		  Matrices.model *= brick[i/50].transvector;
		  MVP = VP * Matrices.model;
		  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		  draw3DObject(brick[i].brickimg);
		  Matrices.model = glm::mat4(1.0f);
	 }
  }
  brickcount++;
  if (brickcount==100000)
  {
  	int j;
  	for (j=0;j<100000;j++)
  	{
  		brick[j].xco=0;
  		brick[j].yco=0;
  		brick[j].transvector=glm::mat4(1.0f);
  	}
  	brickcount=0;
  }
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  //glScalef(0.5f, 0.5f, 2.0f);




  // Swap the frame buffers
  glutSwapBuffers ();

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  //triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
}


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)
    
    glutIgnoreKeyRepeat (false); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height)
{
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (0.79f, 0.79f, 0.79f, 1.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	createRectangle ();
	createBucket1();
	createBucket2();
	createLaser();
	createCannon();
	createMirror();
	createMirror1();
	buck[0].transvector = glm::translate (glm::vec3(1.2, 0, 0));        // glTranslatef
	buck[1].transvector = glm::translate (glm::vec3(-1.2, 0, 0));        // glTranslatef
	laser.transvector = glm::translate (glm::vec3(-3.5f, 0.0f, 0.0f));
	cannon.transvector = glm::translate (glm::vec3(-3.6f, 0.0f, 0.0f));
	//createLeftSpace()

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	cout << "\r" << "Score: " <<score << " " << "Lives: " <<lives << flush;  
}

int main (int argc, char** argv)
{
	
    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

	initGL (width, height);

    glutMainLoop ();

    return 0;
}
