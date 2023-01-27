/*
	Author : shikarunochi
	Date   : 2019.05.04-

	[ mztFile ]
*/

#include "mztFile.h"

mztFormatData_t mztFormat[]={
    {MZT_SIGNAL_0,10000},
    {MZT_BIT_LOW,10000},
    {MZT_BIT_HIGH,40},
    {MZT_BIT_LOW,40},
    {MZT_BIT_HIGH,1},
    {MZT_HEADER,1},
    {MZT_BIT_HIGH,1},
    {MZT_BIT_LOW,256},
    {MZT_HEADER,1},
    {MZT_BIT_HIGH,1},
    {MZT_BIT_LOW,10000},
    {MZT_BIT_HIGH,20},
    {MZT_BIT_LOW,20},
    {MZT_BIT_HIGH,1},
    {MZT_BODY,1},
    {MZT_BIT_HIGH,1}
};

void MZTFILE::initialize(FILEIO *parent_play_fio){
    play_fio = parent_play_fio;
	int file_size =  play_fio->FileLength();
    Serial.printf("filesize:%d\n",file_size);
    play_fio->Fseek(0, FILEIO_SEEK_SET);
    if(file_size > 128){
		play_fio->Fread(header, 128, 1);
		size = header[0x12] | (header[0x13] << 8);
		offs = header[0x14] | (header[0x15] << 8);
		execAddr = header[0x16] | (header[0x17] << 8);
        Serial.printf("size:%X(%d) offs:%X execAddr:%X\n",size, size, offs, execAddr);
	}
    //Serial.println("readOK!");
	inQueuePosition = 0;
	outQueuePosition = -1;
	bufferCounter = 0;
    tapeLoadPhase = 0;

    //ヘッダまでのデータをキューに入れる
    //インフォメーション128バイト
    highCount = 0;
    for(int headerPosition = 0;headerPosition < 128;headerPosition++){
        uint8_t tapeByte = header[headerPosition];
        highCount = highCount + addByte(tapeByte);
    }
    //checkSum
    int cheackByteHi = (highCount >> 8) & 0xff; 
    addByte(cheackByteHi);
    int cheackByteLo = (highCount >> 0) & 0xff; 
    addByte(cheackByteLo);

    lastHeaderPosition = inQueuePosition;
    tapePosition = 0;
    highCount = 0;
    //Serial.println("TapeSetOK");
    signalCounter = 0;
    addCheckSumFlg = false;
}
uint8_t MZTFILE::nextBuffer(){
    if(tapeLoadPhase >= sizeof(mztFormat) / sizeof(mztFormatData_t)){
        return 0x00;
    }

    int type = mztFormat[tapeLoadPhase].type;
    int count = mztFormat[tapeLoadPhase].count;

    switch(type){
        case MZT_SIGNAL_0: //count回数、0x00 を返す
        {
            signalCounter++;
            if(count <= signalCounter){
                signalCounter = 0;
                bufferCounter = 0;
                //Serial.printf("Phase:%d End!\n",tapeLoadPhase);
                tapeLoadPhase++;                
            }
            return 0x00;
        }
	    case MZT_SIGNAL_1: //count回数、0xFF を返す
        {
            signalCounter++;
            if(count <= signalCounter){
                signalCounter = 0;
                bufferCounter = 0;
                //Serial.printf("Phase:%d End!\n",tapeLoadPhase);
                tapeLoadPhase++;
            }
            return 0xFF;
        }
	    case MZT_BIT_HIGH: //count回数、HIGH (0xFF 24回、0x00 29回)を返す
        {
            signalCounter++;
            if(signalCounter < 24){
                return 0xFF;
            }else if(signalCounter < 24 + 28){
                return 0x00;
            }else{
                signalCounter = 0;
                bufferCounter++;
            }
            if(bufferCounter >= count){
                signalCounter = 0;
                bufferCounter = 0;
                //Serial.printf("Phase:%d End!\n",tapeLoadPhase);
                tapeLoadPhase++;
            }
            return 0x00;
        }
	    case MZT_BIT_LOW: //count回数、LOW (0xFF 11回、0x00 15回)を返す
        {
            signalCounter++;
            if(signalCounter < 11){
                return 0xFF;
            }else if(signalCounter < 11 + 14){
                return 0x00;
            }else{
                signalCounter = 0;
                bufferCounter++;
            }
            if(bufferCounter >= count){
                signalCounter = 0;
                bufferCounter = 0;
                //Serial.printf("Phase:%d End!\n",tapeLoadPhase);
                tapeLoadPhase++;
            }
            return 0x00;
        }
	    case MZT_HEADER: //ヘッダーを返す
        {
            //ヘッダデータはbufferQueueに展開済み
            bool returnData = bufferQueue[bufferCounter].isHigh;
            int signalCount = bufferQueue[bufferCounter].count;
            signalCounter++;
            if(signalCount <= signalCounter){
                signalCounter = 0;
                bufferCounter++;
            }
            if(bufferCounter > lastHeaderPosition){
                signalCounter = 0;
                bufferCounter = 0;
                //Serial.printf("Phase:%d End!\n",tapeLoadPhase);
                tapeLoadPhase++;
            }
            return (returnData==TRUE?0xFF:0x00);
        }
	    case MZT_BODY: //ボディを返す
        {
            int remainQueue = 0;
            if(outQueuePosition == -1){//初回
                outQueuePosition = 0;
                inQueuePosition = 0;
                signalCounter = 0;
            }else{
            //残りキューが100以下の場合、次のキューを読んでおく
                if(outQueuePosition < inQueuePosition){
                    remainQueue = inQueuePosition - outQueuePosition;
                }else{
                    remainQueue = (inQueuePosition + MZT_TAPEQUEUE_SIZE) - outQueuePosition;
                }
            }
            if(remainQueue < 100){
                //データ X バイト
                if(tapePosition < size){
                    int readSize = MZT_TAPBUFFERSIZE;
                    readSize = play_fio->Fread(tapeBuffer,readSize,1);
                    for(int readPos = 0;readPos < readSize;readPos++){
                        uint8_t tapeByte = tapeBuffer[readPos];
                        highCount = highCount + addByte(tapeByte);
                    }
                    tapePosition = tapePosition + readSize;
                    //Serial.printf("Read Data[tapePos:%d][readSize:%d][size:%d]\n",tapePosition,readSize,size);
                }else{
                //データ終わったら
                //チェックサム 2バイト
                    if(addCheckSumFlg == false){
                        //Serial.println("Read CheckSum");
                        int cheackByteHi = (highCount >> 8) & 0xff; 
                        int cheackByteLo = (highCount >> 0) & 0xff; 
                        addByte(cheackByteHi);
                        addByte(cheackByteLo);
                        addCheckSumFlg = true;
                    }
                }
            }

            bool returnData = bufferQueue[outQueuePosition].isHigh;
            int signalCount = bufferQueue[outQueuePosition].count;
            signalCounter++;
            if(signalCount <= signalCounter){
                signalCounter = 0;
                //Serial.printf("ReadData:[in:%d][out:%d]\n",inQueuePosition,outQueuePosition);
                if(outQueuePosition == inQueuePosition)
                {       
                    signalCounter = 0;
                    bufferCounter = 0;
                    //Serial.printf("BodyLoadPhase:%d End!\n",tapeLoadPhase);
                    tapeLoadPhase++;
                }else{
                    outQueuePosition++;
                    if(outQueuePosition >= MZT_TAPEQUEUE_SIZE){
                        outQueuePosition = 0;
                    }
                }
            }

            return (returnData==TRUE?0xFF:0x00);
        }
    }

}

