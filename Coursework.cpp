/*
   This is a variation of tutorial3 using a single VBO for specifying the vertex
   attribute data; it is done by setting the VertexAttribPointer parameters
   "stride" and "pointer" to suitable values.
   In particular for the pointer parameter, macro "offsetof" should be used so to
   avoid problem with alignment and padding for different architecture.

   Modified to use GLM

   By consultit@katamail.com

 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stddef.h> /* must include for the offsetof macro */
/*
 *
 * Include files for Windows, Linux and OSX
 * __APPLE is defined if OSX, otherwise Windows and Linux.
 *
 */

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB 1
#include <GLFW/glfw3.h>
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include <stdlib.h>
#include <math.h>

void Check(const char *where) { // Function to check OpenGL error status
  const char * what;
  int err = glGetError();   //0 means no error
  if(!err)
    return;
  if(err == GL_INVALID_ENUM)
    what = "GL_INVALID_ENUM";
  else if(err == GL_INVALID_VALUE)
    what = "GL_INVALID_VALUE";
  else if(err == GL_INVALID_OPERATION)
    what = "GL_INVALID_OPERATION";
  else if(err == GL_INVALID_FRAMEBUFFER_OPERATION)
    what = "GL_INVALID_FRAMEBUFFER_OPERATION";
  else if(err == GL_OUT_OF_MEMORY)
    what = "GL_OUT_OF_MEMORY";
  else
    what = "Unknown Error";
  fprintf(stderr, "Error (%d) %s  at %s\n", err, what, where);
  exit(1);
}

void CheckShader(int sp, const char *x){
  int length;
  char text[1001];
  glGetProgramInfoLog(sp, 1000, &length, text);   // Check for errors
  if(length > 0) {
    fprintf(stderr, "Validate Shader Program\nMessage from:%s\n%s\n", x, text );
    exit(1);
  }
}

char* filetobuf(char *file) { /* A simple function that will read a file into an allocated char pointer buffer */
  FILE *fptr;
  long length;
  char *buf;
  fprintf(stderr, "Loading %s\n", file);
        #pragma warning (disable : 4996)
  fptr = fopen(file, "rb");   /* Open file for reading */
  if (!fptr) {   /* Return NULL on failure */
    fprintf(stderr, "failed to open %s\n", file);
    return NULL;
  }
  fseek(fptr, 0, SEEK_END);   /* Seek to the end of the file */
  length = ftell(fptr);   /* Find out how many bytes into the file we are */
  buf = (char*)malloc(length + 1);   /* Allocate a buffer for the entire length of the file and a null terminator */
  fseek(fptr, 0, SEEK_SET);   /* Go back to the beginning of the file */
  fread(buf, length, 1, fptr);   /* Read the contents of the file in to the buffer */
  fclose(fptr);   /* Close the file */
  buf[length] = 0;   /* Null terminator */
  return buf;   /* Return the buffer */
}


struct Vertex {
  Vertex(): color{0,1,0} {};
  GLfloat position[3];
  GLfloat color[3];
};

typedef struct{
  Vertex p1, p2, p3;
} Facet;
/* These pointers will receive the contents of our shader source code files */
GLchar *vertexsource, *fragmentsource;
/* These are handles used to reference the shaders */
GLuint vertexshader, fragmentshader;
/* This is a handle to the shader program */
GLuint shaderprogram;
GLuint vao, conevao, cylindervao, vbo[1], conevbo[1], cylindervbo[1]; /* Create handles for our Vertex Array Object and One Vertex Buffer Object */
std::vector<Vertex> v, conev, cylinderv;

int mode = 0;
/* Mode 0 corresponds to a wireframe sphere, and is accessed by pressing A.
   Mode 1 corresponds to a lighted sphere, and is accessed by pressing B.
   Mode 2 corresponds to a basic wireframe rocket, and is accessed by pressing C.*/

