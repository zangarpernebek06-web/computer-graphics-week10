// 10-апта: толық еркін камера - тінтуірмен бағыт (yaw/pitch), дөңгелекпен fov
//
// Басқару: W/A/S/D жылжу, Space/Left Shift жоғары/төмен, Left Ctrl жүгіру,
//          тінтуір - бұрылу, дөңгелек - zoom, I - invert Y, P - күйді шығару,
//          R - бастапқы күй, Esc - шығу.
//
// Тексеру флагтары (тінтуір мен дөңгелекті терминалдан имитациялау):
//   --mouse dx,dy      тінтуір қозғалысы (dy>0 - жоғары), пиксельмен
//   --scroll n         дөңгелек (n>0 - алға)
//   --no-pitch-clamp   1-тапсырма: pitch шектеуі жоқ
//   --hold W,D,...     пернелерді басулы деп алу (W A S D SPACE SHIFT CTRL)
//   --fixed-dt s       dt тұрақты
//   --invert           invertY = true болып басталады
//   --shot f.ppm       10-кадрдан кейін скриншот сақтап, күйді шығарып шығу
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// Куб: 6 қыр x 2 үшбұрыш, сыртынан қарағанда CCW. Позиция (3) + түс (3).
float vertices[] = {
    // +Z алдыңғы
     -0.5f,  -0.5f,   0.5f,  1.0f, 0.2f, 0.2f,
      0.5f,  -0.5f,   0.5f,  1.0f, 0.6f, 0.2f,
      0.5f,   0.5f,   0.5f,  1.0f, 1.0f, 0.2f,
      0.5f,   0.5f,   0.5f,  1.0f, 1.0f, 0.2f,
     -0.5f,   0.5f,   0.5f,  1.0f, 0.4f, 0.4f,
     -0.5f,  -0.5f,   0.5f,  1.0f, 0.2f, 0.2f,
    // -Z артқы
      0.5f,  -0.5f,  -0.5f,  0.2f, 1.0f, 0.2f,
     -0.5f,  -0.5f,  -0.5f,  0.2f, 1.0f, 0.6f,
     -0.5f,   0.5f,  -0.5f,  0.2f, 1.0f, 1.0f,
     -0.5f,   0.5f,  -0.5f,  0.2f, 1.0f, 1.0f,
      0.5f,   0.5f,  -0.5f,  0.4f, 1.0f, 0.4f,
      0.5f,  -0.5f,  -0.5f,  0.2f, 1.0f, 0.2f,
    // -X сол
     -0.5f,  -0.5f,  -0.5f,  0.2f, 0.2f, 1.0f,
     -0.5f,  -0.5f,   0.5f,  0.6f, 0.2f, 1.0f,
     -0.5f,   0.5f,   0.5f,  1.0f, 0.2f, 1.0f,
     -0.5f,   0.5f,   0.5f,  1.0f, 0.2f, 1.0f,
     -0.5f,   0.5f,  -0.5f,  0.4f, 0.4f, 1.0f,
     -0.5f,  -0.5f,  -0.5f,  0.2f, 0.2f, 1.0f,
    // +X оң
      0.5f,  -0.5f,   0.5f,  1.0f, 1.0f, 0.2f,
      0.5f,  -0.5f,  -0.5f,  1.0f, 0.8f, 0.4f,
      0.5f,   0.5f,  -0.5f,  0.8f, 1.0f, 0.2f,
      0.5f,   0.5f,  -0.5f,  0.8f, 1.0f, 0.2f,
      0.5f,   0.5f,   0.5f,  1.0f, 1.0f, 0.6f,
      0.5f,  -0.5f,   0.5f,  1.0f, 1.0f, 0.2f,
    // +Y жоғары
     -0.5f,   0.5f,   0.5f,  0.2f, 1.0f, 1.0f,
      0.5f,   0.5f,   0.5f,  0.2f, 0.8f, 1.0f,
      0.5f,   0.5f,  -0.5f,  0.4f, 1.0f, 1.0f,
      0.5f,   0.5f,  -0.5f,  0.4f, 1.0f, 1.0f,
     -0.5f,   0.5f,  -0.5f,  0.6f, 1.0f, 1.0f,
     -0.5f,   0.5f,   0.5f,  0.2f, 1.0f, 1.0f,
    // -Y төмен
     -0.5f,  -0.5f,  -0.5f,  1.0f, 0.2f, 1.0f,
      0.5f,  -0.5f,  -0.5f,  1.0f, 0.4f, 0.8f,
      0.5f,  -0.5f,   0.5f,  0.8f, 0.2f, 1.0f,
      0.5f,  -0.5f,   0.5f,  0.8f, 0.2f, 1.0f,
     -0.5f,  -0.5f,   0.5f,  1.0f, 0.6f, 1.0f,
     -0.5f,  -0.5f,  -0.5f,  1.0f, 0.2f, 1.0f,
};

