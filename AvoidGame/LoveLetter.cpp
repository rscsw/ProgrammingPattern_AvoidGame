#define _CRT_SECURE_NO_WARNINGS

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <Windows.h>

#pragma comment(lib, "OpenGL32")

using namespace std;
//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

static GLuint texName;

int gameState = 0;
// 0 : 타이틀 화면
// 1 : 게임 방법
// 2 : 메인 게임
// 3 : 해피 엔딩
// 4 : 배드 엔딩
// 5 : 사망

typedef struct tagBITMAPHEADER {
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    RGBQUAD hRGB[256];
}BITMAPHEADER;

BYTE* LoadBitmapFile(BITMAPHEADER* bitmapHeader, int* imgSize, const char* filename)
{
    FILE* fp = fopen(filename, "rb");    //파일을 이진읽기모드로 열기
    if (fp == NULL)
    {
        printf("파일로딩에 실패했습니다.\n");    //fopen에 실패하면 NULL값을 리턴
        return NULL;
    }
    else
    {
        fread(&bitmapHeader->bf, sizeof(BITMAPFILEHEADER), 1, fp);    //비트맵파일헤더 읽기
        fread(&bitmapHeader->bi, sizeof(BITMAPINFOHEADER), 1, fp);    //비트맵인포헤더 읽기

        int imgSizeTemp = (bitmapHeader->bi.biWidth * bitmapHeader->bi.biHeight) * 3;    //이미지 사이즈 계산
        *imgSize = imgSizeTemp;    // 이미지 사이즈를 상위 변수에 할당

        BYTE *image = (BYTE*)malloc(sizeof(BYTE) *imgSizeTemp);    //이미지크기만큼 메모리할당
        fread(image, sizeof(BYTE), imgSizeTemp, fp);//이미지 크기만큼 파일에서 읽어오기

        BYTE B, R;

        for (int i = 0; i < imgSize[0]; i += 3) {

            B = image[i];
            R = image[i + 2];

            image[i] = R;
            image[i + 2] = B;
        }
        fclose(fp);

        return image;
    }
}

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

//상위 게임오브젝트
class GameObject
{
public:
    GameObject() {}
    ~GameObject() {}
    virtual void Input() {}
    virtual void Update() {}
    virtual void Render() {}
};

//플레이어
class Player : public GameObject
{
private:
    float r;
    float g;
    float b;
    float a;
    bool moveUp = false;
    bool moveDown = false;
    bool moveLeft = false;
    bool moveRight = false;
    float speed = 0.0005f;
public:
    float x;
    float y;
    float xp;
    float yp;
    Player(float x, float y, float xp, float yp, float r, float g, float b, float a)
    {
        this->x = x;
        this->y = y;
        this->xp = xp;
        this->yp = yp;
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
    ~Player() {}
    void Input()
    {
        if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8001) moveUp = true;
        else moveUp = false;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8001) moveDown = true;
        else moveDown = false;
        if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8001) moveLeft = true;
        else moveLeft = false;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8001) moveRight = true;
        else moveRight = false;
    }
    void Update()
    {
        if (moveUp) y += speed;
        if (moveDown) y -= speed;
        if (moveLeft) x -= speed;
        if (moveRight) x += speed;
    }
    void Render()
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor4f(r, g, b, a);
        glVertex2f(x, y + yp);
        glVertex2f(x + xp, y + yp);
        glVertex2f(x, y);
        glVertex2f(x + xp, y);
        glEnd();
    }
};

