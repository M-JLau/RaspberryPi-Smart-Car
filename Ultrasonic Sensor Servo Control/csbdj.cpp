#include "iostream"
#include "stdio.h"
#include "wiringPi.h"
#include "stdlib.h"
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "string.h"
#include "netdb.h"
#include "sys/types.h"
#include <time.h>
#include "arpa/inet.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "softPwm.h"
#include "pca9685.h"
#include "softServo.h"
#include "softTone.h"


using namespace std;
#define PIN_BASE 300 // PCA9685的基准pin编号，用于通过该基准编号来控制PCA9685上的各个pin
#define HERTZ 50 //PWM频率，50Hz的频率，可以用来驱动舵机
#define BUFSIZE 512 // 缓冲区大小
#define BuzPin 0   // 定义一个宏，BuzPin对应的值为0，即蜂鸣器连接的GPIO接口号为0
#define BtnPin 24  // 定义一个宏，BtnPin对应的值为24，即按钮连接的GPIO接口号为24
#define Gpin 21    // 定义一个宏，Gpin对应的值为21，即LED灯绿色连接的GPIO接口号为21
#define Rpin 22    // 定义一个宏，Rpin对应的值为22，即LED灯红色连接的GPIO接口号为22

//轉動角度計算常量
#define MAX_PWM 4096 // 最大PWM值，PCA9685是12位的，所以最大值是2^12-1=4095

//角度計算時糾偏
#define max(x,y) ((x)>(y)?(x):(y)) // 宏定义，求最大值
#define min(x,y) ((x)<(y)?(x):(y)) // 宏定义，求最小值

#define Trig 25 // 超声波模块的Trig引脚连接的树莓派的GPIO25
#define Echo 26 // 超声波模块的Echo引脚连接的树莓派的GPIO26

void PWM_write(int servonum,float x); // 舵机控制函数
int calcTicks(float impulseMs, int hertz);

int PWMA = 1; // 定义左电机PWM引脚连接到GPIO1
int AIN2 = 2; // 定义左电机方向控制引脚连接到GPIO2
int AIN1 = 3; // 定义左电机方向控制引脚连接到GPIO3
int PWMB = 4; // 定义右电机PWM引脚连接到GPIO4
int BIN2 = 5; // 定义右电机方向控制引脚连接到GPIO5
int BIN1 = 6; // 定义右电机方向控制引脚连接到GPIO6

int myservo1 = 12; // 定义舵机引脚连接到PCA9685的12通道

// 控制电机向前运动
void t_up(unsigned int speed,unsigned int t_time)
{
    digitalWrite(AIN2,0);  // 设置AIN2为低电平，确定电机方向
    digitalWrite(AIN1,1);  // 设置AIN1为高电平，确定电机方向
    softPwmWrite(PWMA,speed);  // 控制左电机PWM值，即速度
    digitalWrite(BIN2,0);  // 设置BIN2为低电平，确定电机方向
    digitalWrite(BIN1,1);  // 设置BIN1为高电平，确定电机方向
    softPwmWrite(PWMB,speed);  // 控制右电机PWM值，即速度
    delay(t_time);  // 延时t_time毫秒
}


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








// 控制电机停止运动
void t_stop(unsigned int t_time)
{
    digitalWrite(AIN2,0);  // 设置AIN2为低电平
    digitalWrite(AIN1,0);  // 设置AIN1为低电平
    softPwmWrite(PWMA,0);  // 控制左电机PWM值为0，即停止
    digitalWrite(BIN2,0);  // 设置BIN2为低电平
    digitalWrite(BIN1,0);  // 设置BIN1为低电平
    softPwmWrite(PWMB,0);  // 控制右电机PWM值为0，即停止
    delay(t_time);  // 延时t_time毫秒
}

