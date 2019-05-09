#pragma once

#include <string>
#include <vector>

#define CURRENT_FILE_VERSION 1

class Document{

    int width;
    int height;

    bool needsSave;

    std::vector<char*> data;

    std::string filename;

public:

    Document(std::string filename);
    ~Document();

    void initalizeEmpty(int width, int height);
    bool loadFromFile();
    bool saveToFile(std::string writeName);
    bool saveToFile();
    char get(int frame, int x, int y, char ifError);
    void set(int frame, int x, int y, char c);
    void insert(int frame, int x, int y, char c);
    void backspace(int frame, int x, int y, char c);
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