/* Return the midpoint of two vectors */
Vertex Midpoint(Vertex p1, Vertex p2){
  Vertex p;
  p.position[0] = (p1.position[0] + p2.position[0])/2;
  p.position[1] = (p1.position[1] + p2.position[1])/2;
  p.position[2] = (p1.position[2] + p2.position[2])/2;
  return p;
}

/* Normalise a vector */
void Normalise(Vertex *p){
  float length;
  length = sqrt(pow(p->position[0],2) + pow(p->position[1],2) + pow(p->position[2],2));
  if(length != 0){
    p->position[0] /= length;
    p->position[1] /= length;
    p->position[2] /= length;
  } else{
    p->position[0] = 0;
    p->position[1] = 0;
    p->position[2] = 0;
  }
}

/* Although there is a function called CreateSphere that implements a sphere, CreateUnitSphere
  contains the algorithm necessary to calculate the vertices and is therefore kept in a separate
  function for ease of reuse in future code.
 */
int CreateUnitSphere(int iterations, Facet *facets){/* Algorithm to calculate vertices of sphere*/
  int i, j, n, nstart;
  Vertex p1,p2,p3,p4,p5,p6;
  p1.position[0] = 0.0; p1.position[1] = 0.0; p1.position[2] = 1.0;
  p2.position[0] = 0.0; p2.position[1] = 0.0; p2.position[2] = -1.0;
  p3.position[0] = -1.0; p3.position[1] = -1.0; p3.position[2] = 0.0;
  p4.position[0] = 1.0; p4.position[1] = -1.0; p4.position[2] = 0.0;
  p5.position[0] = 1.0; p5.position[1] = 1.0; p5.position[2] = 0.0;
  p6.position[0] = -1.0; p4.position[1] = -1.0; p4.position[2] = 0.0;
  Normalise(&p1); Normalise(&p2); Normalise(&p3); Normalise(&p4); Normalise(&p5); Normalise(&p6);

  facets[0].p1 = p1;facets[0].p2 = p4;facets[0].p3 = p5;
  facets[1].p1 = p1;facets[1].p2 = p5;facets[1].p3 = p6;
  facets[2].p1 = p1;facets[2].p2 = p6;facets[2].p3 = p3;
  facets[3].p1 = p1;facets[3].p2 = p3;facets[3].p3 = p4;
  facets[4].p1 = p2;facets[4].p2 = p5;facets[4].p3 = p4;
  facets[5].p1 = p2;facets[5].p2 = p6;facets[5].p3 = p5;
  facets[6].p1 = p2;facets[6].p2 = p3;facets[6].p3 = p6;
  facets[7].p1 = p2;facets[7].p2 = p4;facets[7].p3 = p3;

  n = 8;

  for(i = 1; i<iterations; i++){
    nstart = n;

    for(j = 0; j<nstart; j++){
      /* Create initial copies for the new facets */
      facets[n] = facets[j];
      facets[n+1] = facets[j];
      facets[n+2] = facets[j];

      /* Calculate the midpoints */
      p1 = Midpoint(facets[j].p1, facets[j].p2);
      p2 = Midpoint(facets[j].p2, facets[j].p3);
      p3 = Midpoint(facets[j].p3, facets[j].p1);

      /* Replace the current facet */
      facets[j].p2 = p1;
      facets[j].p3 = p3;

      /* Create the changed vertices in the new facets */
      facets[n].p1 = p1;
      facets[n].p3 = p2;
      facets[n+1].p1 = p3;
      facets[n+1].p2 = p2;
      facets[n+2].p1 = p1;
      facets[n+2].p2 = p2;
      facets[n+2].p3 = p3;
      n += 3;
    }
  }
  for(j = 0; j<n; j++){
    Normalise(&facets[j].p1);
    Normalise(&facets[j].p2);
    Normalise(&facets[j].p3);
  }
  return(n);
}

