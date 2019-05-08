// Another Ascii Art Animator

#include <ncurses.h>
#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "Document.hpp"

using namespace std;

#define MODE_EDIT 0
#define MODE_PREVIEW 1

#define COMMAND_BUFFER_SIZE 255

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


Document* doc;
bool docNeedsName;

bool running;
int mode = MODE_EDIT;

char* commandBuffer;
int commandBufferIndex = 0;

int previewDelay = 250;
int displayFrame = 0;
int viewPanX = 0;
int viewPanY = 0;

int cursorX = 0;
int cursorY = 0;
bool insertMode = false;
bool stepForward = true;

bool enteringCommand = false;

string statusMessage = "";

void processInput(int in);


void segVHandle(int a){
    printf("SEGV, closing");
    endwin();
    exit(1);
}

bool fileExists(const char* filename){
    if (FILE *file = fopen(filename, "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}


void initalizeCurses(){

    setlocale(LC_ALL, "");

    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    curs_set(0);
    timeout(-1);

    start_color();
    use_default_colors();

    short a = 0;
    for (short i = 0; i < 0x10; i++) {
        for (short j = 0; j < 0x10; j++) {
            init_pair(a, j, i);
            a++;
        }
    }

    ESCDELAY = 1;
}

int getColorPair(unsigned char fg, unsigned char bg){
    return COLOR_PAIR(fg + (bg * 0x10));
}

void saveFile(string name, bool noName){
    if(docNeedsName && noName){
        statusMessage = "Must provide a file name.";
    }else{
        bool worked = doc->saveToFile(name);
        if(worked){
            docNeedsName = false;
            statusMessage = "Saved to "+name;
        }else{
            statusMessage = "Failed to save "+name;
        }
    }
}

void executeCommand(){
    const char* s = commandBuffer;
    const int l = commandBufferIndex;

    bool didSomething = false;

    if(l > 0){
        if(s[0] == ':'){
            if(l > 1){
                if(s[1] == 'w'){
                    string writeName = doc->getFilename();
                    bool noName = true;
                    if(l > 2){
                        if(s[2] == 'q'){
                            if(l > 4){
                                if(s[3] == ' '){
                                    writeName = string(s+4);
                                    noName = false;
                                }
                            }
                            if(!noName || !docNeedsName){
                                running = false;
                            }
                        }else if(s[2] == ' '){
                            if(l > 3){
                                writeName = string(s+3);
                                noName = false;
                            }
                        }
                    }
                    saveFile(writeName, noName);
                    didSomething = true;
                } if(s[1] == 'q'){
                    if(l == 2){
                        running = false;
                        didSomething = true;
                    }
                } else if(s[1] == 'p'){
                    if(l > 2){
                        try {
                            int fps = stoi(s+2);
                            if(fps>=1){
                                previewDelay = 1000/fps;
                            }
                        }catch(const std::exception& e){

                        }
                    }
                    mode = MODE_PREVIEW;
                    statusMessage = "Preview Mode, delay: "+to_string(previewDelay)+"ms";
                    timeout(previewDelay);
                    didSomething = true;
                } else if(s[1] == 'r'){
                    if(l > 3){
                        string params = s+3;
                        int newWidth, newHeight;
                        sscanf(params.c_str(), "%d %d", &newWidth, &newHeight);
                        bool worked = doc->resize(newWidth, newHeight);
                        if(worked){
                            statusMessage = "Resized to "+to_string(newWidth)+", "+to_string(newHeight);
                        }else{
                            statusMessage = "Failed to resize to "+to_string(newWidth)+", "+to_string(newHeight);
                        }
                        didSomething = true;
                    }
                }else if(s[1] == 'i'){
                    if(l == 2){
                        insertMode = !insertMode;
                        statusMessage = string("Insert mode ") + (insertMode?"on":"off");
                        didSomething = true;
                    }
                } else if(s[1] == 'f'){
                    if(l == 2){
                        stepForward = !stepForward;
                        statusMessage = string("Step Forward ") + (stepForward?"on":"off");
                        didSomething = true;
                    }
                } else if(s[1] == 'd'){
                    if(l == 2){
                        doc->removeLine(displayFrame, cursorY);
                        statusMessage = "Deleted Row";
                        didSomething = true;
                    }
                } else if(s[1] == 'b'){
                    if(l == 2){
                        doc->insertFrameBefore(displayFrame, false);
                        statusMessage = "Added empty frame";
                        didSomething = true;
                    }
                } else if(s[1] == 'B'){
                    if(l == 2){
                        doc->insertFrameBefore(displayFrame, true);
                        statusMessage = "Added duplicate frame";
                        didSomething = true;
                    }
                } else if(s[1] == 'n'){
                    if(l == 2){
                        doc->insertFrameAfter(displayFrame, false);
                        displayFrame++;
                        statusMessage = "Added empty frame";
                        didSomething = true;
                    }
                } else if(s[1] == 'N'){
                    if(l == 2){
                        doc->insertFrameAfter(displayFrame, true);
                        displayFrame++;
                        statusMessage = "Added duplicate frame";
                        didSomething = true;
                    }
                } else if(s[1] == 'R'){
                    if(l == 2){
                        if(doc->getFrameCount() > 0){
                            doc->removeFrame(displayFrame);
                        }else{
                            doc->clearFrame(displayFrame);
                        }
                        if(displayFrame >= doc->getFrameCount()){
                            displayFrame = doc->getFrameCount()-1;
                        }
                        statusMessage = "Removed frame";
                        didSomething = true;
                    }
                }
            }
        }
    }

    if(!didSomething && commandBufferIndex != 0){
        statusMessage = "Unknown Command: '"+string(commandBuffer)+"'";
    }

    commandBufferIndex = 0;
    commandBuffer[commandBufferIndex] = '\0';
    enteringCommand = false;
}

void checkCommand(){
    const char* s = commandBuffer;
    const int l = commandBufferIndex;

    bool didSomething = false;

    int reprocessIn;
    bool reprocess = false;

    if(l > 0){
        if(s[0] == ':'){
            if(l > 1){
                switch(s[1]){
                    case ':':{
                        reprocessIn = ':';
                        reprocess = true;
                        enteringCommand = false;
                        didSomething = true;
                        break;
                    }
                    case '>':
                    case '.':{
                        displayFrame++;
                        if(displayFrame >= doc->getFrameCount()){
                            displayFrame = 0;
                        }
                        statusMessage = string("frame: ")+to_string(displayFrame);
                        didSomething = true;
                        break;
                    }
                    case '<':
                    case ',':{
                        displayFrame--;
                        if(displayFrame < 0){
                            displayFrame = doc->getFrameCount()-1;
                        }
                        statusMessage = string("frame: ")+to_string(displayFrame);
                        didSomething = true;
                        break;
                    }
                    case 'i':
                    case 'f':{
                        executeCommand();
                        break;
                    }
                }
            }
        }
    }
    if(didSomething){
        if(reprocess){
            processInput(reprocessIn);
        }
        commandBufferIndex = 0;
        commandBuffer[commandBufferIndex] = '\0';
        enteringCommand = false;
    }
}

void processInput(int in){
    switch(mode){

        case MODE_EDIT:{
            if(enteringCommand){
                if(in >= ' ' && in <= '~'){
                    if(commandBufferIndex < COMMAND_BUFFER_SIZE){
                        commandBuffer[commandBufferIndex] = (char)in;
                        commandBufferIndex++;
                        commandBuffer[commandBufferIndex] = '\0';
                        checkCommand();
                    }else{
                        statusMessage = "Command Buffer full!";
                    }
                }else if(in == KEY_ENTER || in == '\n'){
                    executeCommand();
                }else if((in == KEY_BACKSPACE || in == 127) && commandBufferIndex > 0){
                    commandBufferIndex--;
                    commandBuffer[commandBufferIndex] = '\0';
                }
            }else{
                if(in == ':' && commandBufferIndex == 0){
                    enteringCommand = true;
                    processInput(in);
                }else if(in >= ' ' && in <= '~'){
                    if(insertMode){
                        doc->insert(displayFrame, cursorX, cursorY, in);
                    }else{
                        doc->set(displayFrame, cursorX, cursorY, in);
                    }
                    if(stepForward){
                        cursorX++;
                        if(cursorX >= doc->getWidth()){
                            cursorX = 0;
                            cursorY++;
                            if(cursorY >= doc->getHeight()){
                                cursorY = 0;
                            }
                        }
                    }
                } else if(in == 330){
                    doc->set(displayFrame, cursorX, cursorY, ' ');
                    if(insertMode){
                        cursorX++;
                        if(cursorX >= doc->getWidth()){
                            cursorX = 0;
                            cursorY++;
                            if(cursorY >= doc->getHeight()){
                                cursorY = 0;
                            }
                        }
                        doc->backspace(displayFrame, cursorX, cursorY, ' ');
                        cursorX--;
                        if(cursorX < 0){
                            cursorX = doc->getWidth() - 1;
                            cursorY--;
                            if(cursorY < 0){
                                cursorY = doc->getHeight()-1;
                            }
                        }
                    }
                } else if((in == KEY_BACKSPACE || in == 127)){
                    if(insertMode){
                        doc->backspace(displayFrame, cursorX, cursorY, ' ');
                    }
                    cursorX--;
                    if(cursorX < 0){
                        cursorX = doc->getWidth() - 1;
                        cursorY--;
                        if(cursorY < 0){
                            cursorY = doc->getHeight()-1;
                        }
                    }
                    if(!insertMode){
                        doc->set(displayFrame, cursorX, cursorY, ' ');
                    }
                } else if(in == '\n'){
                    if(insertMode){
                        doc->insertLine(displayFrame, cursorY);
                    }
                    cursorY++;
                    if(cursorY >= doc->getHeight()){
                        cursorY = 0;
                    }
                }else if(in == KEY_RIGHT){
                    cursorX++;
                    if(cursorX >= doc->getWidth()){
                        cursorX = 0;
                    }
                }else if(in == KEY_LEFT){
                    cursorX--;
                    if(cursorX < 0){
                        cursorX = doc->getWidth() - 1;
                    }
                }else if(in == KEY_DOWN){
                    cursorY++;
                    if(cursorY >= doc->getHeight()){
                        cursorY = 0;
                    }
                }else if(in == KEY_UP){
                    cursorY--;
                    if(cursorY < 0){
                        cursorY = doc->getHeight()-1;
                    }
                }else{
                    //statusMessage = "Unknown: "+to_string(in);
                }
            }
            
            break;
        }

        case MODE_PREVIEW:{
            if(in == 27){
                mode = MODE_EDIT;
                timeout(-1);
                statusMessage = "Command Mode";
            }
            break;
        }

    }
}

void drawUi(){
    erase();
    int row, col;
    getmaxyx(stdscr, row, col);
    int displayMinX = viewPanX;
    int displayMinY = viewPanY;
    int displayMaxX = displayMinX+col-1;
    int displayMaxY = displayMinY+row-3;

    for(int y = displayMinY; y <= displayMaxY; y++){
        move(y-displayMinY, 0);
        for(int x = displayMinX; x <= displayMaxX; x++){
            char c = doc->get(displayFrame, x, y, ' ');
            if((x == -1 || x == doc->getWidth()) && (y == -1 || y == doc->getHeight())){
                c = '+';
            }else if((y == -1 || y == doc->getHeight()) && (x >= 0 && x < doc->getWidth())){
                c = '-';
            }else if((x == -1 || x == doc->getWidth()) && (y >= 0 && y < doc->getHeight())){
                c = '|';
            }
            if(c < ' ' || c > '~'){
                addch('?');
                continue;
            }
            bool select = false;
            if(mode == MODE_EDIT){
                if(cursorX == x && cursorY == y){
                    select = true;
                }
            }
            if(select){
                attrset(getColorPair(C_DARK_BLACK, C_LIGHT_WHITE));
            }
            addch(c);
            if(select){
                attrset(getColorPair(C_DARK_BLACK, C_DARK_BLACK));
            }
        }
    }
    if(mode == MODE_EDIT){
        mvprintw(row-2, 0, "\"%s\"  [%d / %d]  (%d, %d)/(%d, %d)  %s %s", doc->getFilename().c_str(), displayFrame+1, doc->getFrameCount(), cursorX+1, cursorY+1, doc->getWidth(), doc->getHeight(), insertMode?"ins":"ovr", stepForward?"fwd":"sta");
    }else if(mode == MODE_PREVIEW){
        displayFrame++;
        if(displayFrame >= doc->getFrameCount()){
            displayFrame = 0;
        }
        mvprintw(row-2, 0, "FPS: %d, frame:[%d / %d]", 1000/previewDelay, displayFrame+1, doc->getFrameCount());
    }
    if(enteringCommand){
        mvprintw(row-1, 0, "%s_", commandBuffer);
    }else{
        mvprintw(row-1, 0, statusMessage.c_str());
    }
    refresh();
}

int getchSafe(){
    int g = getch();
    if(g == 27){
        timeout(0);
        int gg = getch();
        int ic = 1;
        while(gg != -1){
            g |= gg << 8*ic;
            gg = getch();
            ic++;
        }
        timeout(MODE_PREVIEW?previewDelay:-1);
    }
    return g;
}


int main(int argc, char* argv[]){

    const int default_width = 40;
    const int default_height = 20;
    
    printf("Initalizing curses...");
    initalizeCurses();

    signal(SIGSEGV, segVHandle);

    if(argc >= 2){
        docNeedsName = false;
        char* filename = argv[1];
        doc = new Document(filename);

        int width = default_width;
        int height = default_height;

        if(argc >= 3){
            width = stoi(argv[2]);
        }
        if(argc >= 4){
            height = stoi(argv[3]);
        }

        if(fileExists(filename)){
            bool loaded = doc->loadFromFile();
            if(loaded){
                statusMessage = string("Loaded ")+filename;
            }else{
                statusMessage = string("Failed to load ")+filename;
                doc->initalizeEmpty(width, height);
            }
        }else{
            statusMessage = string("New file ") + filename;
            doc->initalizeEmpty(width, height);
        }
    }else{
        docNeedsName = true;
        doc = new Document("untitled");
        doc->initalizeEmpty(default_width, default_height);
    }

    running = true;

    commandBuffer = (char*)malloc(COMMAND_BUFFER_SIZE+1);
    commandBuffer[0] = '\0';

    while(running){
        drawUi();
        int in = getchSafe();
        processInput(in);
    }

    free(commandBuffer);

    endwin();

    return 0;
}