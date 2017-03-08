#include "RN8209G_SPI.h"
#include "delay.h" 
#include "math.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
// 1 .�ߵ͵�ƽ�ĳ���ʱ��Ϊ1���룬Ƶ��Ϊ500hz,62.5�ֽ�ÿ��
// 1 .д��ַ�Ͷ������Ǽ��3���룬�ֲ�˵�Ǵ��ڰ��ʱ�����ھ���
// 1 .
// 1 .SCSN  ------B12
//    SCLK--------B13
// 1 .SDO --------B14
// 1 .SDI --------B15

//////////////////////////////////////////////////////////////////////////////////
u8 Vu;   //���ѹ����ʱ����ѹͨ���ĵ�ѹ=���ŵ�ѹ*�Ŵ���
u8 Vi;	
u8 Un=220;		//�����ĵ�ѹ   ��У��ʱ�޸�
float Ib=8;		//�����ĵ���   ��У��ʱ�޸�
unsigned int EC=3200;

float RMSIAreg;
 float RMSIBreg;
 float RMSUreg;//��Чֵ�Ĵ�������ʱ�������
 float PowerPAreg;

u32 RMSIAreg_adj;
u32 RMSIBreg_adj;
u32 RMSUreg_adj;//У����ʱ����Чֵ�Ĵ��������أ���ʱ�������
u32 PowerAreg_adj;

float TempU;
float TempIA;
float TempPowerPA;

float err_PowerA;
float err_Phase_A;
float err_reactive_A;
u8 angle_reactive_A;
float KiA;
float KiB;
float Ku;
float Kp;
extern u32 data_r[4];

Adjust_Parameter_TypeDef Adjust_Parameter_Structure;
 
//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ������SD Card/W25Q64/NRF24L01						  
//SPI�ڳ�ʼ��
//�������Ƕ�SPI2�ĳ�ʼ��
void RN8209G_SPI_config(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTBʱ��ʹ��  	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 |GPIO_Pin_13 | GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;  //PB13/14/15����������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4

	GPIO_SetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_15);  //PB13/14/15����
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);

}   




//***********************************************************************
//*///// ��������: u8 RN8209_ReadData(u8 add,u8 *data,u8 *len) 
//*///// ��������: ��ȡ8209�Ĵ������� 
//*///// �������: add-8209�Ĵ�����ַ *data-��ȡ����õĻ����� *len-���ݳ��� 
//*///// �������: 0-��ȡʧ�� 1-��ȡ�ɹ� 
//**************************2016.10.13���*********************************************/
//u32 RN8209_ReadData(u8 address,u32 *data)
u32 RN8209_ReadData(u8 address)
{
	u8 i;
	u32 data=0;
//	u8 return_s=1;
	u8 address_s=address;	
//	Adjust_Parameter_TypeDef    Adjust_Parameter_Structure;
//	u8 data8;
	
//	data=0;//�´�������ʱ�򣬾�������
	
	SCSN_L;
//	SCLK_L;
	SCLK_H;
  delay_ms(1);	   
	for(i=0;i<8;i++)/*���Ͳ�����͵�ַ*/  
   {
	   SCLK_H;//�����ڸߵ�ƽд�����ֽ�
     if((address_s&0x80)==0x80)  
	    {	
        SDI_H  
	    }
      else
	    {	
        SDI_L;
	    }
	   delay_ms(1);
     SCLK_L;/*���豸�½��ؽ�������*/  
	   delay_ms(1);
     address_s<<= 1; //-------�����Ѿ��ı��˱�����ֵ��������Ҫ���ã��Ͳ���---------------------------
   } 
	
   delay_ms(2);
	 SDI_L;     //--�����ǲ������յ�ʱ������---//
	 delay_ms(1);
	switch(address)
	{
		case 0x07:	 
		case 0x08:	
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43://-------------------------------------------�Ĵ�������Ϊ1-------------------------//
		SCLK_H;//�ӻ��ڸߵ�ƽ���������
		for(i=0;i<8;i++)
		 {
		   delay_ms(1);//�ȴ��ӻ��ź�����
		   SCLK_L;		//һ���½��غ������Ѿ�׼������
		   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)))//���⣺Ϊʲôֱ����SDO ������?
		    {
					data<<=1;
		    	data|=0x00000001;					
		    }
		   else
		    {
					data<<=1;
		    	data&=0xFFFFFFFE;				
		    }	
				delay_ms(1);
				SCLK_H;	
		 }
		SCLK_L;
		delay_ms(2);
		SCSN_H;
		return(data);
