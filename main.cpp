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
int color_pair_counter = 1;

// Массив, сопоставляющий символ тайла и его цвета.
// тайл | foreground color | background color
char tilesColor[][3] = {
    {'-', 11, 11},
    {'u', 5, 5},
};

// Количество различных тайлов в массиве.
int tilesTypes = 2;

// Возвращает:
// номер цвета - если указанный тайл есть в массиве
// -1 - если указанный тайл не найден
int getFColor(char c) {
    for (int i = 0; i < tilesTypes; i++) {
        if (tilesColor[i][0] == c) {
            return tilesColor[i][1];
        }
    }
    return -1;
}
int getBColor(char c) {
    for (int i = 0; i < tilesTypes; i++) {
        if (tilesColor[i][0] == c) {
            return tilesColor[i][2];
        }
    }
    return -1;
}

// Функция для получения цветовой пары на основании двух цветов.
// Если она присутствует в списке цветовых пар, то возвращается ее номер.
// Если отсутствует, то создаётся новая и возвращается её порядковый номер.
int getPair(int f, int b) {
    short int i_f, i_b;
    for (int i = 1; i < color_pair_counter; i++) {
        pair_content(i, &i_f, &i_b);
        if (i_f == f && i_b == b) {
            return i;
        }
    }
    init_pair(color_pair_counter, f, b);
    return color_pair_counter++;
}

class Entity {
private:
public:
    char a; // аватар
    char* title; // название сущности
    int color; // 0-256 по VGA

    Entity() {
        a = '?';
        title = nullptr;
        color = 1;
    }
    Entity(char _a, const char* _t, int _c) {
        a = _a;
        title = strdup(_t);
        color = _c;
    }
};

class Actor : public Entity {
private:
public:
    Actor(char _a, const char* _t, int _c) : Entity(_a, _t, _c) {
    }
};

class Statics : public Entity {
private:
public:
    Statics(char _a, const char* _t, int _c) : Entity(_a, _t, _c) {
    }
};

class Animal : public Actor {
private:
public:
    int hp, defhp; // здоровье
    int sight;
    Animal(char _a, const char* _t, int _c) : Actor(_a, _t, _c) {
    }
};

class Mouse : public Animal {
private:
public:
    Mouse() : Animal('q', "Mouse", 4) {
        hp = 100;
        defhp = 100;
    }
};

class Grass : public Statics {
private:
public:
    Grass() : Statics('w', "Grass", 2) {
    }
};

