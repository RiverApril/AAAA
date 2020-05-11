#pragma once

#include <string>
#include <vector>

#define VERSION_NORMAL 1
#define VERSION_COLOR 2

#define C_DARK_BLACK 0x0 // white when background is black
#define C_DARK_RED 0x1
#define C_DARK_GREEN 0x2
#define C_DARK_YELLOW 0x3
#define C_DARK_BLUE 0x4
#define C_DARK_MAGENTA 0x5
#define C_DARK_CYAN 0x6
#define C_DARK_WHITE 0x7

#define C_LIGHT_BLACK 0x8
#define C_LIGHT_RED 0x9
#define C_LIGHT_GREEN 0xA
#define C_LIGHT_YELLOW 0xB
#define C_LIGHT_BLUE 0xC
#define C_LIGHT_MAGENTA 0xD
#define C_LIGHT_CYAN 0xE
#define C_LIGHT_WHITE 0xF

#define C_LIGHT_GRAY C_DARK_WHITE
#define C_DARK_GRAY C_LIGHT_BLACK
#define C_WHITE C_LIGHT_WHITE
#define C_BLACK C_DARK_BLACK

struct cell{
    char text;
    unsigned char fg;
    unsigned char bg;
    cell(char t, unsigned char fg, unsigned char bg) : text(t), fg(fg), bg(bg){}
};



char colorToChar(unsigned char c);
unsigned char charToColor(char c);

class Document{

    int width;
    int height;

    bool needsSave;

    std::vector<cell*> data;

    std::string filename;

public:

    Document(std::string filename);
    Document(const Document *other);
    ~Document();

    void initalizeEmpty(int width, int height);
    bool loadFromFile();
    bool saveToFile(std::string writeName);
    bool saveToFile();
    cell get(int frame, int x, int y, cell ifError);
    void set(int frame, int x, int y, cell c);
    void insert(int frame, int x, int y, cell c);
    void backspace(int frame, int x, int y, cell c);
    void insertLine(int frame, int y);
    void removeLine(int frame, int y);
    bool resize(int newWidth, int newHeight);

    void insertFrameAfter(int frame, bool copyCurrent);
    void insertFrameBefore(int frame, bool copyCurrent);
    void removeFrame(int frame);
    void clearFrame(int frame);

    int getFrameCount();
    int getWidth();
    int getHeight();
    std::string getFilename();
    bool doesNeedSave();


};