// 控制电机向后运动
void t_down(unsigned int speed,unsigned int t_time)
{
    digitalWrite(AIN2,1);  // 设置AIN2为高电平，确定电机方向
    digitalWrite(AIN1,0);  // 设置AIN1为低电平，确定电机方向
    softPwmWrite(PWMA,speed);  // 控制左电机PWM值，即速度
    digitalWrite(BIN2,1);  // 设置BIN2为高电平，确定电机方向
    digitalWrite(BIN1,0);  // 设置BIN1为低电平，确定电机方向
    softPwmWrite(PWMB,speed);  // 控制右电机PWM值，即速度
    delay(t_time);  // 延时t_time毫秒
}

// 控制电机向左转
void t_left(unsigned int speed,unsigned int t_time)
{
    digitalWrite(AIN2,1);
    digitalWrite(AIN1,0);
    softPwmWrite(PWMA,speed);
    digitalWrite(BIN2,0);
    digitalWrite(BIN1,1);
    softPwmWrite(PWMB,speed);
    delay(t_time);
}

// 控制电机向右转
void t_right(unsigned int speed,unsigned int t_time)
{
    digitalWrite(AIN2,0);
    digitalWrite(AIN1,1);
    softPwmWrite(PWMA,speed);
    digitalWrite(BIN2,1);
    digitalWrite(BIN1,0);
    softPwmWrite(PWMB,speed);
    delay(t_time);
}

// 初始化超声波模块函数
void ultraInit(void)
{
    pinMode(Echo,INPUT); // 设置超声波接收脚为输入模式
    pinMode(Trig,OUTPUT); // 设置超声波发射脚为输出模式
}

// 定义测量距离函数
float disMeasure(void)
{
    struct timeval tv1; // 定义时间结构体tv1
    struct timeval tv2; // 定义时间结构体tv2
    long start, stop; // 定义开始和结束时间
    float dis; // 定义距离

    digitalWrite(Trig, LOW); // 给触发脚位低电平2微秒，// Trig脚位输出低电平
    delayMicroseconds(2);

    digitalWrite(Trig, HIGH); // 给触发脚位高电平10微秒，// Trig脚位输出高电平
    delayMicroseconds(10);
    digitalWrite(Trig, LOW); // 给触发脚位低电平

    while(!(digitalRead(Echo) == 1)); // 等待接收脚位变高
    gettimeofday(&tv1, NULL); // 获取当前时间

    while(!(digitalRead(Echo) == 0)); // 等待接收脚位变低
    gettimeofday(&tv2, NULL); // 获取当前时间

    start = tv1.tv_sec * 1000000 + tv1.tv_usec; // 计算开始时间
    stop  = tv2.tv_sec * 1000000 + tv2.tv_usec; // 计算结束时间

    dis = (float)(stop - start) / 1000000 * 34000 / 2; // 计算距离

    return dis; // 返回距离
}

// 定义前方测距函数
float front_detection()
{
    float dis_f; // 定义距离变量
    PWM_write(myservo1,90); // 舵机转动到90度，即正前方
    delay(500); // 延时500毫秒，等待舵机转动穩定
    dis_f = disMeasure(); // 调用测距函数，测量前方距离
    return dis_f; // 返回距离
}

// 定义左方测距函数
float left_detection()
{
    float dis_l;
    PWM_write(myservo1,175); // 舵机转动到175度，即左方
    delay(500); // 延时500毫秒，等待舵机转动穩定
    dis_l = disMeasure();
    return dis_l;
}

float right_detection()
{
    float dis_r;
    PWM_write(myservo1,5); // 舵机转动到5度，即右方
    delay(500);
    dis_r = disMeasure();
    return dis_r;
}