glm::vec3 positions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  0.5f, -2.0f),
    glm::vec3(-1.8f, -0.3f, -1.0f),
    glm::vec3( 1.2f, -0.8f, -3.5f),
    glm::vec3(-2.2f,  1.0f, -4.0f)
};

// ---- Камера (9-апта) ----
glm::vec3 camPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp    = glm::vec3(0.0f, 1.0f,  0.0f);

// ---- 10-апта: бағыт және zoom ----
float yaw   = -90.0f;   // 0 болса камера +X-ке қарайды, куб көрінбейді!
float pitch =   0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool  firstMouse = true;
float fov = 45.0f;
const float fovMax = 90.0f;  // 5-тапсырма: жоғарғы шек 45 -> 90

const int gWidth = 800, gHeight = 600;

bool invertY = false;          // 3-тапсырма: I пернесі
bool iPressedLast = false;
bool pPressedLast = false;
bool rPressedLast = false;
bool pitchClamp = true;        // 1-тапсырма үшін --no-pitch-clamp

// Бастапқы күй (R пернесі үшін)
const glm::vec3 camStartPos = glm::vec3(0.0f, 0.0f, 3.0f);
const float yawStart = -90.0f, pitchStart = 0.0f, fovStart = 45.0f;

std::vector<int> heldKeys;
GLFWwindow* gWindow = nullptr;

bool keyDown(int key) {
    for (int k : heldKeys) if (k == key) return true;
    return glfwGetKey(gWindow, key) == GLFW_PRESS;
}

int keyFromName(const std::string& n) {
    if (n == "W") return GLFW_KEY_W;
    if (n == "A") return GLFW_KEY_A;
    if (n == "S") return GLFW_KEY_S;
    if (n == "D") return GLFW_KEY_D;
    if (n == "SPACE") return GLFW_KEY_SPACE;
    if (n == "SHIFT") return GLFW_KEY_LEFT_SHIFT;
    if (n == "CTRL") return GLFW_KEY_LEFT_CONTROL;
    return GLFW_KEY_UNKNOWN;
}

// yaw мен pitch бұрыштарынан camFront есептеу (градус -> радиан!)
void updateFront() {
    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(dir);
}

void printState() {
    std::printf("camPos   = (%.2f, %.2f, %.2f)\n", camPos.x, camPos.y, camPos.z);
    std::printf("camFront = (%.2f, %.2f, %.2f)\n", camFront.x, camFront.y, camFront.z);
    std::printf("yaw = %.1f   pitch = %.1f   fov = %.1f\n", yaw, pitch, fov);
    std::printf("length = %.7f\n", glm::length(camFront));
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    if (firstMouse) {  // алғашқы оқиғада камера "лақтырылмайды"
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;  // экранның y-і төмен қарай өседі
    if (invertY) yoffset = -yoffset;
    lastX = (float)xpos;
    lastY = (float)ypos;

    const float sensitivity = 0.1f;
    yaw   += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    // Қ4: yaw шектеусіз өспесін
    if (yaw >  360.0f) yaw -= 360.0f;
    if (yaw < -360.0f) yaw += 360.0f;

    if (pitchClamp) {
        if (pitch >  89.0f) pitch =  89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }
    updateFront();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window; (void)xoffset;
    fov -= (float)yoffset;  // алға айналдыру -> fov азаяды -> жақындату
    if (fov < 1.0f) fov = 1.0f;
    if (fov > fovMax) fov = fovMax;
}

void resetCamera() {
    camPos = camStartPos;
    yaw = yawStart;
    pitch = pitchStart;
    fov = fovStart;
    updateFront();
}

void processInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    float speed = 2.5f * dt;
    if (keyDown(GLFW_KEY_LEFT_CONTROL)) speed *= 2.0f;

    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    if (keyDown(GLFW_KEY_W)) camPos += speed * camFront;
    if (keyDown(GLFW_KEY_S)) camPos -= speed * camFront;
    if (keyDown(GLFW_KEY_A)) camPos -= right * speed;
    if (keyDown(GLFW_KEY_D)) camPos += right * speed;
    if (keyDown(GLFW_KEY_SPACE))      camPos += speed * glm::vec3(0.0f, 1.0f, 0.0f);
    if (keyDown(GLFW_KEY_LEFT_SHIFT)) camPos -= speed * glm::vec3(0.0f, 1.0f, 0.0f);

    // I - invert Y: алдыңғы күйді есте сақтаймыз, әйтпесе перне басулы тұрғанда
    // әр кадр сайын ауысып кетер еді (WASD-та мұндай мәселе жоқ: ол күй емес,
    // "басулы тұр" дегенді әр кадр оқиды)
    bool iNow = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    if (iNow && !iPressedLast) invertY = !invertY;
    iPressedLast = iNow;

    // P - камера күйін шығару
    bool pNow = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (pNow && !pPressedLast) printState();
    pPressedLast = pNow;

    // Қ3: R - бастапқы күй
    bool rNow = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (rNow && !rPressedLast) resetCamera();
    rPressedLast = rNow;
}

