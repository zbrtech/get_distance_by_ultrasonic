#include <AT89x51.H>		//���������ļ�
#include <intrins.h>

#define TRIG P3_3	//�ֱ������������ںͽ��ܿ����ñ�����
#define ECHO P3_4	//����11 12���ſ������ʹ�ã�ֻҪ��P1~P3���IO�ھ���

//�����Ƕ�1602��ʾƵ���ŵĶ���
//�����Ϊ�̶�����Ϊ���ǽӵľ������һ���Ǹ��ӿڣ�
//����������Ҫ�鿴һ�¿������ԭ��ͼ
#define LCM_EN P0_5  //��ʾ����ʹ�ܶ�
#define LCM_RS P0_7	 //�Ĵ���ѡ��
#define LCM_RW P0_6  //��дѡ����
#define LCM_DATA P2	 //����ѡ���ߣ�����P20-P28������ѡ���
#define BEEP P3_6
#define BUSY 0x80   //�������LCM�Ƿ���busy��ʾ����ȡBF�ֶε�ֵ
typedef bit BOOL;


//����һЩ����
static unsigned char DisNum = 0; //��ʾ��ָ��				  
       unsigned int  time=0;
	   unsigned long S =0;
	   bit      flag =0;
	   unsigned char disbuff[4]	   ={ 0,0,0,0,};

unsigned char code first_line[] = {"disstance is:"};
unsigned char code wrong_info[] = {"over the longest distance"};
unsigned char code num_char[] = {'0','1','2','3','4','5','6','7','8','9','.','-','M'};



/*
1602���������Ź��ܣ�
��1�ţ�VSSΪ��Դ��
��2�ţ�VCC��5V��Դ����
��4�ţ�RSΪ�Ĵ���ѡ�񣬸ߵ�ƽ1ʱѡ�����ݼĴ������͵�ƽ0ʱѡ��ָ��Ĵ�����
��5�ţ�RWΪ��д�ź��ߣ��ߵ�ƽ(1)ʱ���ж��������͵�ƽ(0)ʱ����д������
��6�ţ�E(��EN)��Ϊʹ��(enable)��,�ߵ�ƽ��1��ʱ��ȡ��Ϣ
��7��14�ţ�D0��D7Ϊ8λ˫�����ݶˡ�
*/

//��Ϊ��Ƭ��ִ������ʱ���ӳٵģ����Ժܶ�ʱ��Ҫ��һ����ʱ������
//�ӳ�ʱ��;���Ƶ���йأ������岻֪����ps������ʱ����100��ִ��ʱ��
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
	LCM_RS = 0;     //ѡ��ָ��Ĵ���
	LCM_RW = 1;    //������
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
	result = LCM_DATA & BUSY; //���æ�ź�,���æµ��һֱ�ȴ�
	return result;
}


//дָ��
void write_command(unsigned char WCLCM, busy)//��busyΪ0��ʱ�򲻼���Ƿ�æµ
{
	while(get_busy_status());
	LCM_DATA = WCLCM;
	LCM_RS = 0;    //ѡ��ָ��Ĵ���
	LCM_RW = 0;    //ѡ��д������������д������
	LCM_EN = 0;		//��������һ����ʱ������
	delay_ms(50);   //���ú�������ʵ��һ����
	LCM_EN = 1;    //ִ��ָ��
}

//д����
void write_data(unsigned char WRLCM)
{
	while(get_busy_status());
	LCM_DATA = WRLCM;
	LCM_RS = 1;  //��Ϊ��д����������Ҫѡ�����ݼĴ���
	LCM_RW = 0;  //д����
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
}


//������
unsigned char read_data(void)
{
	LCM_RS = 1;
	LCM_RW = 1;  //������
	LCM_EN = 0;
	delay_ms(50);
	LCM_EN = 1;
	return LCM_DATA;
}

//��ʼ��
void LCM_init(void)
{
	LCM_DATA = 0;
	LCM_EN = 0;
	/*0x38  00111000  ����16*2��ʾ  5*7����  8λ���ݽӿ�*/  
	write_command(0x38, 0);
	delay_ms(50);

	write_command(0x38, 1);
	delay_ms(50);
	write_command(0x0f, 1); //������ʾ���
	write_command(0x01, 1); //����һ��
	write_command(0x06, 1); //��ַָ���Զ�+1�ҹ��+1 
	write_command(0x80, 1); //���ó�ʼ��ʾλ�ã���һ��������Բ�Ҫ��
							//����϶���Ҫ���õ�ַ��
}


void display_one_char(unsigned char x, unsigned char y, unsigned char data_)
{
	y &= 0x01;
	x &= 0x0f;		//����x���ܴ���15
	if (y) x |= 0x40; //Ҫ��ʾ�ڶ��е�ʱ��
	x |= 0x80;
	write_command(x, 1);
	write_data(data_);
}


void interrupt_func() interrupt 1 //T0�ж����������������������෶Χ
{
	flag = 1;         //�ж������־
}

void strat_ultrasonic(void)
{
	TRIG = 1; //����������ģ��
	_nop_();	//��ʱ����10ums����
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
	TRIG = 0;  //�رո�ģ��
}

void count_distance(void)
{	unsigned char j;
	time = TH0 * 256 + TL0;
	TH0 = 0;   //���¹��㣬���¼���
	TL0 = 0;

	S = (time * 1.7) / 100;  //�������������cm
	if ((S >= 700) || flag == 1)
	{
		flag = 0;

		display_one_char(0, 1, 'F');
	}
	else
	{
		if (S < 10){
			P1 = 0x00;
			delay_ms(50000);
			P1 = 0xff;
			delay_ms(50000);
			for(j = 200; j > 0; --j){
				BEEP = ~BEEP;
				delay500();
			}	
		}
		disbuff[0]=S%1000/100;
	  	disbuff[1]=S%1000%100/10;
	  	disbuff[2]=S%1000%10 %10;
	  	display_one_char(0, 1, num_char[disbuff[0]]);
	  	display_one_char(1, 1, num_char[10]);	//��ʾ��
	  	display_one_char(2, 1, num_char[disbuff[1]]);
	  	display_one_char(3, 1, num_char[disbuff[2]]);
	  	display_one_char(4, 1, num_char[12]);	//��ʾM
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
	display_one_char(0, 1, 'T');
//	display_one_char(0, 0, 'c');
	while(1)
	{
		TMOD = 0x01;      //��T0Ϊ��ʽ1�� GATE=1
		TH0 = 0;
		TL0 = 0;
		ET0 = 1;         //����T0�ж�
		EA = 1;			 //�������жϣ���һ���Ǳ���ģ���Ϊ�ܹ���������ʱ��

		while(1){
			strat_ultrasonic();
			while(!ECHO);   //�ȴ�
			TR0 = 1;       //��������
			while(ECHO);	//��ʱ���ҵȴ�
			TR0 = 0;
			count_distance();
			delay_ms(80000);   //��ʱ
		}
	}

	//while(1);
	return 0;
}

