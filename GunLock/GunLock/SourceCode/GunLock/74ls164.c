#include "SmartM_M0.h"
#include "74ls164.h"

								
/****************************************
*��������:LS164Init
*��    ��:��
*��    ��:��
*��    ��:74LS164��ʼ��
******************************************/
VOID LS164Init(VOID)
{
   P0_PMD &= ~(3UL<<(LS164_DATA_PIN<<1));//IOģʽ����,P0.4
   P0_PMD &= ~(3UL<<(LS164_CLK_PIN <<1));

	 P0_PMD |= 1UL<<(LS164_DATA_PIN<<1);//IOģʽ��P0.4����Ϊ���ģʽ
	 P0_PMD |= 1UL<<(LS164_CLK_PIN <<1);//IOģʽ��P0.5����Ϊ���ģʽ
}


/****************************************
*��������:LS164Send
*��    ��:byte �����ֽ�
*��    ��:��
*��    ��:74LS164���͵����ֽ�
******************************************/
VOID LS164Send_B2(UINT8 d)
{
   UINT8 i;
   for(i=0; i<=7; i++)
   {
		 if(d & (1<<(7-i)))
     {
			 LS164_DATA(1);
	   }
		 else
		 {
	     LS164_DATA(0);
		 }

     LS164_CLK(0); 
     LS164_CLK(1); 
   }
}