//		break;----------------����return�Ͳ��÷�����
			
		case 0x00:
		case 0x01:
		case 0x02:	
		case 0x03:	
		case 0x04:	
		case 0x05:
		case 0x06:
		case 0x09:	//�ϰ汾			
		case 0x0A:	
		case 0x0B:	
		case 0x0C:	//�ϰ汾
		case 0x0D:	//�ϰ汾
		case 0x0E:	
		case 0x0F:	
		case 0x10:	
		case 0x11:	//�°�����
		case 0x12:	//�°�����
		case 0x13:	//�°�����
		case 0x14:	//�°�����
		case 0x15:	//�°�����
		case 0x16:  //�°�����
		case 0x17:	//�°�����
		case 0x20: 
		case 0x21:
		case 0x25:	
		case 0x45://----------------------------------------�Ĵ�������Ϊ2-------------------------------//
		SCLK_H;//�ӻ��ڸߵ�ƽ���������
		for(i=0;i<16;i++)
		 {
		   delay_ms(1);//�ȴ��ӻ��ź�����
		   SCLK_L;		//һ���½��غ������Ѿ�׼������
		   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)))//���⣺Ϊʲôֱ����SDO ������?
		    {
					data<<=1;
		    	data|=0x00000001;
					
		    }
		   else
		    {
					data<<=1;
		    	data&=0xFFFFFFFE;				
		    }	
				delay_ms(1);
				SCLK_H;	
		 }
		SCLK_L;
		delay_ms(2);
		SCSN_H;
		return(data);
//		break;
		
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x29:	
		case 0x2A:	
		case 0x2B:
		case 0x2C:	
		case 0x2D:
		case 0x30:
		case 0x31:	
		case 0x32:  //�°�����	
		case 0x35:
    case 0x7f://-----------------------------------------����Ϊ3����-------------------------------//
		SCLK_H;//�ӻ��ڸߵ�ƽ���������
		for(i=0;i<24;i++)
		 {
			 
			 
		   delay_ms(1);//�ȴ��ӻ��ź�����
		   SCLK_L;		//һ���½��غ������Ѿ�׼������
		   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)))//���⣺Ϊʲôֱ����SDO ������?
		    {
					data<<=1;
		    	data|=0x00000001;
					
		    }
		   else
		    {
					data<<=1;
		    	data&=0xFFFFFFFE;				
		    }	
				delay_ms(1);
				SCLK_H;		
		 }
//		delay_ms(1);
//		SCLK_L;
		delay_ms(2);
		SCSN_H;
		return(data);
//		break;	
		
		case 0x26 :	
		case 0x27 :	 
		case 0x28 :	 //�ϰ汾
		case 0x44 ://---------------------------------------����Ϊ4���ֽ�--------------------------//
		SCLK_H;//�ӻ��ڸߵ�ƽ���������
		for(i=0;i<32;i++)
		 {
		 	 delay_ms(1);//�ȴ��ӻ��ź�����
		   SCLK_L;		//һ���½��غ������Ѿ�׼������
		   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)))//���⣺Ϊʲôֱ����SDO ������?
		    {
					data<<=1;
		    	data|=0x00000001;
					
		    }
		   else
		    {
					data<<=1;
		    	data&=0xFFFFFFFE;				
		    }	
				delay_ms(1);
				SCLK_H;	
		 }
		SCLK_L;
		delay_ms(2);
		SCSN_H;
		return(data);	
