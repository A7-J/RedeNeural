#include "MotorGrafico.h"
#include <GL/glut.h>
#include <cmath>
#include <string>

bool neuronios[3] = {false, false, false};
int contadores[3] = {0, 0, 0};

// câmera
float anguloX = 0.0f;
float anguloY = 0.0f;
int mouseX, mouseY;
bool mousePressionado = false;

// sinais
struct Sinal {
    bool ativo = false;
    float progresso = 0.0f;
    float x1, y1, x2, y2;
};
Sinal sinais[2];

void configurarLuz() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat luzPosicao[]   = { 2.0f, 2.0f, 3.0f, 1.0f };
    GLfloat luzAmbiente[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat luzDifusa[]    = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat luzEspecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, luzPosicao);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  luzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  luzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);

    GLfloat especular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
    glMateriali(GL_FRONT, GL_SHININESS, 80);
}

void desenharTexto2D(float x, float y, const char* texto) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    while (*texto)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *texto++);
    glEnable(GL_LIGHTING);
}

void desenharTexto(float x, float y, float z, const char* texto) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos3f(x, y, z);
    while (*texto)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *texto++);
    glEnable(GL_LIGHTING);
}

void desenharEsfera(float x, float y, float z, bool ativo) {
    glPushMatrix();
    glTranslatef(x, y, z);
    if (ativo)
        glColor3f(1.0f, 1.0f, 0.0f);
    else
        glColor3f(0.3f, 0.3f, 0.9f);
    glutSolidSphere(0.15, 32, 32);
    glPopMatrix();
}

void desenharConexao(float x1, float y1, float x2, float y2) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex3f(x1, y1, 0.0f);
        glVertex3f(x2, y2, 0.0f);
    glEnd();
    glEnable(GL_LIGHTING);
}

void desenharSinais() {
    glDisable(GL_LIGHTING);
    for (int i = 0; i < 2; i++) {
        if (!sinais[i].ativo) continue;
        float x = sinais[i].x1 + (sinais[i].x2 - sinais[i].x1) * sinais[i].progresso;
        float y = sinais[i].y1 + (sinais[i].y2 - sinais[i].y1) * sinais[i].progresso;
        glColor3f(0.0f, 1.0f, 1.0f);
        glPointSize(10.0f);
        glBegin(GL_POINTS);
            glVertex3f(x, y, 0.0f);
        glEnd();
    }
    glEnable(GL_LIGHTING);
}

void desenharContadores() {
    // muda pra projeção 2D pra desenhar o HUD
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 600, 0, 400);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // título
    glColor3f(0.0f, 1.0f, 1.0f);
    glRasterPos2f(10, 380);
    const char* titulo = "Disparos:";
    while (*titulo)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *titulo++);

    // contadores A, B, C
    const char* nomes[] = {"A: ", "B: ", "C: "};
    for (int i = 0; i < 3; i++) {
        std::string linha = nomes[i] + std::to_string(contadores[i]);
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(10, 360 - i * 20);
        for (char c : linha)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void atualizar(int valor) {
    for (int i = 0; i < 2; i++) {
        if (!sinais[i].ativo) continue;
        sinais[i].progresso += 0.05f;
        if (sinais[i].progresso >= 1.0f) {
            sinais[i].ativo = false;
            sinais[i].progresso = 0.0f;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(30, atualizar, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);
    glRotatef(anguloX, 1.0f, 0.0f, 0.0f);
    glRotatef(anguloY, 0.0f, 1.0f, 0.0f);

    configurarLuz();

    desenharConexao(-0.6f, 0.0f, 0.0f, 0.0f);
    desenharConexao( 0.0f, 0.0f, 0.6f, 0.0f);

    desenharEsfera(-0.6f, 0.0f, 0.0f, neuronios[0]);
    desenharEsfera( 0.0f, 0.0f, 0.0f, neuronios[1]);
    desenharEsfera( 0.6f, 0.0f, 0.0f, neuronios[2]);

    desenharSinais();

    desenharTexto(-0.63f, 0.22f, 0.0f, "A");
    desenharTexto(-0.03f, 0.22f, 0.0f, "B");
    desenharTexto( 0.57f, 0.22f, 0.0f, "C");

    desenharContadores();

    glutSwapBuffers();
}

void mouseClick(int botao, int estado, int x, int y) {
    if (botao == GLUT_LEFT_BUTTON) {
        mousePressionado = (estado == GLUT_DOWN);
        mouseX = x;
        mouseY = y;
    }
}

void mouseMove(int x, int y) {
    if (!mousePressionado) return;
    anguloY += (x - mouseX) * 0.5f;
    anguloX += (y - mouseY) * 0.5f;
    mouseX = x;
    mouseY = y;
    glutPostRedisplay();
}

JNIEXPORT void JNICALL Java_MotorGrafico_inicializar(JNIEnv*, jobject, jint w, jint h) {
    int argc = 0;
    glutInit(&argc, nullptr);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    glutCreateWindow("Rede Neural 3D - RedeNeural");

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w/h, 0.1, 100.0);

    sinais[0] = {false, 0.0f, -0.6f, 0.0f, 0.0f, 0.0f};
    sinais[1] = {false, 0.0f,  0.0f, 0.0f, 0.6f, 0.0f};

    glutDisplayFunc(display);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMove);
    glutTimerFunc(30, atualizar, 0);
    glutMainLoop();
}

JNIEXPORT void JNICALL Java_MotorGrafico_atualizarNeuronio(JNIEnv*, jobject, jint id, jboolean ativo) {
    if (id >= 0 && id < 3)
        neuronios[id] = ativo;
    if (ativo && id < 2) {
        sinais[id].ativo = true;
        sinais[id].progresso = 0.0f;
    }
    glutPostRedisplay();
}

JNIEXPORT void JNICALL Java_MotorGrafico_atualizarContador(JNIEnv*, jobject, jint id, jint valor) {
    if (id >= 0 && id < 3)
        contadores[id] = valor;
    glutPostRedisplay();
}