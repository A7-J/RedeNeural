#include "MotorGrafico.h"
#include <GL/glut.h>
#include <cmath>
#include <string>
#include <cstdlib>

#define NUM_NEURONIOS 6
#define NUM_ESTRELAS 200

bool neuronios[NUM_NEURONIOS] = {false};
int contadores[NUM_NEURONIOS] = {0};
float haloRaio[NUM_NEURONIOS] = {0};
bool haloAtivo[NUM_NEURONIOS] = {false};
bool modoAutomatico = false;
int autoTimer = 0;
float velocidade = 0.04f; // velocidade padrão do sinal

float posX[] = {-0.8f, -0.8f,  0.0f,  0.0f,  0.8f,  0.8f};
float posY[] = { 0.3f, -0.3f,  0.3f, -0.3f,  0.3f, -0.3f};
const char* nomes[] = {"A", "B", "C", "D", "E", "F"};

float anguloX = 0.0f;
float anguloY = 0.0f;
int mouseX, mouseY;
bool mousePressionado = false;
bool mouseMoveu = false;

static JNIEnv* jniEnv = nullptr;
static jobject mainObj = nullptr;
static jmethodID estimularMethod = nullptr;

struct Sinal {
    bool ativo = false;
    float progresso = 0.0f;
    float x1, y1, x2, y2;
};
Sinal sinais[8];

int sinalPorNeuronio[6][2] = {
    {0, 1}, {2, 3}, {4, 5}, {6, 7}, {-1,-1}, {-1,-1}
};

struct Estrela { float x, y, z, brilho; };
Estrela estrelas[NUM_ESTRELAS];

void inicializarEstrelas() {
    srand(42);
    for (int i = 0; i < NUM_ESTRELAS; i++) {
        estrelas[i].x = ((rand() % 2000) - 1000) / 500.0f;
        estrelas[i].y = ((rand() % 2000) - 1000) / 500.0f;
        estrelas[i].z = ((rand() % 2000) - 1000) / 500.0f - 2.0f;
        estrelas[i].brilho = (rand() % 100) / 100.0f;
    }
}