//미로 벽
class Wall : public GameObject
{
private:
    float r = 1.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
public:
    float x;
    float y;
    float xp;
    float yp;
    Wall(float x, float y, float xp, float yp)
    {
        this->x = x;
        this->y = y;
        this->xp = xp;
        this->yp = yp;
    }
    ~Wall() {}
    //void Input() {}
    void Update(Player p)
    {
        bool collision[2];
        collision[0] = false;
        collision[1] = false;

        //x축 연산
        if (x < p.x && x + xp > p.x) collision[0] = true;
        else if (p.x < x && p.x + p.xp > x) collision[0] = true;
        else if (x < p.x + p.xp && x + xp > p.x + p.xp) collision[0] = true;
        else if (p.x < x && p.x + p.xp > x + xp) collision[0] = true;
        else collision[0] = false;
        //y축 연산
        if (y < p.y + p.yp && p.y < y + yp) collision[1] = true;
        else if (p.y < y + yp && y < p.y + p.yp) collision[1] = true;
        else if (y < p.y && p.y + p.yp < y + yp) collision[1] = true;
        else if (p.y < y && p.y + p.yp > y + yp) collision[1] = true;
        else collision[1] = false;
        //겹쳐질 때☆
        if (collision[0] == true && collision[1] == true) gameState = 5;
    }
    void Render()
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor4f(r, g, b, a);
        glVertex2f(x, y + yp);
        glVertex2f(x + xp, y + yp);
        glVertex2f(x, y);
        glVertex2f(x + xp, y);
        glEnd();
    }
};

//흩어진 글자
class Letter : public GameObject
{
private:
    float r = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
public:
    float g = 0.6f;
    float x;
    float y;
    float xp;
    float yp;
    Letter(float x, float y, float xp, float yp)
    {
        this->x = x;
        this->y = y;
        this->xp = xp;
        this->yp = yp;
    }
    ~Letter() {}
    //void Input() {}
    void Update(Player p, int l)
    {
        bool collision[2];
        collision[0] = false;
        collision[1] = false;

        //x축 연산
        if (x < p.x && x + xp > p.x) collision[0] = true;
        else if (p.x < x && p.x + p.xp > x) collision[0] = true;
        else if (x < p.x + p.xp && x + xp > p.x + p.xp) collision[0] = true;
        else if (p.x < x && p.x + p.xp > x + xp) collision[0] = true;
        else collision[0] = false;
        //y축 연산
        if (y < p.y + p.yp && p.y < y + yp) collision[1] = true;
        else if (p.y < y + yp && y < p.y + p.yp) collision[1] = true;
        else if (y < p.y && p.y + p.yp < y + yp) collision[1] = true;
        else if (p.y < y && p.y + p.yp > y + yp) collision[1] = true;
        else collision[1] = false;
        //겹쳐질 때☆
        if (collision[0] == true && collision[1] == true) g = 1.0f;
    }
    void Render()
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor4f(r, g, b, a);
        glVertex2f(x, y + yp);
        glVertex2f(x + xp, y + yp);
        glVertex2f(x, y);
        glVertex2f(x + xp, y);
        glEnd();
    }
};

//골인 지점
class Goal : public GameObject
{
private:
    float r = 0.0f;
    float g = 0.0f;
    float b = 1.0f;
    float a = 1.0f;
public:
    float x;
    float y;
    float xp;
    float yp;
    Goal(float x, float y, float xp, float yp)
    {
        this->x = x;
        this->y = y;
        this->xp = xp;
        this->yp = yp;
    }
    ~Goal() {}
    //void Input() {}
    void Update(Player p, Letter l)
    {
        if (p.y < y)
        {
            if (l.g == 0.6f) gameState = 3;
            else gameState = 4;
        }
    }
    void Render()
    {
        glBegin(GL_TRIANGLE_STRIP);
        glColor4f(r, g, b, a);
        glVertex2f(x, y + yp);
        glVertex2f(x + xp, y + yp);
        glVertex2f(x, y);
        glVertex2f(x + xp, y);
        glEnd();
    }
};

void LoadMyImage(const char* filename)
{
    BITMAPHEADER originalHeader;	//비트맵의 헤더부분을 파일에서 읽어 저장할 구조체
    int imgSize;			//이미지의 크기를 저장할 변수
    BYTE* image = LoadBitmapFile(&originalHeader, &imgSize, filename); //비트맵파일을 읽어 화소정보를 저장
    if (image == NULL) return;        //파일 읽기에 실패하면 프로그램 종료

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 700, 700, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
}

