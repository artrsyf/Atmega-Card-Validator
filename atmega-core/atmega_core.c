#define F_CPU 8000000UL
#define BAUD 4800
#define MY_UBRR 103
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <ctype.h>
#include "twi.h"
#include "lcd_i2c.h"
#include <avr/interrupt.h>	// ??????????

#include "mfrc522.h"

// ?????? DS18B20 - ????? ????????? ????? ??
#define DS18B20			0
#define DS18B20_PORT	PORTD
#define DS18B20_PIN		PIND
#define DS18B20_DDR		DDRD

uint8_t DS_scratchpad[9] = {0,0,0,0,0,0,0,0,0};	// 9 ????, ????????? ? DS18B20, ??? ??? ?????????? "???????"
uint8_t Presense_errors = 0;		// ??????? ?????? - ????????????? DS18B20
uint8_t Short_circuit_errors = 0;	// ??????? ?????? - ?? ????? ?????? DS18B20
int8_t Temperature = 0;


void uart_init() {
    unsigned int ubrr = 103;
    UBRRH = (unsigned char)(ubrr>>8);  // ??????? ????
    UBRRL = (unsigned char)ubrr;       // ??????? ????
    UCSRB = (1<<RXEN) | (1<<TXEN);     // ???????? ???????? ? ??????????
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);
}

void uart_transmit(unsigned char data) {
    while (!(UCSRA & (1<<UDRE)));  // ????, ???? ??????? ???????? ????? ????????
    UDR = data;  // ???????? ??????
}

void uart_send_string(const char *str) {
    while(*str) {
        uart_transmit(*str);
        str++;
    }
}

unsigned char uart_receive(void) {
    while (!(UCSRA & (1<<RXC)));  // ????, ???? ? ???????? ???? ??????
    return UDR;  // ?????????? ???????? ????
}

void uart_receive_string(char *str, unsigned int max_len) {
    unsigned int i = 0;
    while (i < max_len - 1) {
        str[i] = uart_receive();  // ????????? ??????
        if (str[i] == '\n') {  // ????? ??????
            break;
        }
        i++;
    }
    str[i] = '\0';  // ??????????? ??????? ??????
}

#define LCD_MAX_COLS 16  // ?????????? ???????? ? ??????
#define LCD_MAX_ROWS 2   // ?????????? ????? ?? ???????

static char lcd_current_col = 0;  // ??????? ???????
static char lcd_current_row = 0;  // ??????? ??????

void lcd_putchar_auto(char c) {
    if (lcd_current_col >= LCD_MAX_COLS) {
        lcd_current_col = 0;
        lcd_current_row++;
        if (lcd_current_row >= LCD_MAX_ROWS) {
            lcd_current_row = 0;  // ????????? ?? ?????? ??????, ???? ??? ?????? ?????????
        }
        lcd_gotoxy(lcd_current_col, lcd_current_row);
    }

    lcd_putchar(c);  // ?????????? ???????????? ??????? ??? ?????? ???????
    lcd_current_col++;
}

void lcd_putstring_auto (char stroka[])
{  char i;
   for(i=0;stroka[i]!='\0';i++)
   lcd_putchar_auto(stroka[i]);
}

void clear_temp_data(char* operation_symbol, char* buffer, char* uart_buffer, uchar* card_num, size_t buffer_size, size_t uart_buffer_size, size_t card_num_size)
{
    *operation_symbol = 'e';
    memset(buffer, 0, buffer_size);
    memset(uart_buffer, 0, uart_buffer_size);
    memset(card_num, 0, card_num_size);
}