void CreateCone(){
  float cf = 0.0;
  Vertex t;
  t.color[0] = cf;
  cf = 1. - cf;
  t.color[1] = cf;
  cf = 1. - cf;
  t.color[2] = cf;
  cf = 1. - cf;
  conev.push_back(t); // Apex
  int lod = 32;
  float step = 2. * 3.141596 / float(lod);
  float Radius = 1.;
  for(float a = 0; a <= (2. * 3.141596 + step); a += step) {
    float c = Radius * cos(a);
    float s = Radius * sin(a);
    t.position[0] = c;
    t.position[1] = s;
    t.position[2] = 2.0; // set to 0.0 for a circle, >= 1.0 for a cone.
    t.color[0] = cf;
    cf = 1. - cf;
    t.color[1] = cf;
    cf = 1. - cf;
    t.color[2] = cf;
    cf = 1. - cf;
    conev.push_back(t);
  }
  printf("cone v Size %d\n", conev.size());
}

void CreateSphere(){/* Actually implementing the sphere */
  int i;
  int n = 5;
  Facet *f = NULL;
  f = (Facet *)malloc((int)pow(4,n) * 8 * sizeof(Facet)); // I added *8 because that's the expected number of facets
  n = CreateUnitSphere(n,f);
  printf("%d facets generated\n", n);

  for(i = 0; i<n; i++){
    v.push_back(f[i].p1);
    v.push_back(f[i].p2);
    v.push_back(f[i].p3);
  }
  printf("v Size %d\n", v.size());
}

void CreateCylinder(){
  Vertex t;
  float radius = 1.0, halfLength = 2;
  int slices = 50;
  for(int i = 0; i<slices; i++){
    float theta = ((float)i) * 2.0 * M_PI/slices;
    float nextTheta = ((float)i+1) * 2.0 * M_PI/slices;
    // Vertex at middle of end
    t.position[0]=0.0; t.position[1]=halfLength; t.position[2]=0.0;
    cylinderv.push_back(t);
    // Vertices at edges of circle
    t.position[0]=radius*cos(theta); t.position[1]=halfLength; t.position[2]=radius*sin(theta);
    cylinderv.push_back(t);
    t.position[0]=radius*cos(nextTheta); t.position[1]=halfLength; t.position[2]=radius*sin(nextTheta);
    cylinderv.push_back(t);
    // Same vertices at bottom of cylinder
    t.position[0]=radius*cos(theta); t.position[1]=-halfLength; t.position[2]=radius*sin(theta);
    cylinderv.push_back(t);
    t.position[0]=radius*cos(nextTheta); t.position[1]=-halfLength; t.position[2]=radius*sin(nextTheta);
    cylinderv.push_back(t);
    // Vertex at middle of end of bottom
    t.position[0]=0.0; t.position[1]=-halfLength; t.position[2]=0.0;
    cylinderv.push_back(t);
  }
}

