//LED闪烁程序，通过GPIO控制LED灯的亮灭状态。

#include <iostream>
#include <wiringPi.h>

using namespace std;

#define Gpin 21   // 定义一个宏，Gpin对应的值为21，即綠色LED灯连接的GPIO接口号
#define Rpin 22   // 定义一个宏，Rpin对应的值为22，即紅色LED灯连接的GPIO接口号

// 定义一个函数用于初始化LED灯，设置Gpin和Rpin为输出模式
void led_init(void){
    pinMode(Gpin, OUTPUT);   // 设置GPIO端口Gpin（即GPIO 21）为输出模式
    pinMode(Rpin, OUTPUT);   // 设置GPIO端口Rpin（即GPIO 22）为输出模式
}

// 定义一个函数用于打开LED灯，即输出高电平
void led_on(void){
    digitalWrite(Gpin, HIGH);  // 设置GPIO端口Gpin（即GPIO 21）输出高电平，使得第一个LED灯亮起
    digitalWrite(Rpin, HIGH);  // 设置GPIO端口Rpin（即GPIO 22）输出高电平，使得第二个LED灯亮起
}

// 定义一个函数用于关闭LED灯，即输出低电平
void led_off(void){
    digitalWrite(Gpin, LOW);   // 设置GPIO端口Gpin（即GPIO 21）输出低电平，使得第一个LED灯熄灭
    digitalWrite(Rpin, LOW);   // 设置GPIO端口Rpin（即GPIO 22）输出低电平，使得第二个LED灯熄灭
}

// 主函数
int main(int argc, char *argv[])
{
    wiringPiSetup();  // 初始化wiringPi库，为后续的GPIO操作做准备
    led_init();       // 调用led_init函数，对LED灯进行初始化

    // 循环控制LED灯的亮灭状态
    while(1)   // 创建一个无限循环，使得LED灯可以持续闪烁
    {
        led_on();   // 调用led_on函数，打开LED灯
        delay(500);  // 延时500毫秒（即0.5秒），使得LED灯保持亮起状态

        led_off();  // 调用led_off函数，关闭LED灯
        delay(500);  // 延时500毫秒（即0.5秒），使得LED灯保持熄灭状态
    }

    return 0;  // 程序正常结束，返回0
}




//这个程序使用的是wiringPi库来操作树莓派的GPIO接口，主要用于控制两个LED灯的亮灭。


//1、引入所需的头文件：iostream用于输入输出操作，wiringPi.h用于操作树莓派的GPIO。

//2、定义LED灯连接的GPIO接口号：在这里，Gpin对应GPIO 21，Rpin对应GPIO 22。

//3、实现LED初始化函数led_init：这个函数设置Gpin和Rpin为输出模式，以便我们可以通过它们控制LED。

//4、实现LED打开函数led_on：这个函数使Gpin和Rpin接口输出高电平，使连接的LED灯亮起。

//5、实现LED关闭函数led_off：这个函数使Gpin和Rpin接口输出低电平，使连接的LED灯熄灭。

//6、main函数：首先调用wiringPiSetup初始化wiringPi库，然后调用led_init初始化LED，最后在一个无限循环中，使LED交替亮灭，亮和灭的时间各为500毫秒。