void desenharEstrelas() {
    glDisable(GL_LIGHTING);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_ESTRELAS; i++) {
        float b = estrelas[i].brilho;
        // alterna entre branco, azul e roxo
        if (i % 3 == 0)
            glColor3f(b, b, b);           // branco
        else if (i % 3 == 1)
            glColor3f(b * 0.6f, b * 0.6f, b); // azulado
        else
            glColor3f(b * 0.7f, b * 0.3f, b); // roxo
        glVertex3f(estrelas[i].x, estrelas[i].y, estrelas[i].z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

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

void desenharHalo(int id) {
    if (!haloAtivo[id]) return;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // aditivo — mais brilhante

    float raio = haloRaio[id];
    float alpha = 1.0f - (raio / 0.5f);
    if (alpha < 0) alpha = 0;

    glPushMatrix();
    glTranslatef(posX[id], posY[id], 0.0f);

    // cor do bloom por camada
    float r, g, b;
    if (neuronios[id]) {
        r = 1.0f; g = 1.0f; b = 0.0f; // amarelo
    } else if (id < 2) {
        r = 1.0f; g = 0.2f; b = 0.2f; // vermelho neon
    } else if (id < 4) {
        r = 0.2f; g = 1.0f; b = 0.2f; // verde neon
    } else {
        r = 0.2f; g = 0.4f; b = 1.0f; // azul neon
    }

    // múltiplos anéis concêntricos — efeito bloom
    int aneis = 5;
    for (int a = 0; a < aneis; a++) {
        float raioAnel = raio + a * 0.03f;
        float alphaAnel = alpha * (1.0f - (float)a / aneis) * 0.6f;

        glColor4f(r, g, b, alphaAnel);
        glLineWidth(2.0f - a * 0.3f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 64; i++) {
            float angulo = i * 2.0f * M_PI / 64;
            glVertex3f(raioAnel * cos(angulo), raioAnel * sin(angulo), 0.0f);
        }
        glEnd();
    }

    // brilho central — círculo preenchido translúcido
    float alphaCore = alpha * 0.15f;
    glColor4f(r, g, b, alphaCore);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= 64; i++) {
        float angulo = i * 2.0f * M_PI / 64;
        glVertex3f(raio * cos(angulo), raio * sin(angulo), 0.0f);
    }
    glEnd();

    glPopMatrix();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void desenharEsfera(int id) {
    glPushMatrix();
    glTranslatef(posX[id], posY[id], 0.0f);

    float r, g, b;
    if (neuronios[id]) {
        r = 1.0f; g = 1.0f; b = 0.0f;
    } else if (id < 2) {
        r = 0.9f; g = 0.3f; b = 0.3f;
    } else if (id < 4) {
        r = 0.3f; g = 0.9f; b = 0.3f;
    } else {
        r = 0.3f; g = 0.3f; b = 0.9f;
    }

    int stacks = 64;
    int slices = 64;
    float raio = 0.12f;

    // direção da luz para cálculo manual de iluminação
    float lx = 2.0f, ly = 2.0f, lz = 3.0f;
    float llen = sqrt(lx*lx + ly*ly + lz*lz);
    lx /= llen; ly /= llen; lz /= llen;

    for (int i = 0; i < stacks; i++) {
        float phi1 = M_PI * i / stacks;
        float phi2 = M_PI * (i + 1) / stacks;

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * M_PI * j / slices;

            float u = (float)j / slices;
            float v1 = (float)i / stacks;
            float v2 = (float)(i + 1) / stacks;

            for (int v = 0; v < 2; v++) {
                float phi = (v == 0) ? phi1 : phi2;
                float vt = (v == 0) ? v1 : v2;

                // normal da esfera
                float nx = sin(phi) * cos(theta);
                float ny = sin(phi) * sin(theta);
                float nz = cos(phi);

                // difuso — dot product normal * luz
                float difuso = nx * lx + ny * ly + nz * lz;
                if (difuso < 0) difuso = 0;

                // especular — reflexão de Phong
                float rx = 2.0f * difuso * nx - lx;
                float ry = 2.0f * difuso * ny - ly;
                float rz = 2.0f * difuso * nz - lz;
                float vdotr = rz; // câmera em Z+
                if (vdotr < 0) vdotr = 0;
                float especular = pow(vdotr, 32) * 0.8f;

                // textura por camada
                float textura;
                if (id < 2) {
                    // grade — entrada
                    int grade = 8;
                    bool linhaU = (fmod(u * grade, 1.0f) < 0.08f);
                    bool linhaV = (fmod(vt * grade, 1.0f) < 0.08f);
                    textura = (linhaU || linhaV) ? 0.25f : 1.0f;
                } else if (id < 4) {
                    // ondas senoidais — oculta
                    float onda = sin(u * M_PI * 10) * sin(vt * M_PI * 10);
                    textura = 0.5f + 0.5f * onda;
                } else {
                    // manchas — saída
                    float ruido = fabs(sin(u * 19.3f) * cos(vt * 15.7f) *
                                  sin((u + vt) * 11.1f));
                    textura = 0.35f + 0.65f * ruido;
                }

                // combina textura + difuso + especular
                float ambient = 0.15f;
                float final_r = r * (ambient + textura * difuso * 0.85f) + especular;
                float final_g = g * (ambient + textura * difuso * 0.85f) + especular;
                float final_b = b * (ambient + textura * difuso * 0.85f) + especular;

                // clamp 0-1
                if (final_r > 1.0f) final_r = 1.0f;
                if (final_g > 1.0f) final_g = 1.0f;
                if (final_b > 1.0f) final_b = 1.0f;

                glColor3f(final_r, final_g, final_b);
                glNormal3f(nx, ny, nz);
                glVertex3f(raio * nx, raio * ny, raio * nz);
            }
        }
        glEnd();
    }

    glPopMatrix();
}

void desenharCurvaBezier(float x1, float y1, float x2, float y2, float r, float g, float b) {
    // ponto de controle no meio — deslocado pra cima pra criar curva
    float cx = (x1 + x2) / 2.0f;
    float cy = (y1 + y2) / 2.0f + 0.2f;

    int segmentos = 30;
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segmentos; i++) {
        float t = (float)i / segmentos;
        float mt = 1.0f - t;

        // Bézier quadrática: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2
        float x = mt*mt*x1 + 2*mt*t*cx + t*t*x2;
        float y = mt*mt*y1 + 2*mt*t*cy + t*t*y2;

        glColor3f(r, g, b);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();
}

void desenharConexoes() {
    glDisable(GL_LIGHTING);
    glLineWidth(1.5f);

    for (int i = 0; i < 8; i++) {
        float brilho = sinais[i].ativo ? sinais[i].progresso : 0.0f;

        float r = 0.3f + 0.0f * brilho;
        float g = 0.3f + 0.7f * brilho;
        float b = 0.3f + 0.7f * brilho;

        desenharCurvaBezier(
            sinais[i].x1, sinais[i].y1,
            sinais[i].x2, sinais[i].y2,
            r, g, b
        );
    }
    glEnable(GL_LIGHTING);
}

void desenharSinais() {
    glDisable(GL_LIGHTING);
    for (int i = 0; i < 8; i++) {
        if (!sinais[i].ativo) continue;

        float t = sinais[i].progresso;
        float mt = 1.0f - t;

        float x1 = sinais[i].x1, y1 = sinais[i].y1;
        float x2 = sinais[i].x2, y2 = sinais[i].y2;
        float cx = (x1 + x2) / 2.0f;
        float cy = (y1 + y2) / 2.0f + 0.2f;

        // segue a curva de Bézier
        float x = mt*mt*x1 + 2*mt*t*cx + t*t*x2;
        float y = mt*mt*y1 + 2*mt*t*cy + t*t*y2;

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

    glColor3f(1.0f, 0.8f, 0.0f);
    glRasterPos2f(10, 90);
    const char* p = "[ Controles ]";
    while (*p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p++);

    const char* teclas[] = {
        "1 -> Estimular A", "2 -> Estimular B",
        "3 -> Estimular C", "4 -> Estimular D",
        "5 -> Estimular E", "6 -> Estimular F",
        "R -> Resetar"
    };
    for (int i = 0; i < 7; i++) {
        glColor3f(0.8f, 0.8f, 0.8f);
        glRasterPos2f(10, 72 - i * 14);
        const char* k = teclas[i];
        while (*k) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *k++);
    }

    // modo automático
    glColor3f(0.0f, 1.0f, 0.0f);
    glRasterPos2f(10, 200);
    const char* modo = modoAutomatico ? "A -> Modo AUTO: ON " : "A -> Modo AUTO: OFF";
    while (*modo) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *modo++);

    glColor3f(0.8f, 0.8f, 0.0f);
    glRasterPos2f(10, 250);
    std::string vel = "+ / - Velocidade: " + std::to_string((int)(velocidade * 100)) + "%";
    const char* vs = vel.c_str();
    while (*vs) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *vs++);
    // título no canto superior direito
    glColor3f(0.0f, 0.8f, 1.0f);
    glRasterPos2f(420, 480);
    const char* titulo = "Visualizador de Redes Neurais Biologicas";
    while (*titulo) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *titulo++);

    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2f(420, 465);
    const char* autor = "Arthur Pires - UNIR";
    while (*autor) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *autor++);

    glColor3f(0.5f, 0.5f, 0.5f);
    glRasterPos2f(420, 450);
    const char* orient = "Orientador: Prof. Dr. Lucas Marques da Cunha";
    while (*orient) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *orient++);

    glColor3f(0.4f, 0.4f, 0.4f);
    glRasterPos2f(420, 435);
    const char* stack = "Java + JNI + C++ + OpenGL";
    while (*stack) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *stack++);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void estimularAutomatico(int valor) {
    if (!modoAutomatico) return;
    if (jniEnv != nullptr && estimularMethod != nullptr) {
        int id = (autoTimer % 2 == 0) ? 0 : 1;
        jniEnv->CallStaticVoidMethod(
            jniEnv->GetObjectClass(mainObj),
            estimularMethod, id);
        autoTimer++;
    }
    glutTimerFunc(800, estimularAutomatico, 0);
}