//		break;	
		default :
//		return_s = 0;	
		break;
	}
	 delay_ms(2);
	 SCSN_H;

//	return  data;
 
}



/***********************************************************************
//��������: void RN8209_WriteData(u8 *ptr)  
//��������: д�����ݵ�8209�Ĵ����� 
//�������: *ptr-ָ��Ҫд������ݻ�����  
//�������: ��  
//��ַ��8λ�����λ0��������1--д
***********************************************************************/
void RN8209_WriteData(u8 address,u32 order)    
{
	u8 i;
	u32 res;
//	u8 return_s=1;
	u8 address_s=address;
	if(address_s!=0xEA)//��ַ��8λ�����λ0��������1--д
	 {
	 	 address_s|=0x80;
	 }
   SCSN_H;
	 delay_ms(2);
	 SCSN_L;
	 SCLK_H;
   delay_ms(1);	
	 for(i=0;i<8;i++)/*���Ͳ�����͵�ַ*/  
    {
		  SCLK_H;//�����ڸߵ�ƽд�����ֽ�
      if((address_s&0x80)==0x80)  
		   {
         SDI_H  
		   }
      else
		   {
         SDI_L;
		   }
		   delay_ms(1);
       SCLK_L;/*���豸�½��ؽ�������*/  
		   delay_ms(1);
       address_s<<= 1; //-------�����Ѿ��ı��˱�����ֵ��������Ҫ���ã��Ͳ���---------------------------	
    } 
    delay_ms(2);//--�����ǲ������յ�ʱ������---//
	switch(address)
	{
		case 0xEA:  //-------------------------------------��������Ĵ���--------------------------------//
			
		case 0x07:	 
		case 0x08:	
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43://-------------------------------------------�Ĵ�������Ϊ1---------------------------//
		for(i=0;i<8;i++)/*���Ͳ�����͵�ַ*/  
		 {
		 	 SCLK_H;//�����ڸߵ�ƽд�����ֽ�
		 	 if((order&0x80)==0x80)  
		 	  {
		 	  	SDI_H  
		 	  }
		 	 else
		 	  {
		 	  	SDI_L;
		 	  }
		 	 delay_ms(1);
		 	 SCLK_L;/*���豸�½��ؽ�������*/  
		 	 delay_ms(1);
		 	 order<<= 1;
		 } 
		 delay_ms(100);
		 res=RN8209_ReadData(address);//�ص�����
		 printf("   %x \r\n",res); //��ʾID
		break;
		
		case 0x00:
		case 0x01:
		case 0x02:	
		case 0x03:	
		case 0x04:	
		case 0x05:
		case 0x06:
		case 0x09:	//�ϰ汾			
		case 0x0A:	
		case 0x0B:	
		case 0x0C:	//�ϰ汾
		case 0x0D:	//�ϰ汾
		case 0x0E:	
		case 0x0F:	
		case 0x10:	
		case 0x11:	//�°�����
		case 0x12:	//�°�����
		case 0x13:	//�°�����
		case 0x14:	//�°�����
		case 0x15:	//�°�����
		case 0x16:  //�°�����
		case 0x17:	//�°�����
		case 0x20: 
		case 0x21:
		case 0x25:	
		case 0x45://------------------------------------------�Ĵ�������Ϊ2---------------------------//
		for(i=0;i<16;i++)/*���Ͳ�����͵�ַ*/  
		 {
		 	 SCLK_H;//�����ڸߵ�ƽд�����ֽ�
		 	 if((order&0x8000)==0x8000)  //-------------------λ����ͬ���ֽڲ�ͬ�����λҲ��ͬ�����Ҫע��------------------------
		 	  {
		 	  	SDI_H  
		 	  }
		 	 else
		 	  {
		 	  	SDI_L;
		 	  }
		 	 delay_ms(1);
		 	 SCLK_L;/*���豸�½��ؽ�������*/  
		 	 delay_ms(1);
		 	 order<<= 1; 				
		 } 
		 delay_ms(100);
		 res=RN8209_ReadData(address);//�ص�����
		 printf("   %x \r\n",res); //��ʾID
		break;
			
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x29:	
		case 0x2A:	
		case 0x2B:
		case 0x2C:	
		case 0x2D:
		case 0x30:
		case 0x31:	
		case 0x32:  //�°�����	
		case 0x35:
    case 0x7f://------------------------------------------����Ϊ3����-----------------------------//
		for(i=0;i<24;i++)/*���Ͳ�����͵�ַ*/  
		 {
		 	 SCLK_H;//�����ڸߵ�ƽд�����ֽ�
		 	 if((order&0x800000)==0x800000)  
		 	  {
		 	  	SDI_H  
		 	  }
		 	 else
		 	  {
		 	  	SDI_L;
		 	  }
		 	 delay_ms(1);
		 	 SCLK_L;/*���豸�½��ؽ�������*/  
		 	 delay_ms(1);
		 	 order<<= 1; 			
		 } 
		 delay_ms(100);
		 res=RN8209_ReadData(address);//�ص�����
		 printf("   %x \r\n",res); //��ʾID
		break;	
		
		case 0x26 :	 
		case 0x27 :	 
		case 0x28 :	 //�ϰ汾
		case 0x44 ://----------------------------------����Ϊ4���ֽ�-------------------------//
		for(i=0;i<32;i++)/*���Ͳ�����͵�ַ*/  
		{
			SCLK_H;//�����ڸߵ�ƽд�����ֽ�
			if((order&0x80000000)==0x80000000)  
			 {
			 	 SDI_H  
			 }
			else
			 {
			 	 SDI_L;
			 }
			delay_ms(1);
			SCLK_L;/*���豸�½��ؽ�������*/  
			delay_ms(1);
			order<<= 1; 		
		} 	
		delay_ms(100);
		 res=RN8209_ReadData(address);//�ص�����
		 printf("   %x \r\n",res); //��ʾID
		break;	
		default :
		break;
	}
	SCLK_H;
	delay_ms(2);
	SCSN_H;
	delay_ms(2);
	
}

