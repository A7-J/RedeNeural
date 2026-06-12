#include "MotorGrafico.h"
#include <GL/glut.h>
#include <cmath>

bool neuronios[3] = {false, false, false};

void desenharTexto(float x, float y, float z, const char* texto) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos3f(x, y, z);
    while (*texto)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *texto++);
}

void desenharEsfera(float x, float y, float z, bool ativo) {
    glPushMatrix();
    glTranslatef(x, y, z);

    if (ativo)
        glColor3f(1.0f, 1.0f, 0.0f); // amarelo
    else
        glColor3f(0.3f, 0.3f, 0.9f); // azul

    glutSolidSphere(0.15, 32, 32);
    glPopMatrix();
}

void desenharConexao(float x1, float y1, float x2, float y2) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex3f(x1, y1, 0.0f);
        glVertex3f(x2, y2, 0.0f);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 3,   // posição da câmera
              0, 0, 0,   // olhando pra origem
              0, 1, 0);  // eixo up

    // conexões
    desenharConexao(-0.6f, 0.0f, 0.0f, 0.0f);
    desenharConexao( 0.0f, 0.0f, 0.6f, 0.0f);

    // esferas
    desenharEsfera(-0.6f, 0.0f, 0.0f, neuronios[0]);
    desenharEsfera( 0.0f, 0.0f, 0.0f, neuronios[1]);
    desenharEsfera( 0.6f, 0.0f, 0.0f, neuronios[2]);

    // labels
    desenharTexto(-0.63f, 0.2f, 0.0f, "A");
    desenharTexto(-0.03f, 0.2f, 0.0f, "B");
    desenharTexto( 0.57f, 0.2f, 0.0f, "C");

    glutSwapBuffers();
}

JNIEXPORT void JNICALL Java_MotorGrafico_inicializar(JNIEnv*, jobject, jint w, jint h) {
    int argc = 0;
    glutInit(&argc, nullptr);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    glutCreateWindow("Rede Neural 3D - RedeNeural");

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w/h, 0.1, 100.0);

    glutDisplayFunc(display);
    glutMainLoop();
}

JNIEXPORT void JNICALL Java_MotorGrafico_atualizarNeuronio(JNIEnv*, jobject, jint id, jboolean ativo) {
    if (id >= 0 && id < 3)
        neuronios[id] = ativo;
    glutPostRedisplay();
}