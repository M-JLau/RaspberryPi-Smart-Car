
#include <wiringPi.h>
#include <iostream>
#include "pca9685.h"

using namespace std;

#define PIN_BASE 300 // 你的PCA9685的基础pin号，需要与你的设置相符
#define HERTZ 50 // PWM频率
#define MAX_CHANNELS 16 // PCA9685支持的最大通道数量

int main() {
    wiringPiSetup();
    // 初始化PCA9685
    int fd = pca9685Setup(PIN_BASE, 0x40, HERTZ);
    if (fd < 0) {
        printf("Error in setup\n");
        return -1;
    }
    
    pca9685PWMReset(fd); // 重置PCA9685
    
    // 逐一启动并停止每一个通道，每次只移动一个通道，并给予足够的时间来观察哪个舵机在动
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        int pin = PIN_BASE + i;
        pwmWrite(pin, 500); // 启动通道
        cout << "Moving channel " << i << endl;
        delay(5000); // 等待足够长的时间，让你能够观察到哪个舵机在动
        pwmWrite(pin, 0); // 停止通道
        delay(1000); // 在启动下一个通道之前等待一段时间
    }
    
    return 0;
}