int extract_json_value(const char* buffer, const char* key, char* value, size_t value_size, int parse_as_string) {
    const char* json_start = strchr(buffer, '{'); // Найти начало JSON
    if (!json_start) {
        return -1; // JSON не найден
    }

    const char* key_position = strstr(json_start, key); // Найти ключ
    if (!key_position) {
        return -2; // Ключ не найден
    }

    // Найти символ ':' после ключа
    const char* colon_position = strchr(key_position, ':');
    if (!colon_position) {
        return -3; // Ошибка синтаксиса JSON
    }

    // Пропустить пробелы после ':'
    const char* value_start = colon_position + 1;
    while (*value_start == ' ') {
        value_start++;
    }

    // Если ключ нужно парсить как строку
    if (parse_as_string) {
        // Если значение в кавычках, то обрабатываем как строку
        if (*value_start == '\"') {
            value_start++; // Пропустить первую кавычку
            const char* value_end = strchr(value_start, '\"'); // Найти закрывающую кавычку
            if (!value_end) {
                return -4; // Ошибка синтаксиса JSON (не закрыта кавычка)
            }

            size_t length = value_end - value_start;
            if (length >= value_size) {
                return -5; // Значение слишком большое для буфера
            }

            strncpy(value, value_start, length);
            value[length] = '\0'; // Завершить строку
            return 0; // Успех
        }
    }

    // Если парсим как число
    const char* value_end = value_start;
    while (*value_end && !isspace(*value_end) && *value_end != ',' && *value_end != '}') {
        value_end++;
    }

    // Если значение найдено
    size_t length = value_end - value_start;
    if (length >= value_size) {
        return -6; // Значение слишком большое для буфера
    }

    // Копировать значение в выходной буфер
    strncpy(value, value_start, length);
    value[length] = '\0'; // Завершить строку

    return 0; // Успех
}

int main(void)
{
	sei();

	DDRA &= ~((1 << PA0) | (1 << PA2));
	PORTA |= (1 << PA0) | (1 << PA2);

	I2C_Init();//????????????? ??????
	uart_init();
	lcd_init();//????????????? LCD
	lcd_clear();//??????? LCD ? ??????? ??????? ? 0.0
	MFRC522_Init();

	char operation_symbol = 'e';
	
	char buffer[5];
	char uart_buffer[64];
	uchar card_num[5];
	uint8_t i;
	
	lcd_putstring("Initialization..");
	_delay_ms(5000);
	lcd_clear();
	lcd_gotoxy(0, 0);
	
	while (1)
	{
		if (MFRC522_Request(PICC_REQIDL, card_num) == MI_OK && MFRC522_Anticoll(card_num) == MI_OK) {
	        // ????????? ?????? ID ?????
	        char card_id_string[10];  // 8 ???????? ??? ID + 1 ??? ????????????? ???????
	        for (i = 0; i < 4; i++) {
	            sprintf(&card_id_string[i * 2], "%02X", card_num[i]);  // ??????????? ???? ? ??????
	        }
			card_id_string[8] = '\n';
	        card_id_string[9] = '\0';  // ????????? ??????
			
			if (PINA & (1 << PA2)) {
				// Если на PA2 есть напряжения (логическая 1)
				// Получение инфы
				operation_symbol = 'i';

				char info_payload[14];
				sprintf(info_payload, "GET#%s", card_id_string);
	            uart_send_string(info_payload);
			} else {
				if (PINA & (1 << PA0)) {
					// Если на PA0 есть напряжения (логическая 1)
					// Оплата проезда
					operation_symbol = '-';

					char refill_payload[17];
					sprintf(refill_payload, "POST#0#%s", card_id_string);
					uart_send_string(refill_payload);
		        } else {
		            // Если на PA0 нет напряжения (логическая 0)
					// Пополнение баланса
					operation_symbol = '+';

					char refill_payload[17];
					sprintf(refill_payload, "POST#1#%s", card_id_string);
					uart_send_string(refill_payload);
	            
		        }
			}

	        uart_receive_string(uart_buffer, sizeof(uart_buffer));

			char id_key[] = "card_id";
			char balance_key[] = "balance";
			char id_value[16];
			char balance_value[32];
			extract_json_value(uart_buffer, id_key, id_value, sizeof(id_value), 1);
			extract_json_value(uart_buffer, balance_key, balance_value, sizeof(balance_value), 0);

			char id_output[32];
			char balance_output[48];
			sprintf(id_output, "ID: %s", id_value);
			sprintf(balance_output, "Balance: %s", balance_value);

	        lcd_clear();

			lcd_gotoxy(0, 0);
			lcd_putstring(id_output);

			lcd_gotoxy(15, 0);
			lcd_putchar(operation_symbol);

			lcd_gotoxy(0, 1);
			lcd_putstring(balance_output);

	        _delay_ms(3000);
			clear_temp_data(&operation_symbol, buffer, uart_buffer, card_num, sizeof(buffer), sizeof(uart_buffer), sizeof(card_num));
			lcd_clear();
		} else {
			lcd_gotoxy(0, 0);
			lcd_putstring("Ready for card");
		}
		
	}
}
