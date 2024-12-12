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

// 텍스처 데이터를 저장하기 위한 구조체 정의
typedef struct _AUX_RGBImageRec {
    GLint sizeX, sizeY;
    unsigned char* data;
} AUX_RGBImageRec;

// 텍스처 객체와 텍스처 이미지 데이터 포인터
unsigned int MyTextureObject[1];
AUX_RGBImageRec* pTextureImage[1];

// 광원 설정
GLfloat sunlight_diffuse[] = { 0.8f, 0.8f, 0.6f, 1.0f }; // 아침 햇빛 색상
GLfloat sunset_diffuse[] = { 0.9f, 0.4f, 0.2f, 1.0f };   // 석양 색상
GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };    // 약한 주변광

GLfloat light_position_sunrise[] = { 0.0f, 5.0f, 10.0f, 1.0f }; // 아침 햇빛 위치
GLfloat light_position_sunset[] = { -5.0f, 1.0f, 3.0f, 1.0f };  // 석양 초기 위치

// 석양 애니메이션 변수
float sunset_light_angle = 0.0f; // 석양 각도 (0도에서 90도로 이동)
bool isAnimatingSunset = false; // 애니메이션 활성화 여부
bool isSunrise = false;          // 아침 햇빛 상태

// 카메라 이동 변수
float cameraZ = 10.0f;    // 카메라 초기 z 위치
float cameraSpeed = 0.05f; // 카메라 이동 속도
float pathEndZ = -10.0f;  // 길의 끝 z 위치

// BMP 파일을 읽어 텍스처 데이터로 변환하는 함수
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

// 텍스처를 OpenGL에 로드하는 함수
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

// 광원 초기화
void InitLighting() {
    glEnable(GL_LIGHTING); // 광원 활성화
    glEnable(GL_LIGHT0);   // 첫 번째 광원 활성화

    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunlight_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position_sunrise);
}

// 석양 애니메이션 업데이트
void UpdateSunsetPosition() {
    if (isAnimatingSunset) {
        sunset_light_angle += 0.5f; // 석양이 내려가는 속도
        if (sunset_light_angle > 90.0f) {
            sunset_light_angle = 90.0f; // 제한
            isAnimatingSunset = false; // 애니메이션 종료
        }

        // 석양 위치 계산
        float x = -5.0f + 5.0f * cos(sunset_light_angle * M_PI / 180.0f);
        float y = 5.0f * sin(sunset_light_angle * M_PI / 180.0f);
        light_position_sunset[0] = x;
        light_position_sunset[1] = y;

        // 광원 위치 업데이트
        glLightfv(GL_LIGHT0, GL_POSITION, light_position_sunset);
        glutPostRedisplay();
    }
}

// 카메라 이동 업데이트
void UpdateCameraPosition() {
    cameraZ -= cameraSpeed; // 카메라를 앞으로 이동
    if (cameraZ <= pathEndZ) {
        cameraZ = 10.0f; // 초기 위치로 되돌림
    }
    glutPostRedisplay(); // 화면 갱신 요청
}

// 바닥 렌더링 함수
void DrawGround() {
    glColor3f(0.6, 0.8, 0.3);
    glBegin(GL_QUADS);
    glVertex3f(-20.0, 0.0, -20.0);
    glVertex3f(20.0, 0.0, -20.0);
    glVertex3f(20.0, 0.0, 20.0);
    glVertex3f(-20.0, 0.0, 20.0);
    glEnd();
}

// 길 렌더링 함수
void DrawPath() {
    glColor3f(0.4, 0.4, 0.4); // 회색
    glBegin(GL_QUADS);
    glVertex3f(-1.0, 0.01, -20.0);
    glVertex3f(1.0, 0.01, -20.0);
    glVertex3f(1.0, 0.01, 20.0);
    glVertex3f(-1.0, 0.01, 20.0);
    glEnd();
}

// 나무 렌더링 함수
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

// 큰 돌 렌더링 함수
void DrawRock(float x, float z) {
    glPushMatrix();
    glColor3f(0.5, 0.5, 0.5);
    glTranslatef(x, 0.5, z);
    glutSolidSphere(0.5, 16, 16);
    glPopMatrix();
}

// 텍스처가 적용된 입간판 렌더링 함수
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

// 광원 전환 함수
void ToggleLighting(char mode) {
    if (mode == 'x') { // 기본 모드 (광원 없음)
        glDisable(GL_LIGHTING);
    }
    else {
        glEnable(GL_LIGHTING);
        if (mode == 'c') { // 아침 햇빛 모드
            glLightfv(GL_LIGHT0, GL_DIFFUSE, sunlight_diffuse);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position_sunrise);
            isSunrise = true;
            isAnimatingSunset = false;
        }
        else if (mode == 'v') { // 석양 모드
            glLightfv(GL_LIGHT0, GL_DIFFUSE, sunset_diffuse);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position_sunset);
            isSunrise = false;
            isAnimatingSunset = true; // 석양 애니메이션 활성화
        }
    }
}

// 장면을 렌더링하는 함수
void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0, 1.5, cameraZ,  // 카메라 위치
        0.0, 1.5, cameraZ - 1.0f,  // 관찰점
        0.0, 1.0, 0.0);  // 상향 벡터

    DrawGround();
    DrawPath();
    DrawTree(-2.0, -2.0);
    DrawTree(2.0, 2.0);
    DrawTree(-5.0, -5.0);
    DrawTree(5.0, -5.0);
    DrawTree(-7.0, 3.0);
    DrawTree(7.0, 3.0);
    DrawRock(-2.5, -2.5); // 큰 돌 추가
    DrawSignWithTexture();

    glutSwapBuffers();
}

// 창 크기 변경 시 호출되는 함수
void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
}

// 키보드 입력 처리 함수
void Keyboard(unsigned char key, int x, int y) {
    if (key == 'h') {
        printf("사용법:\n");
        printf("'h': 도움말 표시\n");
        printf("'x': 기본 모드 (광원 없음)\n");
        printf("'c': 아침 햇빛 모드\n");
        printf("'v': 석양 모드\n");
    }
    else if (key == 'x' || key == 'c' || key == 'v') {
        ToggleLighting(key);
    }
    glutPostRedisplay(); // 화면 갱신
}

// 타이머 콜백 함수 (애니메이션)
void Timer(int value) {
    UpdateCameraPosition(); // 카메라 이동 업데이트
    UpdateSunsetPosition(); // 석양 위치 업데이트
    glutTimerFunc(16, Timer, 0); // 16ms마다 갱신 (약 60FPS)
}

// 프로그램 진입점
int main(int argc, char** argv) {
    const char* textureFilePath = "다람쥐주의.bmp";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Park Lighting Animation");

    if (LoadGLTextures(textureFilePath)) {
        InitLighting(); // 광원 초기화
        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glutDisplayFunc(Display);
        glutReshapeFunc(Reshape);
        glutKeyboardFunc(Keyboard); // 키보드 입력 처리
        glutTimerFunc(16, Timer, 0); // 타이머 함수로 애니메이션 실행

        glutMainLoop();
    }
    else {
        std::cerr << "Failed to load texture!" << std::endl;
        return -1;
    }

    return 0;
}
