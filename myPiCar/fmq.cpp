//用于树莓派的GPIO接口控制蜂鸣器播放两段旋律

#include "stdio.h"
#include "wiringPi.h"
#include "softTone.h"
#include "iostream"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"

using namespace std;


#define BuzPin 0   // 定义一个宏，BuzPin对应的值为0，即蜂鸣器连接的GPIO接口号为0
// 定义各音阶频率，以C调为例，L为低音，M为中音，H为高音，数字为音阶
#define CL1 131
#define CL2 147
#define CL3 165
#define CL4 175
#define CL5 196
#define CL6 221
#define CL7 248
#define CM1 262
#define CM2 294
#define CM3 330
#define CM4 350
#define CM5 393
#define CM6 441
#define CM7 495
#define CH1 525
#define CH2 589
#define CH3 661
#define CH4 700
#define CH5 786
#define CH6 882
#define CH7 990


// 定义两首歌的音符，用已定义的音阶宏表示
int song_1[]={CM3,CM5,CM6,CM3,CM2,CM3,CM5,CM6,CH1,CM6,CM5,CM1,CM3,
              CM2,CM2,CM3,CM5,CM2,CM3,CM3,CL6,CL6,CL6,CM1,CM2,
              CM3,CM2,CL7,CL6,CM1,CL5};
// 定义两首歌的每个音符的节拍，1表示1/8拍，2表示1/4拍，3表示3/8拍
int beat_1[]={1,1,3,1,1,3,1,1,1,1,1,1,1,1,3,1,1,
              3,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,
              1,3};

int song_2[]={CM1,CM1,CM1,CL5,CM3,CM3,CM3,CM1,CM1,CM3,CM5,CM5,CM4,
              CM3,CM2,CM2,CM3,CM4,CM4,CM3,CM2,CM3,CM1,CM1,CM3,
              CM2,CL5,CL7,CM2,CM1};

int beat_2[]={1,1,1,3,1,1,1,3,1,1,1,1,1,1,3,1,1,
              1,2,1,1,1,3,1,1,1,3,3,2,3};

int main(){
    // 初始化wiringPi库，如果失败则输出错误信息并返回1
    if(wiringPiSetup()==-1){
        cout<<"Setup wiringPi failed!"<<endl;
        return 1;
    }
    // 为蜂鸣器创建一个软件音调控制，如果失败则输出错误信息并返回1
    if(softToneCreate(BuzPin)==-1){
        cout<<"Setup softTone failed!"<<endl;
        return 1;
    }
    // 设置蜂鸣器的初始频率为300Hz
    softToneWrite(BuzPin, 300);
    
    while(1){ // 创建一个无限循环，使得蜂鸣器可以持续播放两首歌
        // 播放第一首歌曲
        for(int i=0;i<sizeof(song_1)/sizeof(int);i++){
            softToneWrite(BuzPin,song_1[i]); // 设置蜂鸣器播放的频率为song_1[i]
            delay(beat_1[i]*500); // 根据beat_1[i]的值来决定音符的持续时间
        }
        delay(200); // 在两首歌之间暂停0.2秒

        for(int i=0;i<sizeof(song_2)/sizeof(int);i++){
            softToneWrite(BuzPin,song_2[i]); // 设置蜂鸣器播放的频率为song_2[i]
            delay(beat_2[i]*500); // 根据beat_2[i]的值来决定音符的持续时间
        }
        delay(200); // 在两首歌之间暂停0.2秒
    }
    return 0;
}