void SetupGeometry() {
  if(mode == 0 || mode == 1){
    int i;
    int n = 5;
    Facet *f = NULL;
    f = (Facet *)malloc((int)pow(4,n) * 8 * sizeof(Facet));
    n = CreateUnitSphere(n,f);
    printf("%d facets generated\n", n);

    for(i = 0; i<n; i++){ // Place vertices into vertex array
      v.push_back(f[i].p1);
      v.push_back(f[i].p2);
      v.push_back(f[i].p3);
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    /* Allocate and assign One Vertex Buffer Object to our handle */
    glGenBuffers(1, vbo);
    /* Bind our VBO as being the active buffer and storing vertex attributes (coordinates + colors) */
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    /* Copy the vertex data from cone to our buffer */
    /* v,size() * sizeof(GLfloat) is the size of the cone array, since it contains 12 Vertex values */
    glBufferData ( GL_ARRAY_BUFFER, v.size() * sizeof ( struct Vertex ), v.data(), GL_STATIC_DRAW );
    /* Specify that our coordinate data is going into attribute index 0, and contains three doubles per vertex */
    /* Note stride = sizeof ( struct Vertex ) and pointer = ( const GLvoid* ) 0 */
    glVertexAttribPointer ( ( GLuint ) 0, 3, GL_FLOAT, GL_FALSE,  sizeof ( struct Vertex ), ( const GLvoid* ) offsetof (struct Vertex, position) );
    /* Enable attribute index 0 as being used */
    glEnableVertexAttribArray(0);
    /* Specify that our color data is going into attribute index 1, and contains three floats per vertex */
    /* Note stride = sizeof ( struct Vertex ) and pointer = ( const GLvoid* ) ( 3 * sizeof ( GLdouble ) ) i.e. the size (in bytes)
       occupied by the first attribute (position) */
    glVertexAttribPointer ( ( GLuint ) 1, 3, GL_FLOAT, GL_FALSE, sizeof ( struct Vertex ), ( const GLvoid* ) offsetof(struct Vertex, color) );   // bug );
    /* Enable attribute index 1 as being used */
    glEnableVertexAttribArray ( 1 );  /* Bind our second VBO as being the active buffer and storing vertex attributes (colors) */
    glBindVertexArray(0);
  }

  if(mode == 2){
    CreateSphere();
    CreateCone();
    CreateCylinder();

    // VAO settings for sphere
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData ( GL_ARRAY_BUFFER, v.size() * sizeof ( struct Vertex ), v.data(), GL_STATIC_DRAW );
    glVertexAttribPointer ( ( GLuint ) 0, 3, GL_FLOAT, GL_FALSE,  sizeof ( struct Vertex ), ( const GLvoid* ) offsetof (struct Vertex, position) );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer ( ( GLuint ) 1, 3, GL_FLOAT, GL_FALSE, sizeof ( struct Vertex ), ( const GLvoid* ) offsetof(struct Vertex, color) );   // bug );
    glEnableVertexAttribArray ( 1 );
    glBindVertexArray(0);

    // VAO settings for cone
    glGenVertexArrays(1, &conevao);
    glBindVertexArray(conevao);
    glGenBuffers(1, conevbo);
    glBindBuffer(GL_ARRAY_BUFFER, conevbo[0]);
    glBufferData ( GL_ARRAY_BUFFER, conev.size() * sizeof ( struct Vertex ), conev.data(), GL_STATIC_DRAW );
    glVertexAttribPointer ( ( GLuint ) 0, 3, GL_FLOAT, GL_FALSE,  sizeof ( struct Vertex ), ( const GLvoid* ) offsetof (struct Vertex, position) );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer ( ( GLuint ) 1, 3, GL_FLOAT, GL_FALSE, sizeof ( struct Vertex ), ( const GLvoid* ) offsetof(struct Vertex, color) );
    glEnableVertexAttribArray ( 1 );
    glBindVertexArray(0);

    // VAO settings for cylinder
    glGenVertexArrays(1, &cylindervao);
    glBindVertexArray(cylindervao);
    glGenBuffers(1, cylindervbo);
    glBindBuffer(GL_ARRAY_BUFFER, cylindervbo[0]);
    glBufferData ( GL_ARRAY_BUFFER, cylinderv.size() * sizeof ( struct Vertex ), cylinderv.data(), GL_STATIC_DRAW );
    glVertexAttribPointer ( ( GLuint ) 0, 3, GL_FLOAT, GL_FALSE,  sizeof ( struct Vertex ), ( const GLvoid* ) offsetof (struct Vertex, position) );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer ( ( GLuint ) 1, 3, GL_FLOAT, GL_FALSE, sizeof ( struct Vertex ), ( const GLvoid* ) offsetof(struct Vertex, color) );
    glEnableVertexAttribArray ( 1 );
    glBindVertexArray(0);
  }
}

void SetupShaders(void) {
  /* Read our shaders into the appropriate buffers */
  vertexsource = filetobuf("./mode1_mode3.vert");
  fragmentsource = filetobuf("./mode1_mode3.frag");
  /* Assign our handles a "name" to new shader objects */
  vertexshader = glCreateShader(GL_VERTEX_SHADER);
  fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
  /* Associate the source code buffers with each handle */
  glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
  glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
  /* Compile our shader objects */
  glCompileShader(vertexshader);
  glCompileShader(fragmentshader);
  /* Assign our program handle a "name" */
  shaderprogram = glCreateProgram();
  glAttachShader(shaderprogram, vertexshader);  /* Attach our shaders to our program */
  glAttachShader(shaderprogram, fragmentshader);
  glBindAttribLocation(shaderprogram, 0, "in_Position");   /* Bind attribute 0 (coordinates) to in_Position and attribute 1 (colors) to in_Color */
  glBindAttribLocation(shaderprogram, 1, "in_Color");
  glLinkProgram(shaderprogram);  /* Link our program, and set it as being actively used */
  CheckShader(shaderprogram, "Basic Shader");
  glUseProgram(shaderprogram);
}

void SetupShaders2(void) {
  vertexsource = filetobuf("./mode2.vert");
  fragmentsource = filetobuf("./mode2.frag");
  vertexshader = glCreateShader(GL_VERTEX_SHADER);
  fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
  glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
  glCompileShader(vertexshader);
  glCompileShader(fragmentshader);
  shaderprogram = glCreateProgram();
  glAttachShader(shaderprogram, vertexshader);
  glAttachShader(shaderprogram, fragmentshader);
  glBindAttribLocation(shaderprogram, 0, "in_Position");
  glBindAttribLocation(shaderprogram, 1, "in_Color");
  glLinkProgram(shaderprogram);
  CheckShader(shaderprogram, "Basic Shader");
  glUseProgram(shaderprogram);
}

void Render() {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  GLfloat angle;
  glm::mat4 Projection = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);
  float t = glfwGetTime();
  float p = 400.;
  t = fmod(t, p);
  angle = t * 360. / p;
  glm::mat4 View = glm::mat4(1.);
  glm::mat4 Model = glm::mat4(1.0);
  if((mode == 0)||(mode == 1)){
    if(mode == 0){ /* Draw a wireframe sphere */
      View = glm::translate(View, glm::vec3(0.f, 0.f, -5.0f));
      View = glm::rotate(View, angle * -1.0f, glm::vec3(1.f, 0.f, 0.f));
      View = glm::rotate(View, angle * 0.5f, glm::vec3(0.f, 1.f, 0.f));
      View = glm::rotate(View, angle * 0.5f, glm::vec3(0.f, 0.f, 1.f));
      glm::mat4 Model = glm::mat4(1.0);
    }
    if(mode == 1){ /* Draw a sphere with lighting */
      View = glm::translate(View, glm::vec3(0.f, 0.f, -5.0f));
      Model = glm::rotate(Model, angle * -1.0f, glm::vec3(0.f, 0.f, 1.f));
    }
    glm::mat4 MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    /* Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram */
    glClearColor(0.0, 0.0, 0.0, 1.0);  /* Make our background black */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao);
    if(mode == 0){
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawArrays(GL_TRIANGLES, 0, v.size());
    }
    if(mode == 1){
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDrawArrays(GL_TRIANGLE_FAN, 0, v.size());
    }
    glBindVertexArray(0);
  }

  if(mode == 2){ /* Draw a basic wireframe rocket */
    // Draw the sphere
    glm::mat4 View = glm::mat4(1.);
    View = glm::translate(View, glm::vec3(0.f, 0.f, -5.0f));
    View = glm::scale(View, glm::vec3(0.5f, 0.5f, 0.5f));
    View = glm::rotate(View, angle * -1.0f, glm::vec3(1.f, 0.f, 0.f));
    View = glm::rotate(View, angle * 0.5f, glm::vec3(0.f, 1.f, 0.f));
    View = glm::rotate(View, angle * 0.5f, glm::vec3(0.f, 0.f, 1.f));
    glm::mat4 Model = glm::mat4(1.0);
    glm::mat4 MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    glClearColor(0.0, 0.0, 0.0, 1.0);  /* Make our background black. Do NOT use when drawing several objects */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, v.size());
    glBindVertexArray(0);

    // Draw the cylinder
    Model = glm::translate(Model, glm::vec3(0.f, 0.f, 0.f));
    GLfloat cylinder_angle = 90.0;
    Model = glm::translate(Model, glm::vec3(0.f, 2.f, 0.f));
    MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    glBindVertexArray(cylindervao);
    glDrawArrays(GL_LINE_STRIP, 0, cylinderv.size());
    glBindVertexArray(0);
    // Draw cone
    Model = glm::mat4(1.0);
    GLfloat cone_angle = M_PI/2;
    Model = glm::rotate(Model, cone_angle * 1.0f, glm::vec3(1.f, 0.f, 0.f));
    Model = glm::translate(Model, glm::vec3(0.f, 0.f, -6.f));
    MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    glBindVertexArray(conevao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, conev.size());
    glBindVertexArray(0);

    // second sphere
    Model = glm::mat4(1.0);
    Model = glm::translate(Model, glm::vec3(1.5f, 3.75f, 0.0f));
    Model = glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.5f));
    MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    glBindVertexArray(vao); // Use vao for spheres, conevao for cones
    glDrawArrays(GL_TRIANGLES, 0, v.size());
    glBindVertexArray(0);

    // third sphere
    Model = glm::mat4(1.0);
    Model = glm::translate(Model, glm::vec3(-1.5f, 3.75f, 0.0f));
    Model = glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.5f));
    MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    glBindVertexArray(vao); // Use vao for spheres, conevao for cones
    glDrawArrays(GL_TRIANGLES, 0, v.size());
    glBindVertexArray(0);

    // second cone
    Model = glm::mat4(1.0);
    Model = glm::rotate(Model, cone_angle * 1.0f, glm::vec3(1.f, 0.f, 0.f));
    Model = glm::translate(Model, glm::vec3(1.5f, 0.f, -5.f));
    Model = glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.5f));
    MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    glBindVertexArray(conevao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, conev.size());
    glBindVertexArray(0);

    // third cone
    Model = glm::mat4(1.0);
    Model = glm::rotate(Model, cone_angle * 1.0f, glm::vec3(1.f, 0.f, 0.f));
    Model = glm::translate(Model, glm::vec3(-1.5f, 0.f, -5.f));
    Model = glm::scale(Model, glm::vec3(0.5f, 0.5f, 0.5f));
    MVP = Projection * View * Model;
    glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "mvpmatrix"), 1, GL_FALSE, glm::value_ptr(MVP));
    glBindVertexArray(conevao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, conev.size());
    glBindVertexArray(0);
  }

}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  if ((key == GLFW_KEY_A) && action == GLFW_PRESS){
    mode = 0;
    SetupGeometry();
    SetupShaders();
  }

  if ((key == GLFW_KEY_B) && action == GLFW_PRESS){
    mode = 1;
    SetupGeometry();
    SetupShaders2();
  }

  if ((key == GLFW_KEY_C) && action == GLFW_PRESS){
    mode = 2;
    SetupGeometry();
    SetupShaders();
  }
}

int main( void ) {
  GLFWwindow* window;
  if( !glfwInit() ) {
    printf("Failed to start GLFW\n");
    exit( EXIT_FAILURE );
  }

#ifdef __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

  window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
  if (!window) {
    glfwTerminate();
    printf("GLFW Failed to start\n");
    return -1;
  }
  /* Make the window's context current */
  glfwMakeContextCurrent(window);

#ifndef __APPLE__
  // IMPORTANT: make window current must be done so glew recognises OpenGL
  glewExperimental = GL_TRUE;
  int err = glewInit();
  if (GLEW_OK != err) {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
#endif

  glfwSetKeyCallback(window, key_callback);
  fprintf(stderr, "GL INFO %s\n", glGetString(GL_VERSION));
  glEnable(GL_DEPTH_TEST);
  SetupGeometry();
  SetupShaders();
  printf("Ready to render\n");
  while(!glfwWindowShouldClose(window)) {  // Main loop
    Render();        // OpenGL rendering goes here...
    glfwSwapBuffers(window);        // Swap front and back rendering buffers
    glfwPollEvents();         // Poll for events.

  }
  glfwTerminate();  // Close window and terminate GLFW
  exit( EXIT_SUCCESS );  // Exit program
}
