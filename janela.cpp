#include <GL/glut.h>
#include <cmath>

void desenharCirculo(float x, float y, float raio) {
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

    // Neurônio A — vermelho
    glColor3f(1.0f, 0.3f, 0.3f);
    desenharCirculo(-0.6f, 0.0f, 0.1f);

    // Neurônio B — azul
    glColor3f(0.3f, 0.3f, 1.0f);
    desenharCirculo(0.0f, 0.0f, 0.1f);

    // Neurônio C — verde
    glColor3f(0.3f, 1.0f, 0.3f);
    desenharCirculo(0.6f, 0.0f, 0.1f);

    // Conexão A → B
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex2f(-0.5f, 0.0f);
        glVertex2f(-0.1f, 0.0f);
    glEnd();

    // Conexão B → C
    glBegin(GL_LINES);
        glVertex2f(0.1f, 0.0f);
        glVertex2f(0.5f, 0.0f);
    glEnd();

    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutCreateWindow("Rede Neural - RedeNeural");
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}