const char* vsSrc = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 vColor;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vColor = aColor;
})";

const char* fsSrc = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() { FragColor = vec4(vColor, 1.0); })";

GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, 1024, nullptr, log);
        std::fprintf(stderr, "Shader error: %s\n", log);
    }
    return s;
}

void saveShot(const char* path, int w, int h) {
    std::vector<unsigned char> px(w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    FILE* f = std::fopen(path, "wb");
    if (!f) return;
    std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int y = h - 1; y >= 0; y--) std::fwrite(&px[y * w * 3], 1, w * 3, f);
    std::fclose(f);
}

int main(int argc, char** argv) {
    const char* shotPath = nullptr;
    float fixedDt = 0.0f, simDx = 0.0f, simDy = 0.0f, simScroll = 0.0f;
    bool simMouse = false;
    for (int i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--shot") && i + 1 < argc) shotPath = argv[++i];
        else if (!std::strcmp(argv[i], "--fixed-dt") && i + 1 < argc) fixedDt = (float)std::atof(argv[++i]);
        else if (!std::strcmp(argv[i], "--no-pitch-clamp")) pitchClamp = false;
        else if (!std::strcmp(argv[i], "--invert")) invertY = true;
        else if (!std::strcmp(argv[i], "--scroll") && i + 1 < argc) simScroll = (float)std::atof(argv[++i]);
        else if (!std::strcmp(argv[i], "--mouse") && i + 1 < argc) {
            std::sscanf(argv[++i], "%f,%f", &simDx, &simDy);
            simMouse = true;
        } else if (!std::strcmp(argv[i], "--hold") && i + 1 < argc) {
            std::string list = argv[++i], tok;
            for (size_t p = 0; p <= list.size(); p++) {
                if (p == list.size() || list[p] == ',') {
                    int k = keyFromName(tok);
                    if (k != GLFW_KEY_UNKNOWN) heldKeys.push_back(k);
                    tok.clear();
                } else tok += list[p];
            }
        }
    }

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Week 10 - Mouse look", nullptr, nullptr);
    if (!window) { std::fprintf(stderr, "Window error\n"); glfwTerminate(); return -1; }
    gWindow = window;
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::fprintf(stderr, "GLEW error\n"); return -1; }

    // 2-қадам: курсорды ұстау және callback-тер (скриншот режимінде курсорды ұстамаймыз)
    // (скриншот режимінде нақты тінтуір оқиғалары тіркелмейді, имитация қолданылады)
    if (!shotPath) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);
    GLuint shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int locModel = glGetUniformLocation(shader, "model");
    int locView  = glGetUniformLocation(shader, "view");
    int locProj  = glGetUniformLocation(shader, "projection");

    updateFront();  // yaw=-90, pitch=0 -> camFront = (0,0,-1)

    // Имитацияланған тінтуір/дөңгелек (тек тексеру үшін)
    if (simMouse) {
        mouse_callback(window, 400.0, 300.0);                 // firstMouse
        mouse_callback(window, 400.0 + simDx, 300.0 - simDy); // dy>0 - жоғары
    }
    if (simScroll != 0.0f) scroll_callback(window, 0.0, simScroll);

    float lastTime = (float)glfwGetTime();
    int frame = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float now = (float)glfwGetTime();
        float dt = fixedDt > 0.0f ? fixedDt : now - lastTime;
        lastTime = now;

        processInput(window, dt);

        // Қ2: бұрыштарды терезе тақырыбында көрсету
        char buf[128];
        std::snprintf(buf, sizeof(buf), "yaw=%.1f  pitch=%.1f  fov=%.1f", yaw, pitch, fov);
        glfwSetWindowTitle(window, buf);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        // 5-қадам: fov perspective-ке байланады (тұрақты 45 емес)
        glm::mat4 projection = glm::perspective(glm::radians(fov),
            (float)gWidth / (float)gHeight, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(vao);
        for (int i = 0; i < 5; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, positions[i]);
            model = glm::rotate(model, now * (0.5f + i * 0.3f), glm::vec3(0.3f, 1.0f, 0.0f));
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        frame++;
        if (shotPath && frame == 10) {
            saveShot(shotPath, gWidth, gHeight);
            printState();
            std::printf("view has NaN: %s\n", std::isnan(view[0][0] + view[1][1] + view[2][2]) ? "yes" : "no");
            break;
        }
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}