/*****************************************************************************
//��������:RN8209_Parameter_Adjust(void)  
//��������:У����һ���������������������á�
//�������:
//�������:
******************************************************************************/
void RN8209_Parameter_Adjust(void)
{
//	float temp_HFConst;
	
	Adjust_Parameter_TypeDef Adjust_Parameter_Structure;
	RN8209_WriteData(0xEA,0XE5);//дʹ��
	
	Adjust_Parameter_Structure.AdjustSYSCON=0x000f;//00H
	Adjust_Parameter_Structure.AdjustEMUCON=0x1403;//01H
//	Adjust_Parameter_Structure.AdjustHFConst=0X0B02;//�������PFCNT�Ĵ������Ƚϣ�5A
	Adjust_Parameter_Structure.AdjustHFConst=0X06E1;//�������PFCNT�Ĵ������Ƚ�,8A
//	Adjust_Parameter_Structure.AdjustHFConst=(int)(16.1079*Vu*Vi*(10^11)/(EC*Un*Ib));
	Adjust_Parameter_Structure.AdjustPStart=0x0060;//����Ĭ�ϵ�
	Adjust_Parameter_Structure.AdjustEMUCON2=0x0080;//���ʼ���Чֵ�����ٶ�Ϊ13.982HZ
	
	RN8209_WriteData(SYSCON,Adjust_Parameter_Structure.AdjustSYSCON);//Bͨ��ADCON���ã�ADC����ѡ��
	RN8209_WriteData(EMUCON,Adjust_Parameter_Structure.AdjustEMUCON);//�����ۼ�ģʽ����	
	RN8209_WriteData(HFConst,Adjust_Parameter_Structure.AdjustHFConst);//hfconst���ó�Ĭ�ϵ�1000	
	RN8209_WriteData(PStart,Adjust_Parameter_Structure.AdjustPStart);//�����������ó�Ĭ�ϵ�
	RN8209_WriteData(EMUCON2,Adjust_Parameter_Structure.AdjustEMUCON2);
}
/*****************************************************************************
//��������:RN8209_ActivePower_Adjust(void)  
//��������:У����2.1���������������й�У��
//�������:
//�������:
******************************************************************************/
void RN8209_ActivePower_Adjust(void)
{
//	float Pgain;
	
	
	RN8209_WriteData(0xEA,0XE5);//дʹ��
	
	if((-err_PowerA/(1+err_PowerA))>=0)                  //Aͨ����������У��
	{
		Adjust_Parameter_Structure.AdjustGPQA=(int)((-err_PowerA/(1+err_PowerA))*(2^15));
		RN8209_WriteData(GPQA,Adjust_Parameter_Structure.AdjustGPQA);
	}
	else
	{
		Adjust_Parameter_Structure.AdjustGPQA=(int)((2^16)+(-err_PowerA/(1+err_PowerA))*(2^15));
		RN8209_WriteData(GPQA,Adjust_Parameter_Structure.AdjustGPQA);
	}
	
	if((asin(-err_Phase_A/1.732))>=0)										 //Aͨ����λУ��
	{
		Adjust_Parameter_Structure.AdjustPhsA=(int)(((asin(-err_Phase_A/1.732))/0.02));
		RN8209_WriteData(PhsA,Adjust_Parameter_Structure.AdjustPhsA);
	}
	else
	{
		Adjust_Parameter_Structure.AdjustPhsA=(int)(((asin(-err_Phase_A/1.732))/0.02)+(2^8));
		RN8209_WriteData(PhsA,Adjust_Parameter_Structure.AdjustPhsA);                        
	}
	
	
}
/*****************************************************************************
//��������:RN8209_ActivePower_Adjust(void)  
//��������:У����2.2���������������޹�У��
//�������:
//�������:
******************************************************************************/
void RN8209_ReactivePower_Adjust(void)
{
	if((err_reactive_A*(tan(angle_reactive_A)))>=0) 
	{
		Adjust_Parameter_Structure.AdjustQPhsCal=(int)((err_reactive_A*(tan(angle_reactive_A)))*(2^15));
		RN8209_WriteData(QPhsCal,Adjust_Parameter_Structure.AdjustQPhsCal);
	}		
	else 
	{
		Adjust_Parameter_Structure.AdjustQPhsCal=(int)((err_reactive_A*(tan(angle_reactive_A)))*(2^15)+(2^16));
		RN8209_WriteData(QPhsCal,Adjust_Parameter_Structure.AdjustQPhsCal);
	}
}

