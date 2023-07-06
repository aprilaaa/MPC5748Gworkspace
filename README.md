# MPC5748Gworkspace

![image](https://github.com/aprilaaa/MPC5748Gworkspace/assets/68526860/42730b48-1bcb-41d4-85f7-1f79ca540984)


## 1. testgpio_1_Z4_0

  这是第一次使用MPC5748G时编写的代码，该程序可以使用两个SW按键控制8个LED灯和1个gpio口。
  
  This is the code written when using MPC5748G for the first time. The program can use two SW buttons to control 8bit LED lights and 1bit GPIO port
    ![image](https://github.com/aprilaaa/MPC5748Gworkspace/assets/68526860/6e315af8-9930-4244-b1ab-7b4bfcd5768d)
  
  如果想要看到详细过程，可以阅读这篇文章：
  
  If you want to see the detailed process, you can read this article:
  
  https://blog.csdn.net/guojingyue123/article/details/131283652?spm=1001.2014.3001.5501

## 2. testADC_Z4_0
<br>
   这个工程使用到了八个LED灯，一个电位器和一位ADC输入PB[4]，通过输入的模拟量来控制LED灯的亮灭。

    This project uses eight LED lights, one Potentiometer and one ADC input PB [4] to control the LED lights on and off through the input analog quantity.
    
    ![image](https://github.com/aprilaaa/MPC5748Gworkspace/assets/68526860/de116e4a-6ba5-4fed-890c-5a6c226e5b40)

  
    如果想要看到详细过程，可以阅读这篇文章：
  
    If you want to see the detailed process, you can read this article:

    https://blog.csdn.net/guojingyue123/article/details/131429335?spm=1001.2014.3001.5501

## 3. testSTM_Z4_0
<br>


这个工程使用了系统定时器模块STM，功能为设定一个1s的延时，控制LED的亮灭。

    This project uses the system timer module STM, which sets a 1-second delay to control the LED on and off.
    ![image](https://github.com/aprilaaa/MPC5748Gworkspace/assets/68526860/5df06232-2e8f-47c5-bdfa-07c3fe91456a)

    如果想要看到详细过程，可以阅读这篇文章：
  
    If you want to see the detailed process, you can read this article:
 
    https://blog.csdn.net/guojingyue123/article/details/131556027?spm=1001.2014.3001.5501

# 4. 待续
<br>
CAN总线和UART模块因为某些原因无法测试，PWM我试图驱动一个舵机但是没成功，Timer模块跟STM功能差不多，这四个部分参考这三篇文章：

  CAN：https://blog.csdn.net/weifengdq/article/details/103996238
  
  UART:https://blog.csdn.net/weifengdq/article/details/103981747
  
  Timer:https://blog.csdn.net/weifengdq/article/details/103964297
  
  PWM:https://blog.csdn.net/weifengdq/article/details/104012375
  
