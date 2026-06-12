#include "MotorGrafico.h"
#include <GL/glut.h>
#include <cmath>
#include <string>

#define NUM_NEURONIOS 6

bool neuronios[NUM_NEURONIOS] = {false};
int contadores[NUM_NEURONIOS] = {0};

// posições das esferas por camada
float posX[] = {-0.8f, -0.8f,  0.0f,  0.0f,  0.8f,  0.8f};
float posY[] = { 0.3f, -0.3f,  0.3f, -0.3f,  0.3f, -0.3f};
const char* nomes[] = {"A", "B", "C", "D", "E", "F"};

// câmera
float anguloX = 0.0f;
float anguloY = 0.0f;
int mouseX, mouseY;
bool mousePressionado = false;

// sinais — uma conexão por par
struct Sinal {
    bool ativo = false;
    float progresso = 0.0f;
    float x1, y1, x2, y2;
};

// A→C, A→D, B→C, B→D, C→E, C→F, D→E, D→F
Sinal sinais[8];

void inicializarSinais() {
    sinais[0] = {false, 0.0f, posX[0], posY[0], posX[2], posY[2]}; // A→C
    sinais[1] = {false, 0.0f, posX[0], posY[0], posX[3], posY[3]}; // A→D
    sinais[2] = {false, 0.0f, posX[1], posY[1], posX[2], posY[2]}; // B→C
    sinais[3] = {false, 0.0f, posX[1], posY[1], posX[3], posY[3]}; // B→D
    sinais[4] = {false, 0.0f, posX[2], posY[2], posX[4], posY[4]}; // C→E
    sinais[5] = {false, 0.0f, posX[2], posY[2], posX[5], posY[5]}; // C→F
    sinais[6] = {false, 0.0f, posX[3], posY[3], posX[4], posY[4]}; // D→E
    sinais[7] = {false, 0.0f, posX[3], posY[3], posX[5], posY[5]}; // D→F
}

// mapa: qual neurônio dispara → quais sinais ativar
int sinalPorNeuronio[6][2] = {
    {0, 1}, // A → sinais 0,1
    {2, 3}, // B → sinais 2,3
    {4, 5}, // C → sinais 4,5
    {6, 7}, // D → sinais 6,7
    {-1,-1},// E → saída, sem sinais
    {-1,-1} // F → saída, sem sinais
};

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

void desenharTexto(float x, float y, float z, const char* texto) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos3f(x, y, z);
    while (*texto)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *texto++);
    glEnable(GL_LIGHTING);
}

void desenharEsfera(int id) {
    glPushMatrix();
    glTranslatef(posX[id], posY[id], 0.0f);
    if (neuronios[id])
        glColor3f(1.0f, 1.0f, 0.0f); // amarelo
    else if (id < 2)
        glColor3f(0.9f, 0.3f, 0.3f); // vermelho = entrada
    else if (id < 4)
        glColor3f(0.3f, 0.9f, 0.3f); // verde = oculta
    else
        glColor3f(0.3f, 0.3f, 0.9f); // azul = saída
    glutSolidSphere(0.12, 32, 32);
    glPopMatrix();
}

void desenharConexoes() {
    glDisable(GL_LIGHTING);
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(1.5f);
    for (int i = 0; i < 8; i++) {
        glBegin(GL_LINES);
            glVertex3f(sinais[i].x1, sinais[i].y1, 0.0f);
            glVertex3f(sinais[i].x2, sinais[i].y2, 0.0f);
        glEnd();
    }
    glEnable(GL_LIGHTING);
}

void desenharSinais() {
    glDisable(GL_LIGHTING);
    for (int i = 0; i < 8; i++) {
        if (!sinais[i].ativo) continue;
        float x = sinais[i].x1 + (sinais[i].x2 - sinais[i].x1) * sinais[i].progresso;
        float y = sinais[i].y1 + (sinais[i].y2 - sinais[i].y1) * sinais[i].progresso;
        glColor3f(0.0f, 1.0f, 1.0f);
        glPointSize(8.0f);
        glBegin(GL_POINTS);
            glVertex3f(x, y, 0.0f);
        glEnd();
    }
    glEnable(GL_LIGHTING);
}

void desenharContadores() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 700, 0, 500);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // título
    glColor3f(0.0f, 1.0f, 1.0f);
    glRasterPos2f(10, 480);
    const char* titulo = "Disparos:";
    while (*titulo)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *titulo++);

    // legenda camadas
    glColor3f(0.9f, 0.3f, 0.3f);
    glRasterPos2f(10, 460);
    const char* l1 = "Entrada: A B";
    while (*l1) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *l1++);

    glColor3f(0.3f, 0.9f, 0.3f);
    glRasterPos2f(10, 445);
    const char* l2 = "Oculta:  C D";
    while (*l2) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *l2++);

    glColor3f(0.3f, 0.3f, 0.9f);
    glRasterPos2f(10, 430);
    const char* l3 = "Saida:   E F";
    while (*l3) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *l3++);

    // contadores
    for (int i = 0; i < NUM_NEURONIOS; i++) {
        std::string linha = std::string(nomes[i]) + ": " + std::to_string(contadores[i]);
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(10, 405 - i * 18);
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
    for (int i = 0; i < 8; i++) {
        if (!sinais[i].ativo) continue;
        sinais[i].progresso += 0.04f;
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
    desenharConexoes();

    for (int i = 0; i < NUM_NEURONIOS; i++)
        desenharEsfera(i);

    desenharSinais();

    for (int i = 0; i < NUM_NEURONIOS; i++)
        desenharTexto(posX[i] - 0.03f, posY[i] + 0.17f, 0.0f, nomes[i]);

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

    inicializarSinais();

    glutDisplayFunc(display);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMove);
    glutTimerFunc(30, atualizar, 0);
    glutMainLoop();
}

JNIEXPORT void JNICALL Java_MotorGrafico_atualizarNeuronio(JNIEnv*, jobject, jint id, jboolean ativo) {
    if (id >= 0 && id < NUM_NEURONIOS)
        neuronios[id] = ativo;

    if (ativo && id < NUM_NEURONIOS) {
        for (int s = 0; s < 2; s++) {
            int idx = sinalPorNeuronio[id][s];
            if (idx >= 0) {
                sinais[idx].ativo = true;
                sinais[idx].progresso = 0.0f;
            }
        }
    }
    glutPostRedisplay();
}

JNIEXPORT void JNICALL Java_MotorGrafico_atualizarContador(JNIEnv*, jobject, jint id, jint valor) {
    if (id >= 0 && id < NUM_NEURONIOS)
        contadores[id] = valor;
    glutPostRedisplay();
}