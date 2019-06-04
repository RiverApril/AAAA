

#include "Document.hpp"

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;


Document::Document(std::string filename){
    this->filename = filename;
    needsSave = true;
}

Document::~Document(){
    for(int i = 0; i < data.size(); i++){
        free(data[i]);
    }
    data.clear();
}

void Document::initalizeEmpty(int width, int height){
    this->width = width;
    this->height = height;

    for(int i = 0; i < data.size(); i++){
        free(data[i]);
    }
    data.clear();

    data.push_back((cell*)malloc(width*height*(sizeof(cell))));
    for(int i = 0; i < width*height; i++){
        data[0][i] = cell(' ', 0, 0);
    }
    needsSave = true;
}

bool Document::saveToFile(){
    return saveToFile(filename);
}

char colorToChar(unsigned char c){
    switch(c){
        case C_LIGHT_BLACK: return 'k';
        case C_LIGHT_RED: return 'r';
        case C_LIGHT_GREEN: return 'g';
        case C_LIGHT_YELLOW: return 'y';
        case C_LIGHT_BLUE: return 'b';
        case C_LIGHT_MAGENTA: return 'm';
        case C_LIGHT_CYAN: return 'c';
        case C_LIGHT_WHITE: return 'w';

        case C_DARK_BLACK: return ' ';
        case C_DARK_RED: return 'R';
        case C_DARK_GREEN: return 'G';
        case C_DARK_YELLOW: return 'Y';
        case C_DARK_BLUE: return 'B';
        case C_DARK_MAGENTA: return 'M';
        case C_DARK_CYAN: return 'C';
        case C_DARK_WHITE: return 'W';
        default: return ' ';
    }
}

unsigned char charToColor(char c){
    switch(c){
        case 'w': return C_LIGHT_WHITE;
        case 'r': return C_LIGHT_RED;
        case 'g': return C_LIGHT_GREEN;
        case 'y': return C_LIGHT_YELLOW;
        case 'b': return C_LIGHT_BLUE;
        case 'm': return C_LIGHT_MAGENTA;
        case 'c': return C_LIGHT_CYAN;
        case 'k': return C_LIGHT_BLACK;

        case 'W': return C_DARK_WHITE;
        case 'R': return C_DARK_RED;
        case 'G': return C_DARK_GREEN;
        case 'Y': return C_DARK_YELLOW;
        case 'B': return C_DARK_BLUE;
        case 'M': return C_DARK_MAGENTA;
        case 'C': return C_DARK_CYAN;
        case 'K': return C_DARK_BLACK;
        case ' ': return C_DARK_BLACK;
        default: return 0;
    }
}

bool Document::saveToFile(string writeName){

    FILE* file = fopen(writeName.c_str(), "w");

    if(file){


        bool hasColor = false;
        for(int i = 0; i < data.size() && !hasColor; i++){
            for(int y = 0; y < height && !hasColor; y++){
                for(int x = 0; x < width && !hasColor; x++){
                    if(data[i][y*width+x].fg != 0 || data[i][y*width+x].bg != 0){
                        hasColor = true;
                    }
                }
            }
        }

        fprintf(file, "AAAA %d %d %d\n", hasColor?VERSION_COLOR:VERSION_NORMAL, width, height);
        for(int i = 0; i < data.size(); i++){
            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    fputc(data[i][y*width+x].text, file);
                }
                if(hasColor){
                    for(int x = 0; x < width; x++){
                        fputc(colorToChar(data[i][y*width+x].fg), file);
                    }
                    for(int x = 0; x < width; x++){
                        fputc(colorToChar(data[i][y*width+x].bg), file);
                    }
                }
                fputc('\n', file);
            }
        }

        fclose(file);

        filename = writeName;
        needsSave = false;
        return true;
    }
    return false;
}

bool Document::loadFromFile(){

    for(int i = 0; i < data.size(); i++){
        free(data[i]);
    }
    data.clear();

    FILE* file = fopen(filename.c_str(), "r");

    if (file) {

        try{
            char head[5];
            if(fgets(head, 5, file) == NULL){
                throw (string("Failed to read first line."));
            }
            if(head[0] != 'A' || head[1] != 'A' || head[2] != 'A' || head[3] != 'A'){
                throw (string("Head is not AAAA: ")+head);
            }

            int version;
            if(fscanf(file, "%d", &version) != 1){
                throw (string("Failed to read version number."));
            }

            if(version == VERSION_COLOR){

                int width, height;
                if(fscanf(file, "%d %d", &width, &height) != 2){
                    throw (string("Failed to read one or both of: width, height."));
                }
                
                char c;
                do{
                    c = fgetc(file);
                }while(!feof(file) && c != '\n');

                this->width = width;
                this->height = height;

                bool reading = true;
                int frame = -1;
                while(reading){
                    for(int y = 0; y < height; y++){
                        char line[width*4];
                        if(fgets(line, width*4, file) == NULL){
                            reading = false;
                            break;
                        }
                        if(y == 0){
                            data.push_back((cell*)malloc(width*height*(sizeof(cell))));
                            frame++;
                            for(int i = 0; i < width*height; i++){
                                data[frame][i] = cell(' ', 0, 0);
                            }
                        }
                        int i = 0;
                        for(int x = 0; x < width; x++){
                            data[frame][y*width + x].text = line[i];
                            i++;
                        }
                        for(int x = 0; x < width; x++){
                            data[frame][y*width + x].fg = charToColor(line[i]);
                            i++;
                        }
                        for(int x = 0; x < width; x++){
                            data[frame][y*width + x].bg = charToColor(line[i]);
                            i++;
                        }
                        //memcpy(data[frame] + y*width, line, width);
                    }
                }

            }else if(version == VERSION_NORMAL){

                int width, height;
                if(fscanf(file, "%d %d", &width, &height) != 2){
                    throw (string("Failed to read one or both of: width, height."));
                }
                
                char c;
                do{
                    c = fgetc(file);
                }while(!feof(file) && c != '\n');

                this->width = width;
                this->height = height;

                bool reading = true;
                int frame = -1;
                while(reading){
                    for(int y = 0; y < height; y++){
                        char line[width*2];
                        if(fgets(line, width*2, file) == NULL){
                            reading = false;
                            break;
                        }
                        if(y == 0){
                            data.push_back((cell*)malloc(width*height*(sizeof(cell))));
                            frame++;
                            for(int i = 0; i < width*height; i++){
                                data[frame][i] = cell(' ', 0, 0);
                            }
                        }
                        int i = 0;
                        for(int x = 0; x < width; x++){
                            data[frame][y*width + x].text = line[i];
                            data[frame][y*width + x].fg = 0;
                            data[frame][y*width + x].bg = 0;
                            i++;
                        }
                        //memcpy(data[frame] + y*width, line, width);
                    }
                }

            }else{
                throw (string("Unknown version number: ") + to_string(version) + ".");
            }


            fclose(file);
            needsSave = false;
            return true;
        }catch(string error){
            fclose(file);
            erase();
            mvprintw(0, 0, error.c_str());
            refresh();
            getch();
            return false;
        }
    }else{
        return false;
    }
    
}


