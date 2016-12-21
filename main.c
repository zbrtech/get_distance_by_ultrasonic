#include <AT89x51.H>		//器件配置文件
#include <intrins.h>

#define TRIG P0_1	//分别给超声波发射口和接受口设置别名，
#define ECHO P0_2	//这里11 12引脚可以随便使用，只要是P1~P3间的IO口就行

//下面是对1602显示频引脚的定义
//这里较为固定，因为我们接的就是最后一排那个接口，
//所以这里需要查看一下开发板的原理图
#define LCM_EN P0_5  //显示器的使能端
#define LCM_RS P0_7	 //寄存器选择
#define LCM_RW P0_6  //读写选择线
#define LCM_DATA P2	 //数据选择线，共有P20-P28个数据选择端
#define key1 P3_5	//选择该预警距离
#define key2 P3_4	//key2 key3 分别是减和加操作
#define key3 P3_3	//用来中断(最后选择了不用中断，因为在中断里面不能使用显示屏操作的功能)
#define key4 P3_2	//这个用来确定

#define BEEP P3_6	//蜂鸣器控制引脚
#define BUSY 0x80   //用来检测LCM是否是busy表示，读取BF字段的值
typedef bit BOOL;


//定义一些常量
static unsigned char DisNum = 0; //显示用指针				  
       unsigned int  time=0;
	   unsigned long S =0;
	   bit      flag =0;
	   unsigned char disbuff[4]	   ={ 0,0,0,0,};

unsigned char code first_line[] = {"disstance is:"};
unsigned char code wrong_info[] = {"over range"};
unsigned char code change_range[] = {"safe range:"};
unsigned char code clr[] = {"                "};   //16个空格
unsigned char code num_char[] = {'0','1','2','3','4','5','6','7','8','9','.','-','M'};

int Num = 10;		  			//最初的报警距离
unsigned char key1num = 0;		  //key1按键次数
unsigned char instant_num = 0;    //闪烁定位用的

/*
1602基本的引脚功能：
第1脚：VSS为电源地
第2脚：VCC接5V电源正极
第4脚：RS为寄存器选择，高电平1时选择数据寄存器、低电平0时选择指令寄存器。
第5脚：RW为读写信号线，高电平(1)时进行读操作，低电平(0)时进行写操作。
第6脚：E(或EN)端为使能(enable)端,高电平（1）时读取信息
第7～14脚：D0～D7为8位双向数据端。
*/

//因为单片机执行是有时间延迟的，所以很多时候都要加一个延时的设置
//延迟时间和晶振频率有关，（具体不知道）ps至少延时大于100个执行时间
void delay_ms(int time)
{
	unsigned int time1 = time;
	unsigned int time2 = 100;
	while(time1) time1--;
	while(time2--);
}

void delay500(void)
{
	unsigned char i;
	for (i = 230; i > 0; --i);
}

BOOL get_busy_status(void)
{
	BOOL result;
	LCM_DATA = 0xff;
	LCM_RS = 0;     //选择指令寄存器
	LCM_RW = 1;    //读操作
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
	result = LCM_DATA & BUSY; //检测忙信号,如果忙碌就一直等待
	return result;
}


//写指令
void write_command(unsigned char WCLCM, busy)//当busy为0的时候不检查是否忙碌
{
	while(get_busy_status());
	LCM_DATA = WCLCM;
	LCM_RS = 0;    //选择指令寄存器
	LCM_RW = 0;    //选择写操作，这里是写操作字
	LCM_EN = 0;		//在这里起一个延时的作用
	delay_ms(50);   //作用和上面其实是一样的
	LCM_EN = 1;    //执行指令
}

//写数据
void write_data(unsigned char WRLCM)
{
	while(get_busy_status());
	LCM_DATA = WRLCM;
	LCM_RS = 1;  //因为是写操作，所以要选择数据寄存器
	LCM_RW = 0;  //写操作
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
}

//坐标定位
void write_by_xy(unsigned char x, unsigned char y)
{
	unsigned char address;
	if(y==0) 
	address=0x80+x;//y=0,写第一行
	else 
	address=0xc0+x;//y=1,写第二行
	write_command(address, 1); 
}


//初始化
void LCM_init(void)
{
	LCM_DATA = 0;
	LCM_EN = 0;           //0x38  00111000  设置16*2显示  5*7点阵  8位数据接口  
	write_command(0x38, 0);
	delay_ms(50);

	write_command(0x38, 1);
	delay_ms(50);
	write_command(0x0f, 1); //设置显示光标
	write_command(0x01, 1); //清屏一波
	write_command(0x06, 1); //地址指针自动+1且光标+1 
	write_command(0x80, 1); //设置初始显示位置，这一步好像可以不要，
							//后面肯定还要设置地址。
}


void display_one_char(unsigned char x, unsigned char y, unsigned char data_)
{
	y &= 0x01;
	x &= 0x0f;		//限制x不能大于15
	if (y) x |= 0x40; //要显示第二行的时候
	x |= 0x80;
	write_command(x, 1);
	write_data(data_);
}


