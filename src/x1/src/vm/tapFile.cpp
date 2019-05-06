/*
	Author : shikarunochi
	Date   : 2019.05.04-

	[ tapFile ]
*/

//APSS 処理未対応

#include "tapFile.h"

void TAPFILE::initialize(FILEIO *parent_play_fio){
    if(tapeBuffer == NULL){
        tapeBuffer = (uint8_t*)ps_malloc(TAPBUFFERSIZE);
    }
    play_fio = parent_play_fio;
    size = play_fio->FileLength() - play_fio->Ftell();//現在の位置以降がデータファイルサイズ
    Serial.printf("FileLength:%d, FilePos:%d\n", play_fio->FileLength(),play_fio->Ftell());
    bufferPosition = 0;
	bitPosition = 0;
    tapePosition = 0;
    play_fio->Fread(tapeBuffer,TAPBUFFERSIZE,1);

}
uint8_t TAPFILE::nextBuffer(){
    int bit = 0x80 >> bitPosition;
    uint8_t data = tapeBuffer[bufferPosition];
    uint8_t returnData = ((data & bit) != 0) ? 255 : 0;
    //if(bitPosition == 0){
    //    Serial.printf("\n%d:[%X][%X]",tapePosition,bit,data);
    //}
    //Serial.printf("%s",((data & bit) != 0) ? "1" : "0");
    bitPosition++;
    if(bitPosition >= 8){
        bufferPosition++;
        bitPosition = 0;
        tapePosition++;
        if(bufferPosition >= TAPBUFFERSIZE){
           bufferPosition = 0;
           bitPosition = 0;
           play_fio->Fread(tapeBuffer,TAPBUFFERSIZE,1);
        }
        //Serial.printf("POS:%d, [%X]SIZE:%d\n",tapePosition,data,size);
    }
    return returnData;
}

int TAPFILE::getTapePercent(){ //Percent
    if(size > 0){
        return (int)(((double)tapePosition / (double)size) * 100.0);
    }
    return 0;
}
int TAPFILE::getTapeSize(){
    return size;
}
int TAPFILE::getTapePosition(){
    return tapePosition;
}