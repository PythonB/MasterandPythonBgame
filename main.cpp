#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <iostream>
#include <cstring>
#include <signal.h>
#include <vector>
#include <time.h>
#include <algorithm>

int RGB = 0;

class Entity {
private:
public:
    char a; // аватар
    char* title; // название сущности
    Entity(char _a) {
        a = _a;
    }
};

class Animal : public Entity {
private:
public:
    int hp, maxhp; // здоровье

    Animal(char _a) : Entity(_a) {
    }
};

class Mouse : public Animal {
private:
public:
    Mouse() : Animal('o') {
    hp = 100;
    maxhp = 100;
    }
};

class Grass : public Entity {
private:
public:
    Grass() : Entity('w') {
    }
};

class Player : public Entity {
private:
    Player() : Entity('@') {
    }
    static Player * player;
public:
    static Player* getInstance() {
        if (Player::player == NULL) {
            Player::player = new Player();
        }
        return Player::player;
    }
};

Player * Player::player = 0;

class Map {
private:
public:
    int width, height, playerX, playerY;
    std::vector<Entity*> ** grid;

    Map(int _w, int _h) {
        width = _w;
        height = _h;
        playerX = -1;
        playerY = -1;

        grid = (std::vector<Entity*>**)malloc(sizeof(std::vector<Entity*>*)*height);
        for (int i = 0; i < height; i++) {
            grid[i] = (std::vector<Entity*>*)malloc(sizeof(std::vector<Entity*>)*width);
        }
    }

    void dullFill() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                    if (rand()%10<2) {
                        if (rand()%2) grid[i][j].push_back(new Mouse());
                        else grid[i][j].push_back(new Grass());
                    }
            }
        }
    }

    ~Map() {
        for (int i = 0; i < height; i++) {
                free(grid[i]);
        }
    }

    int moveX(int d) {
        if (playerX >= 0) { // если игрок поставлен на карту
            if ((d > 0 && playerX + d < width ) || (d < 0 && playerX + d >= 0)) {
                grid[playerY][playerX].erase(std::find(grid[playerY][playerX].begin(), grid[playerY][playerX].end(), Player::getInstance()));
                playerX += d;
                grid[playerY][playerX].push_back(Player::getInstance());
                return 1;
            }
            else if (d) return 0;
            else return 1;
        }
        else return 0;
    }

    int moveY(int d) {
        if (playerY >= 0) { // если игрок поставлен на карту
            if ((d > 0 && playerY + d < height) || (d < 0 && playerY + d >= 0)) {
                grid[playerY][playerX].erase(std::find(grid[playerY][playerX].begin(), grid[playerY][playerX].end(), Player::getInstance()));
                playerY += d;
                grid[playerY][playerX].push_back(Player::getInstance());
                return 1;
            }
            else if (d) return 0;
            else return 1;
        }
        else return 0;
    }

    void render(int viewX, int viewY, int viewWidth, int viewHeight, int mapX, int mapY) {
        int a, b, c;
        int d, e, f;

        if (viewHeight <= height) {
            a = viewY;
            b = viewHeight+viewY;
            c = -viewY;
        }
        else {
            a = 0;
            b = height;
            c = (viewHeight-height)/2;
        }

        if (viewWidth <= width) {
            d = viewX;
            e = viewWidth+viewX;
            f = -viewX;
        }
        else {
            d = 0;
            e = width;
            f = (viewWidth-width)/2;
        }


        for (int i = a; i < b; i++) {
            for (int j = d; j < e; j++) {
                if (grid[i][j].size()) mvaddch(mapY+i+c, mapX+j+f, grid[i][j][grid[i][j].size()-1]->a);
                else mvaddch(mapY+i+c, mapX+j+f, ' ');
            }
        }
    }

    int putPlayer(int _x, int _y) {
        grid[_y][_x].push_back(Player::getInstance());
        playerX = _x;
        playerY = _y;
    }

    int popPlayer() {
        grid[playerY][playerX].erase(std::find(grid[playerY][playerX].begin(), grid[playerY][playerX].end(), Player::getInstance()));
        playerX = -1;
        playerY = -1;
    }
};

class Window {
private:
public:
    int x, y, width, height;
    char bLow = '-', bUp = '-', bLeft = '|', bRight = '|';
    // Это символы, которыми отрисовываются границы окна.
    // Внимание! Границы рисуются вне самого окна, то есть, например, верхняя граница имеет следующие коорд-ы:
    // (x-1, y-1, x+width+1, y-1)

    Window(int _x, int _y, int _w, int _h) {
        x = _x;
        y = _y;
        width = _w;
        height = _h;
    }

