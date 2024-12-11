#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

// BMP 텍스처 데이터를 저장하기 위한 구조체 정의
typedef struct _AUX_RGBImageRec {
    GLint sizeX, sizeY;          // 이미지 크기
    unsigned char* data;         // 픽셀 데이터
} AUX_RGBImageRec;

// 텍스처 객체와 텍스처 이미지 데이터 포인터
unsigned int MyTextureObject[1];
AUX_RGBImageRec* pTextureImage[1];

// 창 크기 변경 시 호출되는 함수
void MyReshape(int w, int h) {
    glViewport(0, 0, w, h);  // 뷰포트를 창 크기에 맞게 설정
    glMatrixMode(GL_PROJECTION);  // 투영 행렬 설정
    glLoadIdentity();
    gluPerspective(40.0, (GLfloat)w / (GLfloat)h, 1.0, 100.0);  // 원근 투영 설정
    glMatrixMode(GL_MODELVIEW);  // 모델뷰 행렬 설정
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0,     // 카메라 위치
        0.0, 0.0, 0.0,     // 카메라가 바라보는 지점
        0.0, 1.0, 0.0);    // 카메라 상단 방향
}

// 장면을 렌더링하는 함수
void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 색상과 깊이 버퍼 초기화

    glBindTexture(GL_TEXTURE_2D, MyTextureObject[0]);  // 텍스처 바인딩

    // 사각형에 텍스처를 매핑하여 렌더링
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f); // 왼쪽 아래
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, 1.0f);  // 오른쪽 아래
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);   // 오른쪽 위
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);  // 왼쪽 위
    glEnd();

    glutSwapBuffers();  // 버퍼 교환
}

// BMP 파일을 읽어 텍스처 데이터로 변환하는 함수
AUX_RGBImageRec* LoadBMP(const char* filename) {
    std::ifstream file(filename, std::ios::binary);  // 바이너리 모드로 파일 열기
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return nullptr;
    }

    // BMP 파일 헤더 읽기
    BITMAPFILEHEADER bfh;
    file.read(reinterpret_cast<char*>(&bfh), sizeof(BITMAPFILEHEADER));

    if (bfh.bfType != 0x4D42) {  // 파일 형식 확인 ("BM")
        std::cerr << "Not a BMP file: " << filename << std::endl;
        return nullptr;
    }

    // BMP 정보 헤더 읽기
    BITMAPINFOHEADER bih;
    file.read(reinterpret_cast<char*>(&bih), sizeof(BITMAPINFOHEADER));

    if (bih.biBitCount != 24) {  // 24비트 BMP만 지원
        std::cerr << "Only 24-bit BMPs are supported." << std::endl;
        return nullptr;
    }

    // 텍스처 데이터 구조체 생성 및 초기화
    AUX_RGBImageRec* texture = new AUX_RGBImageRec;
    texture->sizeX = bih.biWidth;
    texture->sizeY = bih.biHeight;

    int imageSize = texture->sizeX * texture->sizeY * 3;  // 3(RGB) 바이트
    texture->data = new unsigned char[imageSize];

    // BMP 데이터 읽기 시작 위치로 이동
    file.seekg(bfh.bfOffBits, std::ios::beg);

    // BMP 행의 패딩 계산
    int padding = (4 - (texture->sizeX * 3) % 4) % 4;

    // BMP 데이터 읽기
    std::vector<unsigned char> row(texture->sizeX * 3 + padding);
    for (int y = texture->sizeY - 1; y >= 0; y--) {  // 아래에서 위로 읽기
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

// 텍스처를 OpenGL에 로드하는 함수
int LoadGLTextures(char* szFilePath) {
    int Status = FALSE;  // 성공 여부
    glClearColor(0.0, 0.0, 0.0, 0.5);  // 배경색 설정
    memset(pTextureImage, 0, sizeof(void*) * 1);  // 초기화

    // 텍스처 파일 로드
    if (pTextureImage[0] = LoadBMP(szFilePath)) {
        Status = TRUE;  // 성공
        glGenTextures(1, &MyTextureObject[0]);  // 텍스처 생성
        glBindTexture(GL_TEXTURE_2D, MyTextureObject[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, 3,
            pTextureImage[0]->sizeX, pTextureImage[0]->sizeY,
            0, GL_RGB, GL_UNSIGNED_BYTE, pTextureImage[0]->data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glEnable(GL_TEXTURE_2D);
    }
    if (pTextureImage[0]) {  // 메모리 해제
        if (pTextureImage[0]->data) {
            free(pTextureImage[0]->data);
        }
        free(pTextureImage[0]);
    }
    return Status;
}

// 프로그램 진입점
void main(int argc, char** argv) {
    const char* textureFilePath = "다람쥐주의.bmp";  // 텍스처 파일 경로

    glutInit(&argc, argv);  // GLUT 초기화
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("OpenGL Sample Program");  // 윈도우 생성
    glutDisplayFunc(Display);  // 디스플레이 콜백
    glutReshapeFunc(MyReshape);  // 리쉐이프 콜백

    // 텍스처 로드
    if (LoadGLTextures((char*)textureFilePath)) {
        glEnable(GL_TEXTURE_2D);  // 텍스처 활성화
        glShadeModel(GL_SMOOTH);  // 부드러운 음영 모드
        glClearDepth(1.0);  // 깊이 버퍼 초기화
        glEnable(GL_DEPTH_TEST);  // 깊이 테스트 활성화
        glDepthFunc(GL_LEQUAL);  // 깊이 테스트 조건
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        glutMainLoop();  // 이벤트 루프 시작
    }
    else {
        std::cerr << "Failed to load texture!" << std::endl;
    }
}
