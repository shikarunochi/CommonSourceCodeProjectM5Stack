/*
	Author : shikarunochi
	Date   : 2020.05.25-

	[ standardCasFile ]
*/

//APSS 処理未対応

#include "standardCasFile.h"

void STANDARDCASFILE::initialize(FILEIO *parent_play_fio){
    if(tapeBuffer == NULL){
        tapeBuffer = (uint8_t*)ps_malloc(STANDARDCAS_BUFFERSIZE);
    }
    play_fio = parent_play_fio;
    size = play_fio->FileLength() - play_fio->Ftell();//現在の位置以降がデータファイルサイズ
    Serial.printf("FileLength:%d, FilePos:%d\n", play_fio->FileLength(),play_fio->Ftell());
    bufferPosition = 0;
    tapePosition = 0;
    play_fio->Fread(tapeBuffer,STANDARDCAS_BUFFERSIZE,1);
    
    dataCount = 0;
    currentData = tapeBuffer[0];
    bufferPosition++;
    checkCount = currentData & 0x7f;
}
uint8_t STANDARDCASFILE::nextBuffer(){
    
    dataCount++;
    if(dataCount < checkCount){
        return  (currentData & 0x80) ? 255 : 0;
    }
    dataCount = 0;
    
    currentData = tapeBuffer[bufferPosition];
    bufferPosition++;
    checkCount = currentData & 0x7f;
    tapePosition++;
    if(bufferPosition >= STANDARDCAS_BUFFERSIZE){
        bufferPosition = 0;
        play_fio->Fread(tapeBuffer,STANDARDCAS_BUFFERSIZE,1);
    }
    return (currentData & 0x80) ? 255 : 0;
}

int STANDARDCASFILE::getTapePercent(){ //Percent
    if(size > 0){
        return (int)(((double)tapePosition / (double)size) * 100.0);
    }
    return 0;
}
int STANDARDCASFILE::getTapeSize(){
    return size;
}
int STANDARDCASFILE::getTapePosition(){
    return tapePosition;
}