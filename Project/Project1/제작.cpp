#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// �ؽ�ó �����͸� �����ϱ� ���� ����ü ����
typedef struct _AUX_RGBImageRec {
    GLint sizeX, sizeY;
    unsigned char* data;
} AUX_RGBImageRec;

// �ؽ�ó ��ü�� �ؽ�ó �̹��� ������ ������
unsigned int MyTextureObject[1];
AUX_RGBImageRec* pTextureImage[1];

// ���� ����
GLfloat sunlight_diffuse[] = { 0.8f, 0.8f, 0.6f, 1.0f }; // ��ħ �޺� ����
GLfloat sunset_diffuse[] = { 0.9f, 0.4f, 0.2f, 1.0f };   // ���� ����
GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };    // ���� �ֺ���

GLfloat light_position_sunrise[] = { 0.0f, 5.0f, 10.0f, 1.0f }; // ��ħ �޺� ��ġ
GLfloat light_position_sunset[] = { -5.0f, 1.0f, 3.0f, 1.0f };  // ���� �ʱ� ��ġ

// ���� �ִϸ��̼� ����
float sunset_light_angle = 0.0f; // ���� ���� (0������ 90���� �̵�)
bool isAnimatingSunset = false; // �ִϸ��̼� Ȱ��ȭ ����
bool isSunrise = false;          // ��ħ �޺� ����

// ī�޶� �̵� ����
float cameraZ = 10.0f;    // ī�޶� �ʱ� z ��ġ
float cameraSpeed = 0.05f; // ī�޶� �̵� �ӵ�
float pathEndZ = -10.0f;  // ���� �� z ��ġ

// BMP ������ �о� �ؽ�ó �����ͷ� ��ȯ�ϴ� �Լ�
AUX_RGBImageRec* LoadBMP(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return nullptr;
    }

    BITMAPFILEHEADER bfh;
    file.read(reinterpret_cast<char*>(&bfh), sizeof(BITMAPFILEHEADER));

    if (bfh.bfType != 0x4D42) {
        std::cerr << "Not a BMP file: " << filename << std::endl;
        return nullptr;
    }

    BITMAPINFOHEADER bih;
    file.read(reinterpret_cast<char*>(&bih), sizeof(BITMAPINFOHEADER));

    if (bih.biBitCount != 24) {
        std::cerr << "Only 24-bit BMPs are supported." << std::endl;
        return nullptr;
    }

    AUX_RGBImageRec* texture = new AUX_RGBImageRec;
    texture->sizeX = bih.biWidth;
    texture->sizeY = bih.biHeight;

    int imageSize = texture->sizeX * texture->sizeY * 3;
    texture->data = new unsigned char[imageSize];

    file.seekg(bfh.bfOffBits, std::ios::beg);

    int padding = (4 - (texture->sizeX * 3) % 4) % 4;

    std::vector<unsigned char> row(texture->sizeX * 3 + padding);
    for (int y = texture->sizeY - 1; y >= 0; y--) {
        file.read(reinterpret_cast<char*>(row.data()), row.size());
        for (int x = 0; x < texture->sizeX; x++) {
            int i = (y * texture->sizeX + x) * 3;
            texture->data[i] = row[x * 3 + 2];
            texture->data[i + 1] = row[x * 3 + 1];
            texture->data[i + 2] = row[x * 3];
        }
    }

    file.close();
    return texture;
}