class Player : public Actor {
private:
    Player() : Actor('@', "Player", 1) {
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
    char ** tiles;
    Statics*** statics;
    Actor*** actors;

    Map(int _w, int _h) {
        width = _w;
        height = _h;
        playerX = -1;
        playerY = -1;

        // Создание трёх слоёв карты: тайлы, статика, актёры
        tiles = (char**)malloc(sizeof(char*)*height);
        statics = (Statics***)malloc(sizeof(Statics**)*height);
        actors = (Actor***)malloc(sizeof(Actor**)*height);
        for (int i = 0; i < height; i++) {
            tiles[i] = (char*)malloc(sizeof(char)*width);
            statics[i] = (Statics**)malloc(sizeof(Statics*)*width);
            actors[i] = (Actor**)malloc(sizeof(Actor*)*width);
        }
    }

    void dullFill() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                actors[i][j] = nullptr;
                statics[i][j] = nullptr;
                tiles[i][j] = '-';
                if (rand()%10<2) {
                    if (rand()%2) actors[i][j] = new Mouse();
                    else statics[i][j] = new Grass();
                }
                else actors[i][j] = nullptr;
            }
        }
    }

    ~Map() {
        for (int i = 0; i < height; i++) {
            free(tiles[i]);
            for (int j = 0; j < width; j++) {
                free(statics[i][j]);
                free(actors[i][j]);
            }
            free(statics[i]);
            free(actors[i]);
        }
        free(tiles);
        free(statics);
        free(actors);
    }

    // Заглушка для того, чтобы при дебаге смотреть на веселые движущиеся символы :-)
    int AI(int viewX, int viewY, int viewWidth, int viewHeight) {
        for (int i = viewY; i < viewY + viewHeight && i < height; i++) {
            for (int j = viewX; j < viewX + viewWidth && j < width; j++) {
                if (actors[i][j] && !(i == playerY && j == playerX)) {
                    if (rand()%10 > 8) {
                        if (i > 0 && !actors[i-1][j]) {
                            actors[i-1][j] = actors[i][j];
                            actors[i][j] = nullptr;
                        }
                        else if (j < width-1 && !actors[i][j+1]) {
                            actors[i][j+1] = actors[i][j];
                            actors[i][j] = nullptr;
                        }
                        else if (i < height-1 && !actors[i+1][j]) {
                            actors[i+1][j] = actors[i][j];
                            actors[i][j] = nullptr;
                        }
                        else if (j > 0 && !actors[i][j-1]) {
                            actors[i][j-1] = actors[i][j];
                            actors[i][j] = nullptr;
                        }
                    }
                }
            }
        }
        return 0;
    }

    // Обработка движения по горизонтали. Возвращает:
    // 0 - если игрок был успешно передвинут
    // 1 - если движение в указанную сторону было невозможно
    int moveX(int d) {
        if (playerX >= 0) { // если игрок поставлен на карту
            if ((d > 0 && playerX + d < width ) || (d < 0 && playerX + d >= 0)) {
                actors[playerY][playerX+d] = actors[playerY][playerX];
                actors[playerY][playerX] = nullptr;
                playerX += d;
                return 0;
            }
            else if (d) return 1;
            else return 0;
        }
        else return 1;
    }

    // Обработка движения по вертикали. Возвращает:
    // 0 - если игрок был успешно передвинут
    // 1 - если движение в указанную сторону было невозможно
    int moveY(int d) {
        if (playerY >= 0) { // если игрок поставлен на карту
            if ((d > 0 && playerY + d < height) || (d < 0 && playerY + d >= 0)) {
                actors[playerY+d][playerX] = actors[playerY][playerX];
                actors[playerY][playerX] = nullptr;
                playerY += d;
                return 1;
            }
            else if (d) return 0;
            else return 1;
        }
        else return 0;
    }

    // Отрисовка карты
    void render(int viewX, int viewY, int viewWidth, int viewHeight, int mapX, int mapY) {
        int a, b, c;
        int d, e, f;
        // a - Y-координата карты, с которой начинается отрисовка
        // b - Y-координата карты, которая является нижней границей отрисовки
        // c - корректировка Y-оси

        // d - X-координата карты, с которой начинается отрисовка
        // e - X-координата карты, которая является правой границей отрисовки
        // f - корректировка X-оси


        // Если в высоту окно меньше, чем карта, и её надо обрезать по oY:
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

        // Если в ширину окно меньше, чем карта, и её надо обрезать по oX:
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
                int fg, bg, ch;
                // Цвет переднего плана, цвет фона, буква, которую надо нарисовать

                // Данным блоком команд мы автоматически присваиваем указанным выше переменным значения
                // со слоя тайла, то есть символ тайла и его цвета. Потом мы пройдёмся по слоям
                // актёра и статики и, если найдём там какую-то сущность, изменим рисуемый символ и цвет
                // переднего плана.
                if (tiles[i][j]) {
                    ch = tiles[i][j];
                    fg = getFColor(tiles[i][j]);
                    bg = getBColor(tiles[i][j]);
                }
                else {
                    ch = 'N';
                    fg = COLOR_WHITE;
                    bg = COLOR_RED;
                }

                // Преимущественно мы обрабатываем слой актёров, т.к. даже если под актёром находится
                // статика, её будет "не видно", так как в нашей игре слой актёров "лежит" поверх слоя
                // статики.
                if (actors[i][j]) {
                    fg = actors[i][j]->color;
                    ch = actors[i][j]->a;
                }
                else if (statics[i][j]) {
                    fg=statics[i][j]->color;
                    ch=statics[i][j]->a;
                }

                // Поиск цветовой пары (или создание новой) и отрисовка ячейки карты.
                int k = getPair(fg, bg);
                attron(COLOR_PAIR(k));
                mvaddch(mapY+i+c, mapX+j+f, ch);
                attroff(COLOR_PAIR(k));
            }
        }
    }

    // Устанавливает объект игрока на карту. Возвращает:
    // 0 - игрок успешно поставлен
    // 1 - ошибка: на указанной координате находится другой актер
    int putPlayer(int _x, int _y) {
        if (!actors[_y][_x]) {
            actors[_y][_x] = Player::getInstance();
            playerX = _x;
            playerY = _y;
            return 0;
        }
        return 1;
    }

    // Убирает объект игрока с карты. Возвращает:
    // 0 - игрок успешно убран
    // 1 - ошибка: на указанной координате нет объекта игрока
    int popPlayer() {
        if (playerX >= 0 && playerY >= 0) {
            actors[playerY][playerX] = nullptr;
            playerX = -1;
            playerY = -1;
            return 0;
        }
        return 1;
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
        map->AI(viewX, viewY, width, height);
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
	if (!has_colors())
	{	endwin();
		printf("Error: your terminal does not support colors.\n");
		exit(1);
	}
	use_default_colors();
	start_color();


    signal(SIGWINCH,sig_handler);
    srand (time(NULL));
    cbreak();
    noecho();
    timeout(100);  //!!!!!!!!!!!!!!
    keypad(stdscr, TRUE);



    testMap->resize(0,0,COLS,LINES/3*2);
    test->resize(0,LINES/3*2+1,COLS,LINES/3);

    testMap->dullFill();
    test->dullFill();

    while (1) {
        char ch = getch();
        color_pair_counter = 1;
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

