/*
	Author : shikarunochi
	Date   : 2019.05.04-

	[ mztFile ]
*/

#include "mztFile.h"
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
#define MZT_SAMPLE_RATE 48000
//http://www2s.biglobe.ne.jp/~kachan/w2b/Wav2binConverts.htm
mztFormatData_t mztFormat[]={
    {MZT_SIGNAL_0,48000},

//    {MZT_BIT_LOW,22000},
//    {MZT_BIT_HIGH,40},
//    {MZT_BIT_LOW,41},
//    {MZT_HEADER,1},
//    {MZT_BIT_HIGH,1},

//    {MZT_SIGNAL_1,48},
//    {MZT_SIGNAL_0,48},
//    {MZT_SIGNAL_0,48000},
//    {MZT_BIT_LOW,11000},
//    {MZT_BIT_HIGH,20},
//    {MZT_BIT_LOW,21},
//    {MZT_BODY,1},
 //   {MZT_BIT_HIGH,1},
 //   {MZT_BIT_LOW,256}


   {MZT_BIT_LOW,22000},
   {MZT_BIT_HIGH,40},
    {MZT_BIT_LOW,40},
    {MZT_BIT_HIGH,1},
    {MZT_HEADER,1},
    {MZT_BIT_HIGH,1},
    {MZT_BIT_LOW,256},
    {MZT_HEADER,1},
    {MZT_BIT_HIGH,1},
    {MZT_SIGNAL_1,48},
    {MZT_SIGNAL_0,48},
    {MZT_SIGNAL_0,48000},
   {MZT_BIT_LOW,11000},
  {MZT_BIT_HIGH,20},
  {MZT_BIT_LOW,20},
    {MZT_BIT_HIGH,1},
    {MZT_BODY,1},
    {MZT_BIT_HIGH,1},
    {MZT_BIT_LOW,256}
};
#else
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
#endif
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
    beforeTapeLoadPhase = 0;

    adjustCount = 0;

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
    firstCount = 0;
    secondCount = 0;
    addCheckSumFlg = false;
}
uint8_t MZTFILE::nextBuffer(){
    if(tapeLoadPhase >= sizeof(mztFormat) / sizeof(mztFormatData_t)){
        tapePosition = size + 1;
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
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)

            if(signalCounter == 0){
                firstCount = MZ80B_LONG_PULSE;//(120.0 / 16.0 * MZT_SAMPLE_RATE / 22050.0 + 0.5); //= 16.3265306122449‬ + 0.5
                secondCount = MZ80B_LONG_PULSE;//(120.0 / 16.0 * MZT_SAMPLE_RATE / 22050.0 + 0.5); //= 16.3265306122449‬ + 0.5
                //adjustCount = adjustCount + 65306;
                //adjustCount = adjustCount + 90084;
                //if(adjustCount >= 100000){
                //    adjustCount = adjustCount - 100000;
                //    secondCount = secondCount + 1;
               // }
            }
#else
            firstCount = 24;
            secondCount = 29;
#endif
            signalCounter++;
            if(signalCounter < firstCount){
                return 0xFF;
            }else if(signalCounter <= firstCount + secondCount){
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
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
            if(signalCounter==0){
                firstCount = MZ80B_SHORT_PULSE;//(60.0 / 16.0 * MZT_SAMPLE_RATE / 22050.0 + 0.5); //= 8.163265306122449 + 0.5
                //secondCount = 8;//(60.0 / 16.0 * MZT_SAMPLE_RATE / 22050.0 + 0.5); 
                secondCount = MZ80B_SHORT_PULSE;//(60.0 / 16.0 * MZT_SAMPLE_RATE / 22050.0 + 0.5); 
                //adjustCount = adjustCount + 32653;
                //adjustCount = adjustCount + 16567;
                //if(adjustCount >= 100000){
                //    adjustCount = adjustCount - 100000;
                //    secondCount = secondCount + 1;
                //}
            }
#else
            firstCount = 11;
            secondCount = 15;
#endif
            signalCounter++;
            if(signalCounter < firstCount){
                return 0xFF;
            }else if(signalCounter <= firstCount + secondCount){
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
                adjustCount = 0;
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
                        //Serial.printf("%X:",tapeByte);
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
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
    bufferQueue[inQueuePosition].count = MZ80B_LONG_PULSE;//(120.0 / 16.0 * MZT_SAMPLE_RATE / 22050.0 + 0.5);
#else
    bufferQueue[inQueuePosition].count = 24;
#endif
    inQueuePosition++;
    if(inQueuePosition >= MZT_TAPEQUEUE_SIZE){
        inQueuePosition = 0;
    }
    bufferQueue[inQueuePosition].isHigh = FALSE;
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
    bufferQueue[inQueuePosition].count = MZ80B_LONG_PULSE;//(120.0 / 16.0 * MZT_SAMPLE_RATE / 22050.0 + 0.5);
    //adjustCount = adjustCount + 65306;
    //adjustCount = adjustCount + 90084;
    //if(adjustCount >= 100000){
    //    adjustCount = adjustCount - 100000;
    //    bufferQueue[inQueuePosition].count = bufferQueue[inQueuePosition].count + 1;
    //}
#else
    bufferQueue[inQueuePosition].count = 29;
#endif    
    inQueuePosition++;
    if(inQueuePosition >= MZT_TAPEQUEUE_SIZE){
        inQueuePosition = 0;
    }
}

void MZTFILE::addLow(){
    bufferQueue[inQueuePosition].isHigh = TRUE;
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
    bufferQueue[inQueuePosition].count = MZ80B_SHORT_PULSE;//(60.0 / 16.0 * MZT_SAMPLE_RATE / 22050.0 + 0.5);
#else
   bufferQueue[inQueuePosition].count = 11;
#endif
    inQueuePosition++;
    if(inQueuePosition >= MZT_TAPEQUEUE_SIZE){
        inQueuePosition = 0;
    }
    bufferQueue[inQueuePosition].isHigh = FALSE;
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
    //bufferQueue[inQueuePosition].count = 8;//(60.0 / 16 * MZT_SAMPLE_RATE / 22050.0 + 0.5);
    bufferQueue[inQueuePosition].count = MZ80B_LONG_PULSE;//(60.0 / 16 * MZT_SAMPLE_RATE / 22050.0 + 0.5);
    //adjustCount = adjustCount + 16567;
    //if(adjustCount >= 100000){
    //    adjustCount = adjustCount - 100000;
    //    bufferQueue[inQueuePosition].count = bufferQueue[inQueuePosition].count + 1;
    //}

#else
   bufferQueue[inQueuePosition].count = 15;
#endif
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