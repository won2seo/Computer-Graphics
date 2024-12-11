#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

// BMP �ؽ�ó �����͸� �����ϱ� ���� ����ü ����
typedef struct _AUX_RGBImageRec {
    GLint sizeX, sizeY;          // �̹��� ũ��
    unsigned char* data;         // �ȼ� ������
} AUX_RGBImageRec;

// �ؽ�ó ��ü�� �ؽ�ó �̹��� ������ ������
unsigned int MyTextureObject[1];
AUX_RGBImageRec* pTextureImage[1];

// â ũ�� ���� �� ȣ��Ǵ� �Լ�
void MyReshape(int w, int h) {
    glViewport(0, 0, w, h);  // ����Ʈ�� â ũ�⿡ �°� ����
    glMatrixMode(GL_PROJECTION);  // ���� ��� ����
    glLoadIdentity();
    gluPerspective(40.0, (GLfloat)w / (GLfloat)h, 1.0, 100.0);  // ���� ���� ����
    glMatrixMode(GL_MODELVIEW);  // �𵨺� ��� ����
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0,     // ī�޶� ��ġ
        0.0, 0.0, 0.0,     // ī�޶� �ٶ󺸴� ����
        0.0, 1.0, 0.0);    // ī�޶� ��� ����
}

// ����� �������ϴ� �Լ�
void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // ����� ���� ���� �ʱ�ȭ

    glBindTexture(GL_TEXTURE_2D, MyTextureObject[0]);  // �ؽ�ó ���ε�

    // �簢���� �ؽ�ó�� �����Ͽ� ������
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f); // ���� �Ʒ�
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);  // ������ �Ʒ�
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);   // ������ ��
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);  // ���� ��
    glEnd();

    glutSwapBuffers();  // ���� ��ȯ
}

// BMP ������ �о� �ؽ�ó �����ͷ� ��ȯ�ϴ� �Լ�
AUX_RGBImageRec* LoadBMP(const char* filename) {
    std::ifstream file(filename, std::ios::binary);  // ���̳ʸ� ���� ���� ����
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return nullptr;
    }

    // BMP ���� ��� �б�
    BITMAPFILEHEADER bfh;
    file.read(reinterpret_cast<char*>(&bfh), sizeof(BITMAPFILEHEADER));

    if (bfh.bfType != 0x4D42) {  // ���� ���� Ȯ�� ("BM")
        std::cerr << "Not a BMP file: " << filename << std::endl;
        return nullptr;
    }

    // BMP ���� ��� �б�
    BITMAPINFOHEADER bih;
    file.read(reinterpret_cast<char*>(&bih), sizeof(BITMAPINFOHEADER));

    if (bih.biBitCount != 24) {  // 24��Ʈ BMP�� ����
        std::cerr << "Only 24-bit BMPs are supported." << std::endl;
        return nullptr;
    }

    // �ؽ�ó ������ ����ü ���� �� �ʱ�ȭ
    AUX_RGBImageRec* texture = new AUX_RGBImageRec;
    texture->sizeX = bih.biWidth;
    texture->sizeY = bih.biHeight;

    int imageSize = texture->sizeX * texture->sizeY * 3;  // 3(RGB) ����Ʈ
    texture->data = new unsigned char[imageSize];

    // BMP ������ �б� ���� ��ġ�� �̵�
    file.seekg(bfh.bfOffBits, std::ios::beg);

    // BMP ���� �е� ���
    int padding = (4 - (texture->sizeX * 3) % 4) % 4;

    // BMP ������ �б�
    std::vector<unsigned char> row(texture->sizeX * 3 + padding);
    for (int y = texture->sizeY - 1; y >= 0; y--) {  // �Ʒ����� ���� �б�
        file.read(reinterpret_cast<char*>(row.data()), row.size());
        for (int x = 0; x < texture->sizeX; x++) {
            int i = (y * texture->sizeX + x) * 3;
            texture->data[i] = row[x * 3 + 2];     // Red
            texture->data[i + 1] = row[x * 3 + 1]; // Green
            texture->data[i + 2] = row[x * 3];     // Blue
        }
    }

    file.close();
    return texture;
}

// �ؽ�ó�� OpenGL�� �ε��ϴ� �Լ�
int LoadGLTextures(char* szFilePath) {
    int Status = FALSE;  // ���� ����
    glClearColor(0.0, 0.0, 0.0, 0.5);  // ���� ����
    memset(pTextureImage, 0, sizeof(void*) * 1);  // �ʱ�ȭ

    // �ؽ�ó ���� �ε�
    if (pTextureImage[0] = LoadBMP(szFilePath)) {
        Status = TRUE;  // ����
        glGenTextures(1, &MyTextureObject[0]);  // �ؽ�ó ����
        glBindTexture(GL_TEXTURE_2D, MyTextureObject[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3,
            pTextureImage[0]->sizeX, pTextureImage[0]->sizeY,
            0, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage[0]->data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glEnable(GL_TEXTURE_2D);
    }
    if (pTextureImage[0]) {  // �޸� ����
        if (pTextureImage[0]->data) {
            free(pTextureImage[0]->data);
        }
        free(pTextureImage[0]);
    }
    return Status;
}

// ���α׷� ������
void main(int argc, char** argv) {
    const char* textureFilePath = "�ٶ�������.bmp";  // �ؽ�ó ���� ���

    glutInit(&argc, argv);  // GLUT �ʱ�ȭ
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("OpenGL Sample Program");  // ������ ����
    glutDisplayFunc(Display);  // ���÷��� �ݹ�
    glutReshapeFunc(MyReshape);  // �������� �ݹ�

    // �ؽ�ó �ε�
    if (LoadGLTextures((char*)textureFilePath)) {
        glEnable(GL_TEXTURE_2D);  // �ؽ�ó Ȱ��ȭ
        glShadeModel(GL_SMOOTH);  // �ε巯�� ���� ���
        glClearDepth(1.0);  // ���� ���� �ʱ�ȭ
        glEnable(GL_DEPTH_TEST);  // ���� �׽�Ʈ Ȱ��ȭ
        glDepthFunc(GL_LEQUAL);  // ���� �׽�Ʈ ����
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glutMainLoop();  // �̺�Ʈ ���� ����
    }
    else {
        std::cerr << "Failed to load texture!" << std::endl;
    }
}
