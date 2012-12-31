/*
 * example2.c
 * By Steven Smith
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sst.h"

static const char *shaders[] = {"shaders/test1.vert", "shaders/test1.frag"};

GLFWwindow initialize() {
    GLFWwindow window;
    /* Hard-coded values for now */
    if( !glfwInit() ) {
        printf("Failed to init GLFW!\n");
        return NULL;
    }
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    if( (window = glfwCreateWindow(600, 600, GLFW_WINDOWED, "GLFW Window", NULL))
        == NULL ) {
        printf("Failed to open window!\n");
        printf("Error: %s\n", glfwErrorString(glfwGetError()));
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    return window;
}

static GLfloat positions[] = {  1.0f,  1.0f,  1.0f,
                                1.0f,  1.0f, -1.0f,
                               -1.0f,  1.0f, -1.0f,
                               -1.0f,  1.0f,  1.0f,
                                1.0f, -1.0f,  1.0f,
                                1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f, -1.0f,
                               -1.0f, -1.0f,  1.0f };

#define TRIANGLE_COUNT 36

static int triangles[] = { 4, 3, 0,
                           3, 2, 1,
                           1, 2, 5,
                           2, 6, 5,
                           4, 5, 6,
                           7, 4, 6,
                           2, 3, 7,
                           7, 6, 2,
                           1, 5, 4,
                           4, 7, 3,
                           0, 1, 4,
                           3, 1, 0 };

/*
static int triangles[] = { 3, 2, 1,
                           1, 2, 5,
                           2, 6, 5,
                           4, 5, 6,
                           7, 4, 6,
                           2, 3, 7,
                           7, 6, 2,
                           1, 5, 4,
                           4, 7, 3,
                           4, 3, 0,
                           0, 1, 4,
                           3, 1, 0 };
*/

static GLfloat * generateVertices() {
    GLfloat *result;
    unsigned int i;
    result = (GLfloat*)malloc(sizeof(GLfloat) * TRIANGLE_COUNT * 3);
    for( i = 0; i < TRIANGLE_COUNT; i++ ) {
        result[i*3] = positions[triangles[i]*3];
        result[i*3 + 1] = positions[triangles[i]*3 + 1];
        result[i*3 + 2] = positions[triangles[i]*3 + 2];
    }
    return result;
}

static GLfloat * generateNormals( GLfloat *verts ) {
    GLfloat *result;
    unsigned int i;
    result = (GLfloat*)malloc(sizeof(GLfloat) * TRIANGLE_COUNT * 3);
    memcpy(result, verts, sizeof(GLfloat) * TRIANGLE_COUNT * 3);
    for( i = 0; i < TRIANGLE_COUNT; i++ ) {
        sstNormalize3_(&result[i*3]);
    }
    return result;
}

int main( void ) {
    sstProgram *program;
    GLFWwindow window;
    GLfloat *verts, *norms, *proj, *rotY, *rotX, *trans, *scale;

    if( (window = initialize()) == NULL ) {
        exit(EXIT_FAILURE);
    }

    program = sstNewProgram(shaders, 2);
    if( !program ) {
        printf("Failed to start: couldn't create program!\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Set up data */
    verts = generateVertices();
    norms = generateNormals(verts);
    /* Set up uniforms */
    proj = sstPerspectiveMatrix(60.0f, 1.0f, 5.0f, 505.0f);
    rotY = sstRotateMatrix(0.0f, 1.0f, 0.0f, 0.0f);
    rotX = sstRotateMatrix(0.0f, 0.0f, 1.0f, 0.0f);
    trans = sstTranslateMatrix(0.0f, 0.0f, -10.0f);
    scale = sstScaleMatrix(2.0f, 2.0f, 2.0f);
    //proj = sstIdentityMatrix4x4();
    //rotY = sstIdentityMatrix4x4();
    //rotX = sstIdentityMatrix4x4();
    //trans = sstIdentityMatrix4x4();
    //scale = sstIdentityMatrix4x4();

    sstActivateProgram(program);

    sstSetInputData(program, "in_Position", verts, TRIANGLE_COUNT);
    sstSetInputData(program, "in_Normal", norms, TRIANGLE_COUNT);
    sstSetUniformData(program, "projectionMatrix", proj);
    sstSetUniformData(program, "rotateYMatrix", rotY);
    sstSetUniformData(program, "rotateXMatrix", rotX);
    sstSetUniformData(program, "translateMatrix", trans);
    sstSetUniformData(program, "scaleMatrix", scale);

    glViewport(0, 0, 600, 600);

    while( 1 ) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        if( sstDisplayErrors() ) {
            break;
        }

        glFlush();
        glfwSwapBuffers(window);
        glfwPollEvents();
        if( glfwGetKey(window, GLFW_KEY_ESC)
         || glfwGetWindowParam(window, GLFW_CLOSE_REQUESTED) ) {
            break;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
