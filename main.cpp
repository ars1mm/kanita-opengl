#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#define M_PI 3.14159265358979323846

float cameraRadius = 5.0f;
float cameraHeight = 2.5f;
float cameraAngle = 45.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -20.0f) pitch = -20.0f;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraRadius -= yoffset * 0.5f;
    if (cameraRadius < 2.0f) cameraRadius = 2.0f;
    if (cameraRadius > 10.0f) cameraRadius = 10.0f;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        yaw -= 2.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw += 2.0f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pitch += 2.0f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch -= 2.0f;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -20.0f) pitch = -20.0f;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraHeight += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraHeight -= 0.1f;
    cameraRadius = (cameraRadius < 2.0f) ? 2.0f : (cameraRadius > 10.0f ? 10.0f : cameraRadius);
    cameraHeight = (cameraHeight < 2.0f) ? 2.0f : (cameraHeight > 5.0f ? 5.0f : cameraHeight); 
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraRadius -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraRadius += 0.1f;
}

void applyPerspective(float fov, float aspect, float near, float far) {
    float top = near * tan(fov * 0.5f * M_PI / 180.0f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;
    float matrix[16] = {
        (2.0f * near) / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, (2.0f * near) / (top - bottom), 0.0f, 0.0f,
        (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near), -1.0f,
        0.0f, 0.0f, -(2.0f * far * near) / (far - near), 0.0f
    };
    glMultMatrixf(matrix);
}

void applyLookAt(float eyeX, float eyeY, float eyeZ,
    float centerX, float centerY, float centerZ,
    float upX, float upY, float upZ) {
    float forward[3], up[3], side[3];
    float matrix[16];

    forward[0] = centerX - eyeX;
    forward[1] = centerY - eyeY;
    forward[2] = centerZ - eyeZ;
    float forwardLength = sqrt(forward[0] * forward[0] +
        forward[1] * forward[1] +
        forward[2] * forward[2]);
    forward[0] /= forwardLength;
    forward[1] /= forwardLength;
    forward[2] /= forwardLength;

    up[0] = upX;
    up[1] = upY;
    up[2] = upZ;

    side[0] = forward[1] * up[2] - forward[2] * up[1];
    side[1] = forward[2] * up[0] - forward[0] * up[2];
    side[2] = forward[0] * up[1] - forward[1] * up[0];
    float sideLength = sqrt(side[0] * side[0] +
        side[1] * side[1] +
        side[2] * side[2]);
    side[0] /= sideLength;
    side[1] /= sideLength;
    side[2] /= sideLength;

    up[0] = side[1] * forward[2] - side[2] * forward[1];
    up[1] = side[2] * forward[0] - side[0] * forward[2];
    up[2] = side[0] * forward[1] - side[1] * forward[0];

    matrix[0] = side[0];
    matrix[4] = side[1];
    matrix[8] = side[2];
    matrix[12] = 0.0f;

    matrix[1] = up[0];
    matrix[5] = up[1];
    matrix[9] = up[2];
    matrix[13] = 0.0f;

    matrix[2] = -forward[0];
    matrix[6] = -forward[1];
    matrix[10] = -forward[2];
    matrix[14] = 0.0f;

    matrix[3] = 0.0f;
    matrix[7] = 0.0f;
    matrix[11] = 0.0f;
    matrix[15] = 1.0f;

    glMultMatrixf(matrix);
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}

void drawCube(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    float matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float matShininess[] = { 50.0f };

    if (glIsEnabled(GL_LIGHTING)) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    }
    float size = 0.5f;
    glBegin(GL_QUADS);

    if (glIsEnabled(GL_LIGHTING)) {
        float frontDiffuse[] = { 0.0f, 0.0f, 1.0f, 1.0f };
        float frontAmbient[] = { 0.0f, 0.0f, 0.1f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, frontDiffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, frontAmbient);
    }
    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);

    if (glIsEnabled(GL_LIGHTING)) {
        float backDiffuse[] = { 0.0f, 1.0f, 0.0f, 1.0f };    
        float backAmbient[] = { 0.0f, 0.1f, 0.0f, 1.0f }; 
        glMaterialfv(GL_FRONT, GL_DIFFUSE, backDiffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, backAmbient);
    }
    glNormal3f(0.0f, 0.0f, -1.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, -size, -size);

    // Top face - Yellow
    if (glIsEnabled(GL_LIGHTING)) {
        float topDiffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f }; 
        float topAmbient[] = { 0.1f, 0.1f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, topDiffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, topAmbient);
    }
    glNormal3f(0.0f, 1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-size, size, -size);
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);

    if (glIsEnabled(GL_LIGHTING)) {
        float bottomDiffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };  
        float bottomAmbient[] = { 0.1f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, bottomDiffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, bottomAmbient);
    }
    glNormal3f(0.0f, -1.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);

    if (glIsEnabled(GL_LIGHTING)) {
        float rightDiffuse[] = { 0.8f, 0.0f, 0.8f, 1.0f }; 
        float rightAmbient[] = { 0.1f, 0.0f, 0.1f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, rightDiffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, rightAmbient);
    }
    glNormal3f(1.0f, 0.0f, 0.0f);
    glColor3f(0.8f, 0.0f, 0.8f);
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(size, -size, size);

    if (glIsEnabled(GL_LIGHTING)) {
        float leftDiffuse[] = { 1.0f, 0.5f, 0.0f, 1.0f };  
        float leftAmbient[] = { 0.1f, 0.05f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, leftDiffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, leftAmbient);
    }
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glColor3f(1.0f, 0.5f, 0.0f);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, size, -size);

    glEnd();
    glPopMatrix();
}