void atualizar(int valor) {
    for (int i = 0; i < 8; i++) {
        if (!sinais[i].ativo) continue;
        sinais[i].progresso += velocidade;
        if (sinais[i].progresso >= 1.0f) {
            sinais[i].ativo = false;
            sinais[i].progresso = 0.0f;
        }
    }

    for (int i = 0; i < NUM_NEURONIOS; i++) {
        if (!haloAtivo[i]) continue;
        haloRaio[i] += 0.02f;
        if (haloRaio[i] >= 0.5f) {
            haloAtivo[i] = false;
            haloRaio[i] = 0.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(30, atualizar, 0);
}

void desenharFundo() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // gradiente roxo/azul profundo
    glBegin(GL_QUADS);
        glColor3f(0.08f, 0.0f, 0.15f); // roxo escuro topo
        glVertex2f(-1.0f,  1.0f);
        glVertex2f( 1.0f,  1.0f);
        glColor3f(0.0f, 0.0f, 0.08f);  // azul escuro base
        glVertex2f( 1.0f, -1.0f);
        glVertex2f(-1.0f, -1.0f);
    glEnd();

    // névoa roxa no centro
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int camadas = 6;
    for (int k = 0; k < camadas; k++) {
        float raio = 0.3f + k * 0.12f;
        float alpha = 0.04f - k * 0.005f;
        if (alpha <= 0) continue;

        glColor4f(0.4f, 0.0f, 0.8f, alpha);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f);
        for (int i = 0; i <= 64; i++) {
            float angulo = i * 2.0f * M_PI / 64;
            glVertex2f(raio * cos(angulo), raio * sin(angulo));
        }
        glEnd();
    }

    // névoa secundária azul deslocada
    for (int k = 0; k < 4; k++) {
        float raio = 0.2f + k * 0.1f;
        float alpha = 0.03f - k * 0.005f;
        if (alpha <= 0) continue;

        glColor4f(0.0f, 0.2f, 0.8f, alpha);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(-0.3f, 0.2f);
        for (int i = 0; i <= 64; i++) {
            float angulo = i * 2.0f * M_PI / 64;
            glVertex2f(-0.3f + raio * cos(angulo), 0.2f + raio * sin(angulo));
        }
        glEnd();
    }

    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

    desenharFundo(); 
    desenharEstrelas();

    glRotatef(anguloX, 1.0f, 0.0f, 0.0f);
    glRotatef(anguloY, 0.0f, 1.0f, 0.0f);

    configurarLuz();
    desenharConexoes();

    for (int i = 0; i < NUM_NEURONIOS; i++)
        desenharHalo(i);

    for (int i = 0; i < NUM_NEURONIOS; i++)
        desenharEsfera(i);

    desenharSinais();

    for (int i = 0; i < NUM_NEURONIOS; i++)
        desenharTexto(posX[i] - 0.03f, posY[i] + 0.17f, 0.0f, nomes[i]);

    desenharHUD();
    glutSwapBuffers();
}