// �ؽ�ó�� OpenGL�� �ε��ϴ� �Լ�
int LoadGLTextures(const char* szFilePath) {
    int Status = FALSE;
    memset(pTextureImage, 0, sizeof(void*) * 1);

    if (pTextureImage[0] = LoadBMP(szFilePath)) {
        Status = TRUE;
        glGenTextures(1, &MyTextureObject[0]);
        glBindTexture(GL_TEXTURE_2D, MyTextureObject[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3,
            pTextureImage[0]->sizeX, pTextureImage[0]->sizeY,
            0, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage[0]->data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    if (pTextureImage[0]) {
        delete[] pTextureImage[0]->data;
        delete pTextureImage[0];
    }
    return Status;
}

// ���� �ʱ�ȭ
void InitLighting() {
    glEnable(GL_LIGHTING); // ���� Ȱ��ȭ
    glEnable(GL_LIGHT0);   // ù ��° ���� Ȱ��ȭ

    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunlight_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position_sunrise);
}

// ���� �ִϸ��̼� ������Ʈ
void UpdateSunsetPosition() {
    if (isAnimatingSunset) {
        sunset_light_angle += 0.5f; // ������ �������� �ӵ�
        if (sunset_light_angle > 90.0f) {
            sunset_light_angle = 90.0f; // ����
            isAnimatingSunset = false; // �ִϸ��̼� ����
        }

        // ���� ��ġ ���
        float x = -5.0f + 5.0f * cos(sunset_light_angle * M_PI / 180.0f);
        float y = 5.0f * sin(sunset_light_angle * M_PI / 180.0f);
        light_position_sunset[0] = x;
        light_position_sunset[1] = y;

        // ���� ��ġ ������Ʈ
        glLightfv(GL_LIGHT0, GL_POSITION, light_position_sunset);
        glutPostRedisplay();
    }
}

// ī�޶� �̵� ������Ʈ
void UpdateCameraPosition() {
    cameraZ -= cameraSpeed; // ī�޶� ������ �̵�
    if (cameraZ <= pathEndZ) {
        cameraZ = 10.0f; // �ʱ� ��ġ�� �ǵ���
    }
    glutPostRedisplay(); // ȭ�� ���� ��û
}

// �ٴ� ������ �Լ�
void DrawGround() {
    glColor3f(0.6, 0.8, 0.3);
    glBegin(GL_QUADS);
    glVertex3f(-20.0, 0.0, -20.0);
    glVertex3f(20.0, 0.0, -20.0);
    glVertex3f(20.0, 0.0, 20.0);
    glVertex3f(-20.0, 0.0, 20.0);
    glEnd();
}

// �� ������ �Լ�
void DrawPath() {
    glColor3f(0.4, 0.4, 0.4); // ȸ��
    glBegin(GL_QUADS);
    glVertex3f(-1.0, 0.01, -20.0);
    glVertex3f(1.0, 0.01, -20.0);
    glVertex3f(1.0, 0.01, 20.0);
    glVertex3f(-1.0, 0.01, 20.0);
    glEnd();
}

// ���� ������ �Լ�
void DrawTree(float x, float z) {
    glPushMatrix();
    glColor3f(0.5, 0.3, 0.1);
    glTranslatef(x, 0.0, z);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    GLUquadric* trunk = gluNewQuadric();
    gluCylinder(trunk, 0.2, 0.2, 1.5, 16, 16);
    gluDeleteQuadric(trunk);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.2, 0.6, 0.2);
    glTranslatef(x, 1.5, z);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glutSolidCone(0.8, 2.0, 16, 16);
    glPopMatrix();
}

// ū �� ������ �Լ�
void DrawRock(float x, float z) {
    glPushMatrix();
    glColor3f(0.5, 0.5, 0.5);
    glTranslatef(x, 0.5, z);
    glutSolidSphere(0.5, 16, 16);
    glPopMatrix();
}

// �ؽ�ó�� ����� �԰��� ������ �Լ�
void DrawSignWithTexture() {
    glBindTexture(GL_TEXTURE_2D, MyTextureObject[0]);
    glPushMatrix();
    glTranslatef(-2.0, 0.0, 2.0);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, 2.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, 2.0, 0.0);
    glEnd();

    glPopMatrix();
}

// ���� ��ȯ �Լ�
void ToggleLighting(char mode) {
    if (mode == 'x') { // �⺻ ��� (���� ����)
        glDisable(GL_LIGHTING);
    }
    else {
        glEnable(GL_LIGHTING);
        if (mode == 'c') { // ��ħ �޺� ���
            glLightfv(GL_LIGHT0, GL_DIFFUSE, sunlight_diffuse);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position_sunrise);
            isSunrise = true;
            isAnimatingSunset = false;
        }
        else if (mode == 'v') { // ���� ���
            glLightfv(GL_LIGHT0, GL_DIFFUSE, sunset_diffuse);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position_sunset);
            isSunrise = false;
            isAnimatingSunset = true; // ���� �ִϸ��̼� Ȱ��ȭ
        }
    }
}

// ����� �������ϴ� �Լ�
void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0, 1.5, cameraZ,  // ī�޶� ��ġ
        0.0, 1.5, cameraZ - 1.0f,  // ������
        0.0, 1.0, 0.0);  // ���� ����

    DrawGround();
    DrawPath();
    DrawTree(-2.0, -2.0);
    DrawTree(2.0, 2.0);
    DrawTree(-5.0, -5.0);
    DrawTree(5.0, -5.0);
    DrawTree(-7.0, 3.0);
    DrawTree(7.0, 3.0);
    DrawRock(-2.5, -2.5); // ū �� �߰�
    DrawSignWithTexture();

    glutSwapBuffers();
}

// â ũ�� ���� �� ȣ��Ǵ� �Լ�
void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
}

// Ű���� �Է� ó�� �Լ�
void Keyboard(unsigned char key, int x, int y) {
    if (key == 'h') {
        printf("����:\n");
        printf("'h': ���� ǥ��\n");
        printf("'x': �⺻ ��� (���� ����)\n");
        printf("'c': ��ħ �޺� ���\n");
        printf("'v': ���� ���\n");
    }
    else if (key == 'x' || key == 'c' || key == 'v') {
        ToggleLighting(key);
    }
    glutPostRedisplay(); // ȭ�� ����
}

// Ÿ�̸� �ݹ� �Լ� (�ִϸ��̼�)
void Timer(int value) {
    UpdateCameraPosition(); // ī�޶� �̵� ������Ʈ
    UpdateSunsetPosition(); // ���� ��ġ ������Ʈ
    glutTimerFunc(16, Timer, 0); // 16ms���� ���� (�� 60FPS)
}

// ���α׷� ������
int main(int argc, char** argv) {
    const char* textureFilePath = "�ٶ�������.bmp";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Park Lighting Animation");

    if (LoadGLTextures(textureFilePath)) {
        InitLighting(); // ���� �ʱ�ȭ
        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glutDisplayFunc(Display);
        glutReshapeFunc(Reshape);
        glutKeyboardFunc(Keyboard); // Ű���� �Է� ó��
        glutTimerFunc(16, Timer, 0); // Ÿ�̸� �Լ��� �ִϸ��̼� ����

        glutMainLoop();
    }
    else {
        std::cerr << "Failed to load texture!" << std::endl;
        return -1;
    }

    return 0;
}
