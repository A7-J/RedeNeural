#include "MotorGrafico.h"
#include <GL/glut.h>
#include <cmath>

// estado dos neurônios
bool neuronios[3] = {false, false, false};

void desenharTexto(float x, float y, const char* texto) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    while (*texto)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *texto++);
}

void desenharCirculo(float x, float y, float raio, bool ativo) {
    if (ativo)
        glColor3f(1.0f, 1.0f, 0.0f);
    else
        glColor3f(0.3f, 0.3f, 0.8f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 100; i++) {
        float angulo = i * 2.0f * 3.14159f / 100;
        glVertex2f(x + raio * cos(angulo), y + raio * sin(angulo));
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.5f, 0.0f); glVertex2f(-0.1f, 0.0f);
        glVertex2f(0.1f, 0.0f);  glVertex2f(0.5f, 0.0f);
    glEnd();

    desenharCirculo(-0.6f, 0.0f, 0.1f, neuronios[0]);
    desenharCirculo( 0.0f, 0.0f, 0.1f, neuronios[1]);
    desenharCirculo( 0.6f, 0.0f, 0.1f, neuronios[2]);

    desenharTexto(-0.63f, 0.15f, "A");
    desenharTexto(-0.03f, 0.15f, "B");
    desenharTexto( 0.57f, 0.15f, "C");

    glFlush();
}

JNIEXPORT void JNICALL Java_MotorGrafico_inicializar(JNIEnv*, jobject, jint w, jint h) {
    int argc = 0;
    glutInit(&argc, nullptr);
    glutInitWindowSize(w, h);
    glutCreateWindow("Rede Neural - RedeNeural");
    glutDisplayFunc(display);
    glutMainLoop();
}

JNIEXPORT void JNICALL Java_MotorGrafico_atualizarNeuronio(JNIEnv*, jobject, jint id, jboolean ativo) {
    if (id >= 0 && id < 3)
        neuronios[id] = ativo;
    glutPostRedisplay();
}