/*
	Author : shikarunochi
	Date   : 2020.05.23-

	[ wavFile ]
*/

//APSS 処理未対応

#include "wavFile.h"

void WAVFILE::initialize(FILEIO *parent_play_fio){
    if(tapeBuffer == NULL){
        tapeBuffer = (uint8_t*)ps_malloc(TAPBUFFERSIZE);
    }
    play_fio = parent_play_fio;
    size = play_fio->FileLength() - play_fio->Ftell();//現在の位置以降がデータファイルサイズ
    Serial.printf("FileLength:%d, FilePos:%d\n", play_fio->FileLength(),play_fio->Ftell());
    bufferPosition = 0;
	maxSample = 0;
    minSample = 0;
    prevSignal = false;
    tapePosition = 0;
    readDataCount = play_fio->Fread(tapeBuffer,TAPBUFFERSIZE,1);
    
    preCheckSampleSignal =  0;
    sampleSignal =  0;
}

uint8_t WAVFILE::nextBuffer(){
    int8_t returnData = 0x00;
    
    //-2048 ～ +2048の時は、0とする。
    if(bufferPosition < readDataCount){
        nextCheckSampleSignal =  (tapeBuffer[bufferPosition] - 128) * 256;
        if(-2048 < nextCheckSampleSignal && nextCheckSampleSignal < 2048){
            nextCheckSampleSignal = 0;
        }
    }else{
        nextCheckSampleSignal = 0;
    }

    bool signal = (sampleSignal > 0);
    bool preCheckSignal = (preCheckSampleSignal > 0);
    bool nextCheckSignal = (nextCheckSampleSignal > 0);

    //突然1か所だけ異なることはないはず
    if(preCheckSignal == nextCheckSignal){
        signal = preCheckSignal;
    }

    returnData = (signal ? 0xff : 0);

    bufferPosition++;
    tapePosition++;
    if(bufferPosition >= TAPBUFFERSIZE){
        bufferPosition = 0;
        readDataCount = play_fio->Fread(tapeBuffer,TAPBUFFERSIZE,1);
    }

    preCheckSampleSignal = sampleSignal;
    sampleSignal = nextCheckSampleSignal;

    return returnData;
}

int WAVFILE::getTapePercent(){ //Percent
    if(size > 0){
        return (int)(((double)tapePosition / (double)size) * 100.0);
    }
    return 0;
}
int WAVFILE::getTapeSize(){
    return size;
}
int WAVFILE::getTapePosition(){
    return tapePosition;
}