int MZTFILE::addByte(uint8_t byteData){
    addHigh();

    int highCount = 0;
    //Serial.printf("AddByte[%d][%X][",inQueuePosition,byteData);
    for(int bitPosition = 0;bitPosition < 8;bitPosition++){
       if((byteData) & (0x80 >> bitPosition)){
       //    Serial.print("1");
            addHigh();
            highCount++;
        }else{
        //    Serial.print("0");
            addLow();
        }
    }
    //Serial.println("]");
    return highCount;
}

void MZTFILE::addHigh(){
    bufferQueue[inQueuePosition].isHigh = TRUE;
    bufferQueue[inQueuePosition].count = 24;
    inQueuePosition++;
    if(inQueuePosition >= MZT_TAPEQUEUE_SIZE){
        inQueuePosition = 0;
    }
    bufferQueue[inQueuePosition].isHigh = FALSE;
    bufferQueue[inQueuePosition].count = 29;
    inQueuePosition++;
    if(inQueuePosition >= MZT_TAPEQUEUE_SIZE){
        inQueuePosition = 0;
    }
}

void MZTFILE::addLow(){
    bufferQueue[inQueuePosition].isHigh = TRUE;
    bufferQueue[inQueuePosition].count = 11;
    inQueuePosition++;
    if(inQueuePosition >= MZT_TAPEQUEUE_SIZE){
        inQueuePosition = 0;
    }
    bufferQueue[inQueuePosition].isHigh = FALSE;
    bufferQueue[inQueuePosition].count = 15;
    inQueuePosition++;
    if(inQueuePosition >= MZT_TAPEQUEUE_SIZE){
        inQueuePosition = 0;
    }
}


int MZTFILE::getTapePercent(){ //Percent
    if(size > 0){
        return (int)(((double)tapePosition / (double)size) * 100.0);
    }
    return 0;
}
int MZTFILE::getTapeSize(){
    return size;
}
int MZTFILE::getTapePosition(){
    return tapePosition;
}