/*****************************************************************************
//��������:RN8209_Rms_Adjust(void)  
//��������:У����3����������������ЧֵУ��
//�������:
//�������:
******************************************************************************/
void RN8209_Rms_Adjust(void)
{
	u32 temp_IARMS=0;
	u32 temp_IARMS_t=0;
//	u32 temp_IARMS_back=0;
//	u32 temp_IARMS_2=0;
//	
//	u32 temp_IARMS_3=0;
//	
//	u32 temp_IARMS_4=0;
	u8 i=0;
	u32 Iave=0;
	u16 temp_IARMSOS=0;

	for(i=0;i<10;i++)
	{

//		temp_IARMS+=RN8209_ReadData(IARMS,&temp_IARMS_back);//Ҳ����Aͨ��������Чֵ�Ĵ���0X22
		temp_IARMS_t=RN8209_ReadData(IARMS);//Ҳ����Aͨ��������Чֵ�Ĵ���0X22
		temp_IARMS+=temp_IARMS_t;
		delay_ms(5);
//		temp_IARMS_1=RN8209_ReadData(IARMS,&temp_IARMS);
//		delay_ms(5);
//		temp_IARMS_2=RN8209_ReadData(IARMS,&temp_IARMS);
//		delay_ms(5);
//		temp_IARMS_3=RN8209_ReadData(IARMS,&temp_IARMS);
//		delay_ms(5);
//		temp_IARMS_4=RN8209_ReadData(IARMS,&temp_IARMS);

	}
	temp_IARMS=(temp_IARMS/10);
	Iave=(temp_IARMS*temp_IARMS);//����˼�2���е����Ӵ
	Iave=~Iave;
	temp_IARMSOS=(u16 )(Iave>>8);
//	temp_IARMSOS=0xD80D ;`
	Adjust_Parameter_Structure.AdjustIARMSOS=temp_IARMSOS;
	RN8209_WriteData(IARMSOS,Adjust_Parameter_Structure.AdjustIARMSOS);
}

