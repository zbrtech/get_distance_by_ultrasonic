#include <AT89x51.H>		//���������ļ�
#include <intrins.h>

#define TRIG P0_1	//�ֱ������������ںͽ��ܿ����ñ�����
#define ECHO P0_2	//����11 12���ſ������ʹ�ã�ֻҪ��P1~P3���IO�ھ���

//�����Ƕ�1602��ʾƵ���ŵĶ���
//�����Ϊ�̶�����Ϊ���ǽӵľ������һ���Ǹ��ӿڣ�
//����������Ҫ�鿴һ�¿������ԭ��ͼ
#define LCM_EN P0_5  //��ʾ����ʹ�ܶ�
#define LCM_RS P0_7	 //�Ĵ���ѡ��
#define LCM_RW P0_6  //��дѡ����
#define LCM_DATA P2	 //����ѡ���ߣ�����P20-P28������ѡ���
#define key1 P3_5	//ѡ���Ԥ������
#define key2 P3_4	//key2 key3 �ֱ��Ǽ��ͼӲ���
#define key3 P3_3	//�����ж�(���ѡ���˲����жϣ���Ϊ���ж����治��ʹ����ʾ�������Ĺ���)
#define key4 P3_2	//�������ȷ��

#define BEEP P3_6	//��������������
#define BUSY 0x80   //�������LCM�Ƿ���busy��ʾ����ȡBF�ֶε�ֵ
typedef bit BOOL;


//����һЩ����
static unsigned char DisNum = 0; //��ʾ��ָ��				  
       unsigned int  time=0;
	   unsigned long S =0;
	   bit      flag =0;
	   unsigned char disbuff[4]	   ={ 0,0,0,0,};

unsigned char code first_line[] = {"disstance is:"};
unsigned char code wrong_info[] = {"over range"};
unsigned char code change_range[] = {"safe range:"};
unsigned char code clr[] = {"                "};   //16���ո�
unsigned char code num_char[] = {'0','1','2','3','4','5','6','7','8','9','.','-','M'};

int Num = 10;		  			//����ı�������
unsigned char key1num = 0;		  //key1��������
unsigned char instant_num = 0;    //��˸��λ�õ�

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

//���궨λ
void write_by_xy(unsigned char x, unsigned char y)
{
	unsigned char address;
	if(y==0) 
	address=0x80+x;//y=0,д��һ��
	else 
	address=0xc0+x;//y=1,д�ڶ���
	write_command(address, 1); 
}


//��ʼ��
void LCM_init(void)
{
	LCM_DATA = 0;
	LCM_EN = 0;           //0x38  00111000  ����16*2��ʾ  5*7����  8λ���ݽӿ�  
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


//��ʾһ�����֣������λ����Ȼ��ʾ���£�
//����Ҫע��display_one_char��������Ĳ�����Ӧ����һ��char����
void dispaly_Num(unsigned char x, unsigned char y, int num)
{
	int gewei;
	int shiwei;
	gewei = num % 10;
	shiwei = num / 10;
	display_one_char(x, y, num_char[shiwei]);
	display_one_char(x + 1, y, num_char[gewei]);
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


//�������
void count_distance(void)
{
	unsigned char j = 0;
	int k = 200;
	time = TH0 * 256 + TL0;
	TH0 = 0;   //���¹��㣬���¼���
	TL0 = 0;

	S = (time * 1.7) / 100;  //�������������cm
	if ((S >= 700) || flag == 1)
	{
		flag = 0;
		//��ʾ������Ϣ
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
				for(k=S * 100; k > 0; k--){	//�Ƿ������������������й�
					delay500();
				}
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
	if(key1 == 0){  //��ʾ����1�Ѿ�������
		delay500();
		if(key1== 0){ //Ϊ����������������Ӱ�죬����ͬ��
			while(!key1);
			key1num++;
			TR0 = 0;    //�رռ�ʱ
		}

		//�������һЩ�򵥵���ʾ����
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
	while(key1num != 0){   //��ʾkey1���ٱ�����1��
		if(key2 == 0){
			delay500();
			if (key2 == 0)
			{
				while(!key2);   //key2�����£����ּ�һ
				if(Num > 8){
					Num--;
					dispaly_Num(instant_num, 0, Num);
					write_by_xy(instant_num + 1, 0);					
				}else{
					Num = 15;   //���ֲ���С��8
					dispaly_Num(instant_num, 0, Num);
					write_by_xy(instant_num + 1, 0);
				}
			}
		}
		if(key3 == 0){
			delay500();
			if(key3 == 0)    //key3�����£����ּ�һ
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
				while(!key4);//ȡ����˸�� ��key1num = 0
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

				TR0 = 1;  //���¿�ʼ��ʱ��������һ���ǲ�׼��������
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
			key_scan();
		}
	}

	return 0;
}