cell Document::get(int frame, int x, int y, cell ifError){
    if(frame >= 0 && frame < data.size() && x >= 0 && x < width && y >= 0 && y < height){
        return data[frame][y*width+x];
    }
    return ifError;
}

int Document::getFrameCount(){
    return data.size();
}

int Document::getWidth(){
    return width;
}

int Document::getHeight(){
    return height;
}

string Document::getFilename(){
    return filename;
}

void Document::set(int frame, int x, int y, cell c){
    if(frame >= 0 && frame < data.size() && x >= 0 && x < width && y >= 0 && y < height){
        data[frame][y*width+x] = c;
        needsSave = true;
    }
}

void Document::insert(int frame, int x, int y, cell c){
    if(frame >= 0 && frame < data.size() && x >= 0 && x < width && y >= 0 && y < height){
        memcpy(data[frame] + y*width+x + 1, data[frame] + y*width+x, (width-x-1) * sizeof(cell));
        data[frame][y*width+x] = c;
        needsSave = true;
    }
}


void Document::backspace(int frame, int x, int y, cell c){
    if(frame >= 0 && frame < data.size() && x >= 0 && x < width && y >= 0 && y < height){
        memcpy(data[frame] + y*width+x - 1, data[frame] + y*width+x, (width-x)*sizeof(cell));
        data[frame][(y+1)*width-1] = c;
        needsSave = true;
    }
}

void Document::insertLine(int frame, int y){
    if(frame >= 0 && frame < data.size() && y >= 0 && y < height){
        memcpy(data[frame] + (y+1)*width, data[frame] + y*width, (width*height-(y+1)*width)*sizeof(cell));
        for(int x = 0; x < width; x++){
            data[frame][y*width+x] = cell(' ', 0, 0);
        }
        needsSave = true;
    }
}

void Document::removeLine(int frame, int y){
    if(frame >= 0 && frame < data.size() && y >= 0 && y < height){
        memcpy(data[frame] + y*width, data[frame] + (y+1)*width, width*height-y*width);
        for(int x = 0; x < width; x++){
            data[frame][(height-1)*width+x] = cell(' ', 0, 0);
        }
        needsSave = true;
    }
}

 bool Document::resize(int newWidth, int newHeight){
    if(newWidth > 0 && newHeight > 0){
        for(int i = 0; i < data.size(); i++){
            cell* newFrame = (cell*)malloc(newWidth*newHeight*sizeof(cell));
            for(int y = 0; y < newHeight; y++){
                for(int x = 0; x < newWidth; x++){
                    newFrame[y*newWidth+x] = cell(' ', 0, 0);
                }
            }
            
            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    if(x >= 0 && x < newWidth && y >= 0 && y < newHeight){
                        newFrame[y*newWidth+x] = data[i][y*width+x];
                    }
                }
            }
            free(data[i]);
            data[i] = newFrame;
        }
        width = newWidth;
        height = newHeight;
        needsSave = true;
        return true;
    }
    return false;
}

void Document::insertFrameAfter(int frame, bool copyCurrent){
    if(frame >= 0 && frame < data.size()){
        cell* newFrame = (cell*)malloc(width*height*sizeof(cell));
        data.insert(data.begin() + frame + 1, newFrame);
        
        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                data[frame+1][y*width+x] = copyCurrent?data[frame][y*width+x]:cell(' ', 0, 0);
            }
        }
        needsSave = true;
    }
}

void Document::insertFrameBefore(int frame, bool copyCurrent){
    if(frame >= 0 && frame < data.size()){
        cell* newFrame = (cell*)malloc(width*height*sizeof(cell));
        data.insert(data.begin() + frame, newFrame);
        
        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                data[frame][y*width+x] = copyCurrent?data[frame+1][y*width+x]:cell(' ', 0, 0);
            }
        }
        needsSave = true;
    }
}


void Document::removeFrame(int frame){
    if(frame >= 0 && frame < data.size()){
        free(data[frame]);
        data.erase(data.begin() + frame);
        needsSave = true;
    }
}

void Document::clearFrame(int frame){
    if(frame >= 0 && frame < data.size()){
        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                data[frame][y*width+x] = cell(' ', 0, 0);
            }
        }
        needsSave = true;
    }
}

bool Document::doesNeedSave(){
    return needsSave;
}

