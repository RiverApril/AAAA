

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

    data.push_back((char*)malloc(width*height));
    for(int i = 0; i < width*height; i++){
        data[0][i] = ' ';
    }
    needsSave = true;
}

bool Document::saveToFile(){
    return saveToFile(filename);
}

bool Document::saveToFile(string writeName){

    FILE* file = fopen(writeName.c_str(), "w");

    if(file){

        fprintf(file, "AAAA %d %d %d\n", CURRENT_FILE_VERSION, width, height);
        for(int i = 0; i < data.size(); i++){
            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    fputc(data[i][y*width+x], file);
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

            if(version == 1){

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
                            data.push_back((char*)malloc(width*height));
                            frame++;
                            for(int i = 0; i < width*height; i++){
                                data[frame][i] = ' ';
                            }
                        }
                        memcpy(data[frame] + y*width, line, width);
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


char Document::get(int frame, int x, int y, char ifError){
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

void Document::set(int frame, int x, int y, char c){
    if(frame >= 0 && frame < data.size() && x >= 0 && x < width && y >= 0 && y < height){
        data[frame][y*width+x] = c;
        needsSave = true;
    }
}

void Document::insert(int frame, int x, int y, char c){
    if(frame >= 0 && frame < data.size() && x >= 0 && x < width && y >= 0 && y < height){
        memcpy(data[frame] + y*width+x + 1, data[frame] + y*width+x, width-x-1);
        data[frame][y*width+x] = c;
        needsSave = true;
    }
}


void Document::backspace(int frame, int x, int y, char c){
    if(frame >= 0 && frame < data.size() && x >= 0 && x < width && y >= 0 && y < height){
        memcpy(data[frame] + y*width+x - 1, data[frame] + y*width+x, width-x);
        data[frame][(y+1)*width-1] = c;
        needsSave = true;
    }
}

void Document::insertLine(int frame, int y){
    if(frame >= 0 && frame < data.size() && y >= 0 && y < height){
        memcpy(data[frame] + (y+1)*width, data[frame] + y*width, width*height-(y+1)*width);
        for(int x = 0; x < width; x++){
            data[frame][y*width+x] = ' ';
        }
        needsSave = true;
    }
}

void Document::removeLine(int frame, int y){
    if(frame >= 0 && frame < data.size() && y >= 0 && y < height){
        memcpy(data[frame] + y*width, data[frame] + (y+1)*width, width*height-y*width);
        for(int x = 0; x < width; x++){
            data[frame][(height-1)*width+x] = ' ';
        }
        needsSave = true;
    }
}

 bool Document::resize(int newWidth, int newHeight){
    if(newWidth > 0 && newHeight > 0){
        for(int i = 0; i < data.size(); i++){
            char* newFrame = (char*)malloc(newWidth*newHeight);
            for(int y = 0; y < newHeight; y++){
                for(int x = 0; x < newWidth; x++){
                    newFrame[y*newWidth+x] = ' ';
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
        char* newFrame = (char*)malloc(width*height);
        data.insert(data.begin() + frame + 1, newFrame);
        
        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                data[frame+1][y*width+x] = copyCurrent?data[frame][y*width+x]:' ';
            }
        }
        needsSave = true;
    }
}

void Document::insertFrameBefore(int frame, bool copyCurrent){
    if(frame >= 0 && frame < data.size()){
        char* newFrame = (char*)malloc(width*height);
        data.insert(data.begin() + frame, newFrame);
        
        for(int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                data[frame][y*width+x] = copyCurrent?data[frame+1][y*width+x]:' ';
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
                data[frame][y*width+x] = ' ';
            }
        }
        needsSave = true;
    }
}

bool Document::doesNeedSave(){
    return needsSave;
}

