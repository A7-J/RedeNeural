#include "MotorGrafico.h"
#include <GL/glut.h>
#include <cmath>
#include <string>

#define NUM_NEURONIOS 6

bool neuronios[NUM_NEURONIOS] = {false};
int contadores[NUM_NEURONIOS] = {0};

float posX[] = {-0.8f, -0.8f,  0.0f,  0.0f,  0.8f,  0.8f};
float posY[] = { 0.3f, -0.3f,  0.3f, -0.3f,  0.3f, -0.3f};
const char* nomes[] = {"A", "B", "C", "D", "E", "F"};

// câmera
float anguloX = 0.0f;
float anguloY = 0.0f;
int mouseX, mouseY;
bool mousePressionado = false;

// JNI callback
static JNIEnv* jniEnv = nullptr;
static jobject mainObj = nullptr;
static jmethodID estimularMethod = nullptr;

// sinais
struct Sinal {
    bool ativo = false;
    float progresso = 0.0f;
    float x1, y1, x2, y2;
};
Sinal sinais[8];

int sinalPorNeuronio[6][2] = {
    {0, 1}, {2, 3}, {4, 5}, {6, 7}, {-1,-1}, {-1,-1}
};

void inicializarSinais() {
    sinais[0] = {false, 0.0f, posX[0], posY[0], posX[2], posY[2]};
    sinais[1] = {false, 0.0f, posX[0], posY[0], posX[3], posY[3]};
    sinais[2] = {false, 0.0f, posX[1], posY[1], posX[2], posY[2]};
    sinais[3] = {false, 0.0f, posX[1], posY[1], posX[3], posY[3]};
    sinais[4] = {false, 0.0f, posX[2], posY[2], posX[4], posY[4]};
    sinais[5] = {false, 0.0f, posX[2], posY[2], posX[5], posY[5]};
    sinais[6] = {false, 0.0f, posX[3], posY[3], posX[4], posY[4]};
    sinais[7] = {false, 0.0f, posX[3], posY[3], posX[5], posY[5]};
}

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
        glColor3f(1.0f, 1.0f, 0.0f);
    else if (id < 2)
        glColor3f(0.9f, 0.3f, 0.3f);
    else if (id < 4)
        glColor3f(0.3f, 0.9f, 0.3f);
    else
        glColor3f(0.3f, 0.3f, 0.9f);
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

void desenharHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 700, 0, 500);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // disparos
    glColor3f(0.0f, 1.0f, 1.0f);
    glRasterPos2f(10, 480);
    const char* t = "Disparos:";
    while (*t) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *t++);

    glColor3f(0.9f, 0.3f, 0.3f);
    glRasterPos2f(10, 462);
    const char* l1 = "Entrada: A B";
    while (*l1) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *l1++);

    glColor3f(0.3f, 0.9f, 0.3f);
    glRasterPos2f(10, 447);
    const char* l2 = "Oculta:  C D";
    while (*l2) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *l2++);

    glColor3f(0.3f, 0.3f, 0.9f);
    glRasterPos2f(10, 432);
    const char* l3 = "Saida:   E F";
    while (*l3) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *l3++);

    for (int i = 0; i < NUM_NEURONIOS; i++) {
        std::string linha = std::string(nomes[i]) + ": " + std::to_string(contadores[i]);
        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(10, 410 - i * 18);
        for (char c : linha)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    // painel de controle
    glColor3f(1.0f, 0.8f, 0.0f);
    glRasterPos2f(10, 90);
    const char* p = "[ Controles ]";
    while (*p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p++);

    const char* teclas[] = {
        "1 -> Estimular A",
        "2 -> Estimular B",
        "3 -> Estimular C",
        "4 -> Estimular D",
        "5 -> Estimular E",
        "6 -> Estimular F",
        "R -> Resetar"
    };
    for (int i = 0; i < 7; i++) {
        glColor3f(0.8f, 0.8f, 0.8f);
        glRasterPos2f(10, 72 - i * 14);
        const char* k = teclas[i];
        while (*k) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *k++);
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

    desenharHUD();
    glutSwapBuffers();
}

void teclado(unsigned char key, int x, int y) {
    if (jniEnv == nullptr || mainObj == nullptr) return;

    int id = -1;
    switch (key) {
        case '1': id = 0; break;
        case '2': id = 1; break;
        case '3': id = 2; break;
        case '4': id = 3; break;
        case '5': id = 4; break;
        case '6': id = 5; break;
        case 'r': case 'R':
            for (int i = 0; i < NUM_NEURONIOS; i++) {
                neuronios[i] = false;
                contadores[i] = 0;
                sinais[i < 8 ? i : 0].ativo = false;
            }
            glutPostRedisplay();
            return;
    }

    if (id >= 0 && estimularMethod != nullptr)
        jniEnv->CallStaticVoidMethod(
            jniEnv->GetObjectClass(mainObj),
            estimularMethod, id);
}

void mouseClick(int botao, int estado, int x, int y) {
    if (botao == GLUT_LEFT_BUTTON) {
        mousePressionado = (estado == GLUT_DOWN);
        mouseX = x; mouseY = y;
    }
}

void mouseMove(int x, int y) {
    if (!mousePressionado) return;
    anguloY += (x - mouseX) * 0.5f;
    anguloX += (y - mouseY) * 0.5f;
    mouseX = x; mouseY = y;
    glutPostRedisplay();
}

JNIEXPORT void JNICALL Java_MotorGrafico_inicializar(JNIEnv* env, jobject obj, jint w, jint h) {
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
    glutKeyboardFunc(teclado);
    glutTimerFunc(30, atualizar, 0);
    glutMainLoop();
}

JNIEXPORT void JNICALL Java_MotorGrafico_atualizarNeuronio(JNIEnv*, jobject, jint id, jboolean ativo) {
    if (id >= 0 && id < NUM_NEURONIOS)
        neuronios[id] = ativo;
    if (ativo) {
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

JNIEXPORT void JNICALL Java_MotorGrafico_setCallbackEstimulo(JNIEnv* env, jobject obj, jobject mainClass) {
    jniEnv = env;
    mainObj = env->NewGlobalRef(mainClass);
    jclass cls = env->FindClass("Main");
    estimularMethod = env->GetStaticMethodID(cls, "estimular", "(I)V");
}