//显示一个数字，最多两位，不然显示不下，
//这里要注意display_one_char这个函数的参数，应该是一个char类型
void dispaly_Num(unsigned char x, unsigned char y, int num)
{
	int gewei;
	int shiwei;
	gewei = num % 10;
	shiwei = num / 10;
	display_one_char(x, y, num_char[shiwei]);
	display_one_char(x + 1, y, num_char[gewei]);
}


void interrupt_func() interrupt 1 //T0中断用来计数器溢出，超出测距范围
{
	flag = 1;         //中断溢出标志
}


void strat_ultrasonic(void)
{
	TRIG = 1; //启动超声波模块
	_nop_();	//延时至少10ums以上
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	TRIG = 0;  //关闭该模块
}


//计算距离
void count_distance(void)
{
	unsigned char j = 0;
	int k = 200;
	time = TH0 * 256 + TL0;
	TH0 = 0;   //重新归零，重新计数
	TL0 = 0;

	S = (time * 1.7) / 100;  //这里算出来的是cm
	if ((S >= 700) || flag == 1)
	{
		flag = 0;
		//显示错误信息
		while(wrong_info[j] != '\0'){	         
			display_one_char(j, 1, wrong_info[j]);
			j++;			
		}
	}
	else
	{
		if (S < Num){
			P1 = 0x00;
			delay_ms(5000);
			P1 = 0xff;
			if (S < 5){
				while(k--){
					BEEP = ~BEEP;
					delay_ms(1);
				}
			}
			else{
				for(j = 200; j > 0; --j){
					BEEP = ~BEEP;
					delay500();
					delay500();
				}
				for(k=S * 100; k > 0; k--){	//是蜂鸣器鸣警间隔与距离有关
					delay500();
				}
			}	
		}
		disbuff[0]=S%1000/100;
	  	disbuff[1]=S%1000%100/10;
	  	disbuff[2]=S%1000%10 %10;
	  	display_one_char(0, 1, num_char[disbuff[0]]);
	  	display_one_char(1, 1, num_char[10]);	//显示点
	  	display_one_char(2, 1, num_char[disbuff[1]]);
	  	display_one_char(3, 1, num_char[disbuff[2]]);
	  	display_one_char(4, 1, num_char[12]);	//显示M
		j = 5;
	  	while(clr[j] != '\0'){
			display_one_char(j, 1, clr[j]);
			j++;			
		}
	}


}



void key_scan()
{
	unsigned char j = 0;
	unsigned char k;
	if(key1 == 0){  //表示按键1已经被按下
		delay500();
		if(key1== 0){ //为了消除抖动带来的影响，下面同理
			while(!key1);
			key1num++;
			TR0 = 0;    //关闭计时
		}

		//下面就是一些简单的显示操作
		while(change_range[j] != '\0'){
			display_one_char(j, 0, change_range[j]);
			j++;
		}
		k = j;
		instant_num = j;
		while(clr[j] != '\0'){
			display_one_char(j, 0, clr[j]);
			j++;
		}
		dispaly_Num(k, 0, Num);
		write_by_xy(k + 1, 0);
		//DisplayListChar(k, 0, Num);
		
	}
	while(key1num != 0){   //表示key1至少被按了1次
		if(key2 == 0){
			delay500();
			if (key2 == 0)
			{
				while(!key2);   //key2被按下，数字减一
				if(Num > 8){
					Num--;
					dispaly_Num(instant_num, 0, Num);
					write_by_xy(instant_num + 1, 0);					
				}else{
					Num = 15;   //数字不能小于8
					dispaly_Num(instant_num, 0, Num);
					write_by_xy(instant_num + 1, 0);
				}
			}
		}
		if(key3 == 0){
			delay500();
			if(key3 == 0)    //key3被按下，数字加一
			{
				while(!key3);
				if(Num < 15){
					Num++;
					dispaly_Num(instant_num, 0, Num);
					write_by_xy(instant_num + 1, 0);
				}else{
					Num = 8;
					dispaly_Num(instant_num, 0, Num);
					write_by_xy(instant_num + 1, 0);
				}
			}
		}
		if(key4 == 0){
			delay500();
			if(key4 == 0)
			{
				while(!key4);//取消闪烁， 令key1num = 0
				key1num = 0;
				j = 0; 
				while(clr[j] != '\0'){
					display_one_char(j, 0, clr[j]);
					j++;
				}
				j = 0;
				while(first_line[j] != '\0'){
					display_one_char(j, 0, first_line[j]);
					j++;
				}

				TR0 = 1;  //重新开始计时，不过这一次是不准的数据了
			}
		}
	}
}


int main()
{	unsigned char i = 0;
	delay_ms(400);
	LCM_init();
	delay_ms(400);
	while(first_line[i] != '\0'){
		display_one_char(i, 0, first_line[i]);
		i++;
	}

	while(1)
	{
		TMOD = 0x01;      //设T0为方式1， GATE=1
		TH0 = 0;
		TL0 = 0;
		ET0 = 1;         //允许T0中断
		EA = 1;			 //开启总中断，这一步是必须的，因为总共有两个计时器

		while(1){
			strat_ultrasonic();
			while(!ECHO);   //等待
			TR0 = 1;       //开启计数
			while(ECHO);	//计时并且等待
			TR0 = 0;
			count_distance();
			key_scan();
		}
	}

	return 0;
}