void estimular(int id) {
    if (id < 0 || id >= NUM_NEURONIOS) return;
    if (jniEnv != nullptr && estimularMethod != nullptr)
        jniEnv->CallStaticVoidMethod(
            jniEnv->GetObjectClass(mainObj),
            estimularMethod, id);
}

void clicouEmEsfera(int x, int y, int largura, int altura) {
    float nx = (2.0f * x / largura) - 1.0f;
    float ny = 1.0f - (2.0f * y / altura);

    float escalaX = 1.3f;
    float escalaY = 1.3f * ((float)altura / largura) * ((float)largura / altura);
    // ajusta proporção considerando aspect ratio da janela
    float aspect = (float)largura / altura;
    float mundoX = nx * 1.3f;
    float mundoY = ny * 1.3f / aspect * aspect; // simplificado

    mundoX = nx * 1.3f;
    mundoY = ny * 1.3f;

    float menorDist = 999.0f;
    int idMaisProximo = -1;

    for (int i = 0; i < NUM_NEURONIOS; i++) {
        float dx = posX[i] - mundoX;
        float dy = posY[i] - mundoY;
        float dist = sqrt(dx * dx + dy * dy);
        if (dist < menorDist) {
            menorDist = dist;
            idMaisProximo = i;
        }
    }

    if (idMaisProximo >= 0 && menorDist < 0.25f) {
        estimular(idMaisProximo);
    }
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
        case '+':
        velocidade += 0.01f;
        if (velocidade > 0.15f) velocidade = 0.15f;
        return;
        case '-':
        velocidade -= 0.01f;
         if (velocidade < 0.01f) velocidade = 0.01f;
        return;
        case 'a': case 'A':
            modoAutomatico = !modoAutomatico;
            if (modoAutomatico)
                glutTimerFunc(800, estimularAutomatico, 0);
            return;
        case 'r': case 'R':
            for (int i = 0; i < NUM_NEURONIOS; i++) {
                neuronios[i] = false;
                contadores[i] = 0;
                haloAtivo[i] = false;
                haloRaio[i] = 0.0f;
            }
            for (int i = 0; i < 8; i++)
                sinais[i].ativo = false;
            glutPostRedisplay();
            return;
    }

    if (id >= 0 && estimularMethod != nullptr)
        jniEnv->CallStaticVoidMethod(
            jniEnv->GetObjectClass(mainObj),
            estimularMethod, id);
}

void mouseClick(int botao, int estado, int x, int y) {
    if (botao != GLUT_LEFT_BUTTON) return;

    if (estado == GLUT_DOWN) {
        mousePressionado = true;
        mouseMoveu = false;
        mouseX = x;
        mouseY = y;
    } else if (estado == GLUT_UP) {
        mousePressionado = false;
        if (!mouseMoveu) {
            int largura = glutGet(GLUT_WINDOW_WIDTH);
            int altura = glutGet(GLUT_WINDOW_HEIGHT);
            clicouEmEsfera(x, y, largura, altura);
        }
    }
}

void mouseMove(int x, int y) {
    if (!mousePressionado) return;

    int dx = x - mouseX;
    int dy = y - mouseY;

    if (abs(dx) > 2 || abs(dy) > 2)
        mouseMoveu = true;

    if (mouseMoveu) {
        anguloY += dx * 0.5f;
        anguloX += dy * 0.5f;
    }

    mouseX = x;
    mouseY = y;
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
    inicializarEstrelas();

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

    if (ativo && id < NUM_NEURONIOS) {
        haloAtivo[id] = true;
        haloRaio[id] = 0.12f;

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