int main(void)
{
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(700, 700, "Love Letter", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //플레이어 생성
    Player* player = new Player(-0.9f, 0.9f, 0.05f, 0.05f, 0.0f, 0.0f, 0.0f, 1.0f);

    //골 생성
    Goal* goal = new Goal(0.805f, -1.0f, 0.195f, 0.005f);

    //흩어진 글자
    Letter* letter[4];
    letter[0] = new Letter(-0.39f, 0.59f, 0.18f, -0.18f);
    letter[1] = new Letter(0.2f, 0.99f, 0.18f, -0.18f);
    letter[2] = new Letter(0.41f, 0.59f, 0.18f, -0.18f);
    letter[3] = new Letter(-0.79f, -0.21f, 0.18f, -0.18f);

    //쥰내 무식하게 미로 그리기
    Wall* w[49];
    w[0] = new Wall(-1.0f, 1.0f, 0.005f, 2.0f);
    w[1] = new Wall(-1.0f, 1.0f, 2.0f, 0.005f);
    w[2] = new Wall(1.0f, 1.0f, -0.005f, -2.0f);
    w[3] = new Wall(-1.0f, -1.0f, 1.805f, 0.005f);
    w[4] = new Wall(-1.0f, 0.805f, 0.405f, -0.01f);
    w[5] = new Wall(-0.405f, 1.0f, 0.01f, -0.805f);
    w[6] = new Wall(-0.605f, 0.805f, 0.01f, -0.41f);
    w[7] = new Wall(0.395f, 1.0f, 0.01f, -0.205f);
    w[8] = new Wall(0.595f, 0.805f, 0.21f, -0.01f);
    w[9] = new Wall(0.795f, 0.405f, 0.205f, -0.01f);
    w[10] = new Wall(0.795f, 0.605f, 0.01f, -0.21f);
    w[11] = new Wall(0.595f, 0.805f, 0.01f, -0.61f);
    w[12] = new Wall(-0.005f, 0.805f, 0.01f, -0.21f);
    w[13] = new Wall(0.195f, 0.805f, 0.01f, -0.81f);
    w[14] = new Wall(-0.005f, 0.602f, 0.21f, -0.01f);
    w[15] = new Wall(0.195f, 0.805f, 0.21f, -0.01f);
    w[16] = new Wall(-0.205f, 0.805f, 0.01f, -0.41f);
    w[17] = new Wall(-0.405f, 0.405f, 0.41f, -0.01f);
    w[18] = new Wall(-0.805f, 0.605f, 0.01f, -0.61f);
    w[19] = new Wall(-0.805f, 0.205f, 0.41f, -0.01f);
    w[20] = new Wall(0.395f, 0.605f, 0.21f, -0.01f);
    w[21] = new Wall(0.395f, 0.605f, 0.01f, -0.81f);
    w[22] = new Wall(0.395f, 0.005f, 0.21f, -0.01f);
    w[23] = new Wall(-0.005f, 0.405f, 0.01f, -0.61f);
    w[24] = new Wall(-1.0f, -0.795f, 0.41f, -0.01f);
    w[25] = new Wall(-1.0f, -0.195f, 0.405f, -0.01f);
    w[26] = new Wall(-0.805f, 0.005f, 0.41f, -0.01f);
    w[27] = new Wall(-0.405, 0.005f, 0.01f, -0.81f);
    w[28] = new Wall(-0.805f, -0.395f, 0.21f, -0.01f);
    w[29] = new Wall(-0.605f, -0.195f, 0.01f, -0.21f);
    w[30] = new Wall(-0.805f, -0.595f, 0.41f, -0.01f);
    w[31] = new Wall(-0.205f, 0.205f, 0.01f, -0.61f);
    w[32] = new Wall(-0.405f, -0.795f, 0.21f, -0.01f);
    w[33] = new Wall(-0.205f, -0.595f, 0.01f, -0.21f);
    w[34] = new Wall(-0.205f, -0.395f, 0.21f, -0.01f);
    w[35] = new Wall(-0.005f, -0.395f, 0.01f, -0.605f);
    w[36] = new Wall(0.795f, 0.205f, 0.01f, -0.41f);
    w[37] = new Wall(0.595f, 0.205f, 0.21f, -0.01f);
    w[38] = new Wall(0.195f, -0.395f, 0.41f, -0.01f);
    w[39] = new Wall(-0.005f, -0.195f, 0.21f, -0.01f);
    w[40] = new Wall(0.195f, -0.195f, 0.01f, -0.21f);
    w[41] = new Wall(-0.005f, -0.595f, 0.21f, -0.01f);
    w[42] = new Wall(0.195f, -0.595f, 0.01f, -0.21f);
    w[43] = new Wall(0.395f, -0.395f, 0.01f, -0.41f);
    w[44] = new Wall(0.595f, -0.195f, 0.01f, -0.21f);
    w[45] = new Wall(0.595f, -0.195f, 0.21f, -0.01f);
    w[46] = new Wall(0.595f, -0.595f, 0.21f, -0.01f);
    w[47] = new Wall(0.795f, -0.395, 0.01f, -0.41f);
    w[48] = new Wall(0.595f, -0.595f, 0.01f, -0.405f);

    //모은 글자 수
    int getLetter = 0;

    while (!glfwWindowShouldClose(window))
    {
        //_CrtDumpMemoryLeaks();
        glClear(GL_COLOR_BUFFER_BIT);

        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        //배경 그리기
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        switch (gameState)
        {
            case 0 : //타이틀 화면
            {
                LoadMyImage("title.bmp");

                glEnable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glColor3f(1, 1, 1);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(1, 1, 0);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(1, -1, 0);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1, -1, 0);
                glEnd();
                glDisable(GL_TEXTURE_2D);

                if (GetAsyncKeyState(VK_SPACE) & 0x8000 || GetAsyncKeyState(VK_SPACE) & 0x8001)
                {
                    gameState = 1;
                }

                break;
            }
            case 1: //게임 방법
            {
                LoadMyImage("howtoplay.bmp");

                glEnable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glColor3f(1, 1, 1);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(1, 1, 0);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(1, -1, 0);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1, -1, 0);
                glEnd();
                glDisable(GL_TEXTURE_2D);

                if (GetAsyncKeyState(VK_LCONTROL) & 0x8000 || GetAsyncKeyState(VK_LCONTROL) & 0x8001)
                {
                    gameState = 2;
                }

                break;
            }
            case 2: //메인 게임
            {
                player->Input();
                player->Update();
                player->Render();

                for (int i = 0; i < 49; i++)
                {
                    w[i]->Update(*player);
                    w[i]->Render();
                }

                for (int i = 0; i < 4; i++)
                {
                    letter[i]->Update(*player, getLetter);
                    letter[i]->Render();
                }

                for (int i = 0; i < 4; i++)
                {
                    goal->Update(*player, *letter[i]);
                    goal->Render();
                }

                break;
            }
            case 3: //해피 엔딩
            {
                LoadMyImage("happy.bmp");

                glEnable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glColor3f(1, 1, 1);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(1, 1, 0);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(1, -1, 0);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1, -1, 0);
                glEnd();
                glDisable(GL_TEXTURE_2D);

                break;
            }
            case 4: //배드 엔딩
            {
                LoadMyImage("bad.bmp");

                glEnable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glColor3f(1, 1, 1);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(1, 1, 0);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(1, -1, 0);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1, -1, 0);
                glEnd();
                glDisable(GL_TEXTURE_2D);

                if (GetAsyncKeyState(VK_LCONTROL) & 0x8000 || GetAsyncKeyState(VK_LCONTROL) & 0x8001)
                {
                    player->x = -0.9f;
                    player->y = 0.9f;
                    for (int i = 0; i < 4; i++) letter[i]->g = 0.6f;
                    gameState = 2;
                }

                break;
            }
            case 5: //사망
            {
                LoadMyImage("dead.bmp");

                glEnable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glColor3f(1, 1, 1);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1, 1, 0);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(1, 1, 0);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(1, -1, 0);
                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1, -1, 0);
                glEnd();
                glDisable(GL_TEXTURE_2D);

                if (GetAsyncKeyState(VK_SPACE) & 0x8000 || GetAsyncKeyState(VK_SPACE) & 0x8001)
                {
                    player->x = -0.9f;
                    player->y = 0.9f;
                    for (int i = 0; i < 4; i++) letter[i]->g = 0.6f;
                    gameState = 2;
                }

                break;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}