int main(int argc,char *argv[])
{
    float dis1,dis2,dis3; // 定义前方、左侧、右侧距离
    int time = 1;

    wiringPiSetup();


    // 为蜂鸣器创建一个软件音调控制，如果失败则输出错误信息并返回1
    if(softToneCreate(BuzPin)==-1){
        cout<<"Setup softTone failed!"<<endl;
        return 1;
    }
    // 设置蜂鸣器的初始频率为300Hz
    softToneWrite(BuzPin, 300);


    // 设置GPIO接口为输出模式
    pinMode(1,OUTPUT);
    pinMode(2,OUTPUT);
    pinMode(3,OUTPUT);
    pinMode(4,OUTPUT);
    pinMode(5,OUTPUT);
    pinMode(6,OUTPUT);
    ultraInit(); // 初始化超声波模块

    // 创建两个软件控制的PWM信号，用于控制电机转速
    softPwmCreate(PWMA, 0, 100); // 设置电机A的PWM范围为0到100
    softPwmCreate(PWMB, 0, 100); // 设置电机B的PWM范围为0到100

    // 初始化PCA9685，设置I2C地址为0x40，PWM频率为50Hz
    int fd = pca9685Setup(PIN_BASE,0x40,HERTZ);
    if(fd < 0) // 如果初始化失败，输出错误信息
    {
        printf("Error in setup\n");
        return fd;
    }

    pca9685PWMReset(fd); // 重置PCA9685
    printf("OK"); // 输出OK表示初始化成功

    while(1){
        dis1 = front_detection(); //檢測前方距離
        if(dis1 < 40){ // 如果前方距离小于40厘米
            t_stop(200); // 停止200毫秒
            for(int i=0;i<sizeof(song_1)/sizeof(int);i++){
                softToneWrite(BuzPin,song_1[i]); // 设置蜂鸣器播放的频率为song_1[i]
                //delay(beat_1[i]*500); // 根据beat_1[i]的值来决定音符的持续时间
                delay(10* dis1);//鸣笛时间取决于距离
            }
            t_down(50,500); // 后退，速度50，时间500ms
            t_stop(200); // 再次停止200毫秒
            dis2 = left_detection(); // 检测左侧距离
            dis3 = right_detection(); // 检测右侧距离
            if(dis2 < 40 && dis3 < 40){ // 如果左右两侧距离都小于40厘米
                t_left(50,1000); // 左转，速度50，时间1000ms
            } else if(dis2 > dis3 ){ // 如果左侧距离大于右侧距离
                t_left(50,300); // 左转，速度50，时间300ms
                t_stop(100); // 停止100毫秒
            } else{ // 如果右侧距离大于左侧距离
                t_right(50,300); // 右转，速度50，时间300ms
                t_stop(100);
            }
        } else{ // 如果前方距离大于40厘米
            t_up(60,0); // 前进，速度60
        }
    }
    return 0;
}

// 舵机控制函数，servonum为PWM輸出通道（舵机编号），x为舵机转动角度
void PWM_write(int servonum,float x)
{
    float y;
    int tick;
    y=x/90.0+0.5; // 将角度转化为占空比（PWM信號的高電平時間y）
    //防止高電平時間y超出0.5ms~2.5ms的範圍
    y= max(y,0.5); // 保证占空比y的最小值不小于0.5，防止舵机工作在极限角度，导致舵机损坏
    y= min(y,2.5); // 保证占空比y的最大值不大于2.5，防止舵机工作在极限角度，导致舵机损坏
    tick = calcTicks(y, HERTZ); // 将占空比转化为PCA9685的寄存器值，即脉冲宽度（将占空比y和频率HERTZ转换为脉冲宽度tick）
    pwmWrite(PIN_BASE+servonum,tick); // 设置舵机的脉冲宽度（将计算得到的脉冲宽度tick写入到舵机对应的PWM信号中）
}

// 计算脉冲宽度，impulseMs为占空比（脈衝寬度（高電平時間）），hertz为频率50Hz
int calcTicks(float impulseMs, int hertz)
{
    float cycleMs=1000.0f/(float)hertz; // 计算一个PWM周期的时间，单位是ms
    return (int)(MAX_PWM*impulseMs/cycleMs+0.5f); // 将脉冲宽度转换为脉冲数，这是一个线性转换公式，用于计算给定脉冲宽度和频率下的脉冲数
}



//1、超声波传感器主要由Trig（触发）和Echo（回声）两个引脚组成，用于发射和接收超声波。

//2、PCA9685是一款16通道12位PWM I2C总线LED控制器，用于生成PWM信号以控制舵机。

//3、舵机是一种可以精确控制转动角度的电机，这里主要用于控制运动方向。

//4、PWM (Pulse Width Modulation) 脉冲宽度调制，是一种可以用于控制电机速度和LED亮度的技术。





