void drawPyramid(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    float matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float matShininess[] = { 128.0f };

    if (glIsEnabled(GL_LIGHTING)) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    }

    float size = 0.5f;
    float height = 0.8f;

    glBegin(GL_TRIANGLES);
    if (glIsEnabled(GL_LIGHTING)) {
        float frontMaterial[] = { 0.6f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, frontMaterial);
    }
    glNormal3f(0.0f, 0.5f, 0.5f);
    glColor3f(0.8f, 0.0f, 0.0f);
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);

    if (glIsEnabled(GL_LIGHTING)) {
        float rightMaterial[] = { 0.0f, 0.0f, 0.6f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, rightMaterial);
    }
    glNormal3f(0.5f, 0.5f, 0.0f);
    glColor3f(0.0f, 0.0f, 0.8f);
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(size, -size, size); 
    glVertex3f(size, -size, -size);

    if (glIsEnabled(GL_LIGHTING)) {
        float backMaterial[] = { 0.0f, 0.5f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, backMaterial);
    }
    glNormal3f(0.0f, 0.5f, -0.5f);
    glColor3f(0.0f, 0.7f, 0.0f);
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(size, -size, -size);
    glVertex3f(-size, -size, -size);

    if (glIsEnabled(GL_LIGHTING)) {
        float leftMaterial[] = { 0.6f, 0.6f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, leftMaterial);
    }
    glNormal3f(-0.5f, 0.5f, 0.0f);
    glColor3f(0.8f, 0.8f, 0.0f);
    glVertex3f(0.0f, height, 0.0f);  
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glEnd();

    glBegin(GL_QUADS);
    if (glIsEnabled(GL_LIGHTING)) {
        float bottomMaterial[] = { 0.4f, 0.0f, 0.4f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bottomMaterial);
    }
    glNormal3f(0.0f, -1.0f, 0.0f);
    glColor3f(0.6f, 0.0f, 0.6f);
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);
    glEnd();

    glPopMatrix();
}

void calculateShadowMatrix(GLfloat shadowMat[16], GLfloat groundplane[4], GLfloat lightpos[4]) {
    GLfloat dot;

    dot = groundplane[0] * lightpos[0] +
          groundplane[1] * lightpos[1] +
          groundplane[2] * lightpos[2] +
          groundplane[3] * lightpos[3];

    shadowMat[0]  = dot - lightpos[0] * groundplane[0];
    shadowMat[4]  = 0.f - lightpos[0] * groundplane[1];
    shadowMat[8]  = 0.f - lightpos[0] * groundplane[2];
    shadowMat[12] = 0.f - lightpos[0] * groundplane[3];

    shadowMat[1]  = 0.f - lightpos[1] * groundplane[0];
    shadowMat[5]  = dot - lightpos[1] * groundplane[1];
    shadowMat[9]  = 0.f - lightpos[1] * groundplane[2];
    shadowMat[13] = 0.f - lightpos[1] * groundplane[3];

    shadowMat[2]  = 0.f - lightpos[2] * groundplane[0];
    shadowMat[6]  = 0.f - lightpos[2] * groundplane[1];
    shadowMat[10] = dot - lightpos[2] * groundplane[2];
    shadowMat[14] = 0.f - lightpos[2] * groundplane[3];

    shadowMat[3]  = 0.f - lightpos[3] * groundplane[0];
    shadowMat[7]  = 0.f - lightpos[3] * groundplane[1];
    shadowMat[11] = 0.f - lightpos[3] * groundplane[2];
    shadowMat[15] = dot - lightpos[3] * groundplane[3];
}

void drawGround() {
    glPushMatrix();
    

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBegin(GL_QUADS);
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f);  
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-4.0f, -0.5f, -4.0f);
    glVertex3f(-4.0f, -0.5f, 4.0f);
    glVertex3f(4.0f, -0.5f, 4.0f);
    glVertex3f(4.0f, -0.5f, -4.0f);
    glEnd();
    
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawShadow(void (*drawFunc)(float, float, float), float x, float y, float z) {
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);  

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_TEXTURE_2D);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Shadow matrix
    GLfloat shadowMat[16];
    GLfloat groundplane[] = { 0.0f, 1.0f, 0.0f, 0.5f }; 
    GLfloat lightpos[] = { -2.0f, 3.0f, 4.0f, 1.0f };
    
    calculateShadowMatrix(shadowMat, groundplane, lightpos);
    glMultMatrixf(shadowMat);
    
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    

    float black[] = { 0.0f, 0.0f, 0.0f, 0.7f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    

    drawFunc(x, y, z);
    
    glPopAttrib();
    glPopMatrix();
}

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1440, 1080, "Kanita - OpenGL Projekt", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);  

    float lightPos[] = { -2.0f, 3.0f, 4.0f, 1.0f };
    float lightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };    
    float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };    
    float lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.02f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.002f);

    float globalAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };  
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = 1280.0f / 980.0f;
        applyPerspective(45.0f, aspect, 0.1f, 100.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();


        float yawRad = yaw * M_PI / 180.0f;
        float pitchRad = pitch * M_PI / 180.0f;
        float camX = cameraRadius * cos(pitchRad) * cos(yawRad);
        float camY = cameraRadius * sin(pitchRad) + cameraHeight;
        float camZ = cameraRadius * cos(pitchRad) * sin(yawRad);


        applyLookAt(camX, camY, camZ,
                   0.0f, 0.0f, 0.0f,
                   0.0f, 1.0f, 0.0f);

        float lightPos[] = { camX - 2.0f, camY + 3.0f, camZ + 4.0f, 1.0f };
        float lightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };    
        float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };   
        float lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);


        drawGround();


        drawPyramid(-1.0f, 0.0f, 0.0f);
        drawCube(1.0f, 0.0f, 0.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