//*****************************************************************************
//��������: void RN8209_Count_Kx(void )  
//��������: �Ӽ���оƬ�ж�ȡ����,�������ת��ϵ��
//�������: ��
//�������: ��
//******************************************************************************/	
void RN8209_Count_Kx(void )  
{  
	RMSIAreg=RN8209_ReadData(IARMS);
	RMSIBreg=RN8209_ReadData(IBRMS);
	RMSUreg=RN8209_ReadData(URMS);
	if(RMSIAreg>0x800000)
	{
		RMSIAreg=0;
	}
	if(RMSIBreg>0x800000)
	{
		RMSIBreg=0;
	}
	if(RMSUreg>0x800000)//24λ�з����������λΪ1ʱ��0����
	{
		RMSUreg=0;
	}
//	KiA=(float)(((float)Ib)/((float)(RMSIAreg_adj-RMSIAreg)));
	KiA=(float)(0.00007233796);
	KiB=(float)(Ib/(RMSIBreg_adj-RMSIBreg));
	Ku=(float)(Un/(RMSUreg_adj-RMSUreg));
	Kp =(float)((3.22155*(10^12))/((2^32)*(Adjust_Parameter_Structure.AdjustHFConst)*EC));//����
}

//*****************************************************************************
//��������: void RN8209_KZ(void)  
//��������: ����ʱ�ļĴ�����ֵ
//�������: ��
//�������: ��
//******************************************************************************/	
void RN8209_KZ(void)  
{
  RMSIAreg_adj=RN8209_ReadData(IARMS);
	RMSIBreg_adj=RN8209_ReadData(IBRMS);
	RMSUreg_adj=RN8209_ReadData(URMS);//��ȡУ׼ʱ�����ص�ֵ
	PowerAreg_adj=RN8209_ReadData(PowerPA);
}

//*****************************************************************************
//��������: void RN8209_PowerEnergyCount(void )  
//��������: �Ӽ���оƬ��ȡ���ݣ���ѹ�����������ʡ���������������ѹ��������Ƶ�ʡ��������ؼ���
//�������: ��
//�������: ��
//******************************************************************************/	
void RN8209_PowerEnergyCount(void)
{
	RMSIAreg=RN8209_ReadData(IARMS);
	RMSIBreg=RN8209_ReadData(IBRMS);
	RMSUreg=RN8209_ReadData(URMS);
	PowerPAreg=RN8209_ReadData(PowerPA);
	
	RMSIAreg_adj=0x6c000;
	KiA=(float)(0.00007233796);
	if(RMSUreg_adj<RMSUreg)
	{
		TempIA=0;
	}
	else
	{
		TempIA=(float)(KiA*(RMSIAreg_adj-RMSIAreg));
	}
	TempU=(float)(Ku*(RMSUreg_adj-RMSUreg));
	
	TempPowerPA=(float)(Kp*(RMSIBreg_adj-RMSIBreg));
}

