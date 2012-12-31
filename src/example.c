/*
 * example.c
 * By Steven Smith
 */

#include <stdlib.h>
#include <stdio.h>
#include "sst.h"

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

static GLfloat positions[] = { -1.0f, -1.0f, 1.0f,
                                1.0f, -1.0f, 1.0f,
                                0.0f,  1.0f, 1.0f };

static GLfloat colors[] = { 1.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 1.0f };

static GLfloat scaleMatrix[] = { 0.5f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 0.5f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 0.5f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 1.0f };

static const char *shaders[] = {"shaders/simple.vert", "shaders/simple.frag"};

int main( void ) {
    sstProgram *program;
    GLFWwindow window;

    if( (window = initialize()) == NULL ) {
        exit(EXIT_FAILURE);
    }

    program = sstNewProgram(shaders, 2);
    if( !program ) {
        printf("Failed to start: couldn't create program!\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    sstActivateProgram(program);

    sstSetInputData(program, "in_Position", positions, 3);
    sstSetInputData(program, "in_Color", colors, 3);
    sstSetUniformData(program, "scaleMat", scaleMatrix);

    glViewport(0, 0, 600, 600);

    while( 1 ) {
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

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
