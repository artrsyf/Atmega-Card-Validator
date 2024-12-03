
//LCD ���������� ����������
#include <avr/io.h>
#include <util/delay.h>

//����� LCD �����������
#define LCD_ADDR 0b00100111
//������ ������ + ��� ������
#define SLA_W   LCD_ADDR<<1

//������� ���������� ���������� �������� �����
unsigned char Led_port=0;

//������ �������� ������� LCD
#define I2C_SendPocketLCD(x) I2C_SendPocket (x,SLA_W)

/*
������� ������������
Po - RS
P1 - R/W
P2 - E
P3 - ���������
P4 - D4
P5 - D5
P6 - D6
P7 - D7
 */

/* ��������� ���. "1" �� ����� RS */
#define LCD_RS_SET  I2C_SendPocketLCD (Led_port|=0x01)
/* ��������� ���. "0" �� ����� RS */
#define LCD_RS_CLR  I2C_SendPocketLCD (Led_port&=~0x01)
/* ��������� ���. "1"(������) R/W*/
#define LCD_RW_SET  I2C_SendPocketLCD (Led_port|=0x02)
/* ��������� ���. "0"(������) R/W */
#define LCD_RW_CLR  I2C_SendPocketLCD (Led_port&=~0x02)
/* ��������� ���. "1" �� ����� E */
#define LCD_E_SET   I2C_SendPocketLCD (Led_port |=0x04)
/* ��������� ���. "0" �� ����� E */
#define LCD_E_CLR   I2C_SendPocketLCD (Led_port &=~0x04)
/* ��������� ���. "1" �� ����� ��������� */
#define LCD_BT_SET  I2C_SendPocketLCD (Led_port|=0x08)
/* ��������� ���. "0" �� ����� ��������� */
#define LCD_BT_CLR  I2C_SendPocketLCD (Led_port&=~0x08)


/* ������, ����������� �������, ���
���������� ������� */
#define LCD_COMMAND  0
/* ������, ����������� �������, ���
���������� ������ */
#define LCD_DATA     1

#define lcd_putc(x)  lcd_putbyte(x, LCD_DATA)

/* ������� �������� ������� � ��� */
void lcd_putnibble(char t)
{
    t<<=4;
    LCD_E_SET;
    _delay_us(50);
    I2C_SendPocketLCD (Led_port|t);
    LCD_E_CLR;
    _delay_us(50);
}

/* ������� �������� ����� � ���.
char c - ��� ����
char rs - ����������, ����������� ��� ����������:
     rs = 0 - ������� (��������������� ����� RS)
	 rs = 1 - ������ (������������ ����� RS) */
void lcd_putbyte(char c, char rs)
{
    char highc=0;
    highc=c>>4;
	if (rs==LCD_COMMAND) LCD_RS_CLR;
	else                 LCD_RS_SET;
    lcd_putnibble(highc);
    lcd_putnibble(c);
}

void lcd_putchar(char c)
{
    char highc=0;
    highc=c>>4;
	LCD_RS_SET;
    lcd_putnibble(highc);
    lcd_putnibble(c);
}

/* ������� ������������� ������ ���
� 4-������ ������, ��� ������� */
void lcd_init()
{
    _delay_ms(15);
    lcd_putnibble(0b00000011);
    _delay_ms(4);
    lcd_putnibble(0b00000011);
    _delay_us(100);
    lcd_putnibble(0b00000011);
	 _delay_ms(1);
    lcd_putnibble(0b00000010);
	_delay_ms(1);
    lcd_putbyte(0x28, LCD_COMMAND); // ����� (0x28) 5x8 0b00101000 ������ (0x2C) 5x10 0b00101100 
    _delay_ms(1);
    lcd_putbyte(0x0C, LCD_COMMAND);
    _delay_ms(1);
    lcd_putbyte(0x06, LCD_COMMAND);
    _delay_ms(1);
	LCD_BT_SET;//�������� ���������
	LCD_RW_CLR;//������ r/w
}

/* ������� ������� ������� � ��������
������� � ��������� �������*/
void lcd_clear()
{
    lcd_putbyte(0x01, LCD_COMMAND);
    _delay_us(1500);
}

/* ������� ����������� ������� � �������� �������
col - ����� ���������� �� �������������� ��� (�� 0 �� 15)
row - ����� ������ (0 ��� 1) */
void lcd_gotoxy(char col, char row)
{
  char adr;
  adr=0x40*row+col;
  adr|=0x80;
  lcd_putbyte(adr, LCD_COMMAND);
}

void lcd_putstring (char stroka[])
{  char i;
   for(i=0;stroka[i]!='\0';i++)
   lcd_putchar(stroka[i]);
}	