//*****************************************************************************
//��������: void RN8209_DC_Config(void )  
//��������: ����ֱ������
//�������: ��
//�������: ��
//******************************************************************************/	
void RN8209_DC_Config(void )
{
	Adjust_Parameter_TypeDef Adjust_Parameter_Structure;
	
	Adjust_Parameter_Structure.AdjustSYSCON=0x0000;//SYSCON �� BIT[1:0]�� BIT[3:2]�� BIT[5:4]д�� 0��������· ADC Ϊ 1 �����棻
	Adjust_Parameter_Structure.AdjustEMUCON=0x57E3;//EMUCON �н� IA/IB/U ��·�� ADC �ĸ�ͨʹ�ܹرգ� BIT[14]�� BIT[6:5]����Ϊ 1��
	
	RN8209_WriteData(SYSCON,Adjust_Parameter_Structure.AdjustSYSCON);
	RN8209_WriteData(EMUCON,Adjust_Parameter_Structure.AdjustEMUCON);
}  
//*****************************************************************************
//��������: void RN8209_DC_Adjust(void )  
//��������: ֱ��У��
//�������: ��
//�������: ��
//******************************************************************************/	
void RN8209_DC_Adjust(void )
{
	u32 temp1_IARMS_DC;
	u32 temp1_URMS_DC;//����������������3���ֽڣ�����ֻ��32λ��
	u32 temp2_IARMS_DC;
	u32 temp2_URMS_DC;
	u32 temp3_IARMS_DC;
	u32 temp3_URMS_DC;
	u8 i;

	u16 temp_DCIAH;
	u16 temp_DCUH;
	u16 temp_DCL_Ia;
	u16 temp_DCL_U;
	u16 temp_DCL;
	
	u16 temp_DC_IARMSOS;

	for(i=0;i<10;i++)
	{
//			temp1_IARMS_DC+=RN8209_ReadData(IARMS,&temp1_IARMS_DC);
		temp1_IARMS_DC+=RN8209_ReadData(IARMS);
			delay_ms(5);
//			temp1_URMS_DC+=RN8209_ReadData(URMS,&temp1_URMS_DC);
		temp1_URMS_DC+=RN8209_ReadData(URMS);
			delay_ms(5);
	}
	temp1_IARMS_DC=(temp1_IARMS_DC/10);
	temp1_URMS_DC=(temp1_URMS_DC/10);//����ӵأ��� IA�� IB�� U ��·����Чֵ 10 �Σ�������Чֵ��ƽ��ֵ IARMS�� IBRMS��URMS
	
	temp_DCIAH=(u16)(temp1_IARMS_DC>>8);//�� IARMS1 ��Чֵ��BIT[23:8]д�� DCIAH �Ĵ�����ok
	temp_DCL_Ia=(((u16)((temp1_IARMS_DC)&0x000000f0))>>4);////BIT[7:4]д�� DCL �Ĵ�����BIT[3:0
	

	temp_DCUH=(u16 )(temp1_URMS_DC>>8);//�� URMS1 ��Чֵ�� BIT[23:8]д�� DCUH �Ĵ����� ok
	temp_DCL_U=(((u16)((temp1_URMS_DC)&0x000000f0))<<4);//BIT[7:4]д�� DCL �Ĵ�����BIT[11:8]
	
	temp_DCL=temp_DCL_U+temp_DCL_Ia;//
	
	Adjust_Parameter_Structure.AdjustDCIAH=temp_DCIAH;
	RN8209_WriteData(DCIAH,Adjust_Parameter_Structure.AdjustDCIAH);//ok
	Adjust_Parameter_Structure.AdjustDCUH=temp_DCUH;
	RN8209_WriteData(DCUH,Adjust_Parameter_Structure.AdjustDCUH);
	Adjust_Parameter_Structure.AdjustDCL=temp_DCL;
	RN8209_WriteData(DCL,Adjust_Parameter_Structure.AdjustDCL);
	
	delay_ms(1000);
	delay_ms(1000);//�ȴ� 2S ��� IA��IB��U ��·����Чֵ 10 �Σ�������Чֵ��ƽ��ֵ IARMS2��IBRMS2��
	for(i=0;i<10;i++)
	{
//			temp2_IARMS_DC+=RN8209_ReadData(IARMS,&temp2_IARMS_DC);
		temp2_IARMS_DC+=RN8209_ReadData(IARMS);
			delay_ms(5);
//			temp2_URMS_DC+=RN8209_ReadData(URMS,&temp2_URMS_DC);
		 temp2_URMS_DC+=RN8209_ReadData(URMS);
			delay_ms(5);
	}
	temp2_IARMS_DC=(temp2_IARMS_DC/10);
	temp2_URMS_DC=(temp2_URMS_DC/10);
	if((temp2_IARMS_DC>temp1_IARMS_DC)||(temp2_URMS_DC>temp1_URMS_DC))//����Чֵ�����δУ��ǰ��С����У����ɣ���ֵ���Ϊԭ����ԼΪ 2 �����������������һ��������
	{
		temp3_IARMS_DC=~temp1_IARMS_DC;//�� IARMS1ȡ���õ� IARMS3
		temp3_URMS_DC=~temp1_URMS_DC;
		
		temp_DCIAH=(u16)(temp3_IARMS_DC>>8);//�� IARMS3 ��Чֵ��BIT[23:8]д�� DCIAH �Ĵ�����
	  temp_DCL_Ia=(((u16)((temp3_IARMS_DC)&0x000000f0))>>4);////BIT[7:4]д�� DCL �Ĵ�����BIT[3:0
	  
	  temp_DCUH=(u16 )(temp3_URMS_DC>>8);//�� URMS3 ��Чֵ�� BIT[23:8]д�� DCUH �Ĵ�����
	  temp_DCL_U=(((u16)((temp3_URMS_DC)&0x000000f0))<<4);//BIT[7:4]д�� DCL �Ĵ�����BIT[11:8]
	  
	  temp_DCL=temp_DCL_U+temp_DCL_Ia;
	  Adjust_Parameter_Structure.AdjustDCIAH=temp_DCIAH;
	  RN8209_WriteData(DCIAH,Adjust_Parameter_Structure.AdjustDCIAH);
	  Adjust_Parameter_Structure.AdjustDCUH=temp_DCUH;
	  RN8209_WriteData(DCUH,Adjust_Parameter_Structure.AdjustDCUH);
	  Adjust_Parameter_Structure.AdjustDCL=temp_DCL;
	  RN8209_WriteData(DCL,Adjust_Parameter_Structure.AdjustDCL);
	}
		delay_ms(1000);
		delay_ms(1000);//�ȶ� 2s ��,�� IA��IB��������Чֵ 10��,�����ƽ��ֵ������Чֵ offset У��
		for(i=0;i<10;i++)
		{
//				temp2_IARMS_DC+=RN8209_ReadData(IARMS,&temp2_IARMS_DC);
			  temp2_IARMS_DC+=RN8209_ReadData(IARMS);
				delay_ms(5);	
		}
		temp2_IARMS_DC=(temp2_IARMS_DC/10);
		temp_DC_IARMSOS=(u16)(~((temp2_IARMS_DC^2)/(2^8)));
		Adjust_Parameter_Structure.AdjustIARMSOS=temp_DC_IARMSOS;
		RN8209_WriteData(IARMSOS,Adjust_Parameter_Structure.AdjustIARMSOS);
}
//*****************************************************************************
//��������: void RN8209_DC_Count_Kx(void ) 
//��������: DC��ѹ������������ת��ϵ��ȷ��
//�������: ��
//�������: ��
//******************************************************************************/	
//void RN8209_DC_Count_Kx(void ) 
//{
//	
//	
//}

//*****************************************************************************
//��������: void RN8209_DC_Count_Kx(void ) 
//��������: DC��ѹ������������ת��ϵ��ȷ��
//�������: ��
//�������: ��
//******************************************************************************/	
u32 RN8209_readIA_RMS(void ) 
{
	*data_r=RN8209_ReadData(0x22);//����Aͨ����Чֵ
	return *data_r;
}