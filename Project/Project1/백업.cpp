#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 텍스처 데이터와 ID
GLuint textureID;
#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 256
GLubyte textureData[TEXTURE_WIDTH][TEXTURE_HEIGHT][4];

// 광원 설정
GLfloat sunlight_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
GLfloat sunlight_diffuse[] = { 1.0, 1.0, 0.8, 1.0 }; // 아침 햇빛
GLfloat sunset_diffuse[] = { 1.0, 0.5, 0.2, 1.0 };  // 석양
// 아침 햇빛 방향 (태양처럼 아래로 비추는 방향성 광원)
GLfloat light_position[] = { 0.0, -1.0, 0.0, 0.0 };  // y축으로 아래로 향하는 빛


// 공의 애니메이션 위치
float ball_x = 0.0, ball_z = 0.0;
int ball_direction = 1;

// 광원 색상 애니메이션 단계
float light_mix_factor = 0.0;
int light_mix_direction = 1;

void GenerateTexture() {
    // 텍스처 데이터를 직접 생성
    for (int i = 0; i < TEXTURE_HEIGHT; i++) {
        for (int j = 0; j < TEXTURE_WIDTH; j++) {
            // "다람쥐 주의" 텍스트를 나타내기 위한 간단한 텍스처
            if ((i > 50 && i < 200) && (j > 30 && j < 230)) {
                textureData[i][j][0] = 255;
                textureData[i][j][1] = 255;
                textureData[i][j][2] = 255;
                textureData[i][j][3] = 255;
                if (i > 80 && i < 170 && j > 60 && j < 200) {
                    textureData[i][j][0] = 0;
                    textureData[i][j][1] = 0;
                    textureData[i][j][2] = 0;
                }
            }
            else {
                textureData[i][j][0] = 200;
                textureData[i][j][1] = 150;
                textureData[i][j][2] = 100;
                textureData[i][j][3] = 255;
            }
        }
    }
}

void DrawGround() {
    glColor3f(0.6, 0.8, 0.3);
    glBegin(GL_QUADS);
    glVertex3f(-10.0, 0.0, -10.0);
    glVertex3f(10.0, 0.0, -10.0);
    glVertex3f(10.0, 0.0, 10.0);
    glVertex3f(-10.0, 0.0, 10.0);
    glEnd();

    glColor3f(0.4, 0.4, 0.4);
    glBegin(GL_QUADS);
    glVertex3f(-1.0, 0.01, -10.0);
    glVertex3f(1.0, 0.01, -10.0);
    glVertex3f(1.0, 0.01, 10.0);
    glVertex3f(-1.0, 0.01, 10.0);
    glEnd();
}

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

void DrawSign() {
    // 텍스처를 활성화하고 입간판을 그린다
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glPushMatrix();
    glTranslatef(-2.0, 0.0, 2.0);
    glColor3f(1.0, 1.0, 1.0);  // 텍스처를 흰색으로 출력
    // 입간판 크기 키우기
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, 0.0, 0.0); // 좌측 하단
    glTexCoord2f(1.0, 0.0); glVertex3f(1.0, 0.0, 0.0);  // 우측 하단
    glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 2.0, 0.0);  // 우측 상단
    glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 2.0, 0.0); // 좌측 상단
    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void DrawBall() {
    glPushMatrix();
    glTranslatef(ball_x, 0.5, ball_z);
    glColor3f(1.0, 0.0, 0.0);
    glutSolidSphere(0.5, 20, 20);
    glPopMatrix();
}

void Update(int value) {
    ball_x += ball_direction * 0.05;
    if (ball_x > 3.0 || ball_x < -3.0) ball_direction = -ball_direction;

    light_mix_factor += light_mix_direction * 0.01;
    if (light_mix_factor > 1.0 || light_mix_factor < 0.0) light_mix_direction = -light_mix_direction;

    GLfloat mixed_light[4];
    for (int i = 0; i < 4; i++) {
        mixed_light[i] = (1.0 - light_mix_factor) * sunlight_diffuse[i] + light_mix_factor * sunset_diffuse[i];
    }
    glLightfv(GL_LIGHT0, GL_DIFFUSE, mixed_light);

    glutPostRedisplay();
    glutTimerFunc(16, Update, 0);
}

void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DrawGround();
    DrawTree(-2.0, -2.0);
    DrawTree(2.0, 2.0);
    DrawSign();  // 입간판 추가
    DrawBall();
    glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y) {
    if (key == 'h') {
        printf("사용법:\n");
        printf("H: 도움말 표시\n");
        printf("S: 석양 효과\n");
        printf("D: 아침 햇빛 효과\n");
    }
    else if (key == 's') {
        glLightfv(GL_LIGHT0, GL_DIFFUSE, sunset_diffuse);
    }
    else if (key == 'd') {
        glLightfv(GL_LIGHT0, GL_DIFFUSE, sunlight_diffuse);
    }
    glutPostRedisplay();
}

void Init() {
    // 텍스처 초기화
    GenerateTexture();
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 광원 및 기본 설정
    GLfloat sunlight_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };  // 강한 아침 햇빛
    GLfloat sunlight_ambient[] = { 0.3, 0.3, 0.3, 1.0 };  // 약간의 환경광
    GLfloat light_position[] = { 0.0, 10.0, 10.0, 0.0 };  // 방향성 광원 (태양)
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunlight_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunlight_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // 보조 광원 설정
    GLfloat light_position2[] = { -5.0, 5.0, 5.0, 0.0 };  // 보조 광원
    GLfloat light_diffuse2[] = { 0.5, 0.5, 0.5, 1.0 };    // 보조 광원의 세기
    glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse2);
    glEnable(GL_LIGHT1);  // 보조 광원 활성화

    // 배경광 설정
    GLfloat global_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    glEnable(GL_DEPTH_TEST);
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 5.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Park Scene");

    Init();
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutTimerFunc(16, Update, 0);
    glutMainLoop();
    return 0;
}