    // Отрисовка границ окна
    virtual void renderBoard() {
        for (int i = 0; i < width; i++) mvaddch(y+height,x+i,bLow); // верхняя
        for (int i = 0; i < width; i++) mvaddch(y-1,x+i,bUp); // нижняя
        for (int i = 0; i < height; i++) mvaddch(y+i,x-1,bLeft); // левая
        for (int i = 0; i < height; i++) mvaddch(y+i,x+width,bRight); // правая
    }

    // Отрисовка окна целиком
    virtual void render() {
        renderBoard();
    }

    // Обработчик изменения размеров окна
    virtual void resize(int _x, int _y, int _w, int _h) {
        x = _x;
        y = _y;
        width = _w;
        height = _h;
    }

    ~Window() {
    }
};

class MapWindow : public Window {
private:
public:
    Map * map;

    int viewX, viewY;
    // Координаты текущей позиции на карте, с которой надо начинать отрисовку.
    // Внимание! Это координаты относительно карты, а не окна!

    MapWindow(int _x, int _y, int _w, int _h) : Window(_x, _y, _w, _h) {
        map = new Map(50, 10); // это для дебага ширина карты такая, потом надо будет изменить
    }

    ~MapWindow() {
        if (map) {
            delete map;
            free(map);
        }
    }

    // Обработчик движения по горизонтали, где d - приращение координаты.
    void handleMoveX(int d) {
        if ((d > 0 && viewX+d+width <= map->width && map->playerX > width/2) ||
        (d < 0 && viewX+d >= 0 && map->playerX < map->width-width/2)) {
            viewX+=d;
        }
        map->moveX(d);
    }

    // Обработчик движения по вертикали, где d - приращение координаты.
    void handleMoveY(int d) {
        if ((d > 0 && viewY+d+height <= map->height && map->playerY > height/2) ||
        (d < 0 && viewY+d >= 0 && map->playerY < map->height-height/2) ) {
            viewY+=d;
        }
        map->moveY(d);
    }

    // Для дебага, наполняет карту сущностями
    void dullFill() {
        map->dullFill();
        map->putPlayer(0,0);
    }

    // Отрисовка карты
    void renderMap() {
        map->render(viewX, viewY, width, height, x, y);
    }

    // Отрисовка окна целиком
    void render() {
        renderBoard();
        renderMap();
    }
};

class InfoWindow : public Window {
private:
public:
    char* info;
    // Текущее содержание информационной строки

    int infoLen;
    // Длина информационной строки

    InfoWindow(int _x, int _y, int _w, int _h) : Window(_x, _y, _w, _h) {
    }

    ~InfoWindow() {
        if (info) free(info);
    }

    // Для дебага, задает "Hello World" как инфостроку
    void dullFill() {
        infoLen = 12;
        info = (char*)malloc(sizeof(char)*infoLen);
        strcpy(info, "Hello World!");
    }

    // Отрисовка текста инфостроки по строкам
    void renderText() {
        if (info) {
            int i = 0;
            for (; i < height && i < infoLen/width; i++) {
                mvprintw(y+i,x,"%.*s",width,info+i*width);
            }
            mvprintw(y+i, x, "%s",info+i*width);
        }
    }

    // Отрисовка инфоокна
    void render() {
        renderBoard();
        renderText();
    }
};

InfoWindow * test = new InfoWindow(1, 1, 5, 5);
MapWindow * testMap = new MapWindow(60, 60, 50, 0);


// Обработчик сигнала изменения размеров окна.
// Перед изменением размеров окна обязательно надо закрыть старое окно endwin()-ом и через refresh() создать новое!
// В конце обязательно сделать refresh(), чтобы вывести внесенные render()-ом изменения буфера на экран.
void sig_handler(int signum) {
    endwin();
    refresh();
    clear();
    testMap->resize(0,0,COLS,LINES/3*2);
    test->resize(0,LINES/3*2+1,COLS,LINES/3);
    test->render();
    testMap->render();
    refresh();
}

int main() {
    initscr();
    signal(SIGWINCH,sig_handler);
    srand (time(NULL));
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    testMap->resize(0,0,COLS,LINES/3*2);
    test->resize(0,LINES/3*2+1,COLS,LINES/3);

    testMap->dullFill();
    test->dullFill();

    while (1) {
        char ch = getch();
        switch(ch)
        {
            case 'a': {
                testMap->handleMoveX(-1);
                break;
            }
            case 'd': {
                testMap->handleMoveX(1);
                break;
            }
            case 'w': {
                testMap->handleMoveY(-1);
                break;
            }
            case 's': {
                testMap->handleMoveY(1);
                break;
            }
        }

        testMap->render();
        test->render();

        refresh();
    }

    endwin();
    return 0;
}

