// отправка команды СТАРТ
void I2C_StartCondition(void)
{
TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
while (!(TWCR & (1<<TWINT)));//ожидание установки бита TWIN
}

// отправка СТОП
void I2C_StopCondition(void)
{
TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

//отправка байта
void I2C_SendByte(unsigned char c)
{
TWDR = c;//загрузка значения в регистр данных
TWCR = (1<<TWINT)|(1<<TWEN);//начаало передачи байта данных
while (!(TWCR & (1<<TWINT)));//ожидание установки бита TWIN
}

//инициализация I2C как передатчика
void I2C_Init (void)
{
TWBR=0xFF;//скорость передачи
}

//отправка SLA_W + байт данных
void I2C_SendPocket (unsigned char value,unsigned char adres_rw)
{
I2C_StartCondition(); // генерируем условие СТАРТ
I2C_SendByte(adres_rw); //оправляем адрес устройства+бит запись
I2C_SendByte(value);//отправляем байт данных
I2C_StopCondition();//генерируем условие СТОП
}

//добавляем функции чтения по I2C
//чтение байта
unsigned char I2C_RecieveByte(void)
{
unsigned char dat;//переменная хранения значения
TWCR |= (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
while (!(TWCR & (1<<TWINT)));//ожидание установки бита TWIN
dat = TWDR;//читаем регистр данных
return dat;
}

//чтение последнего байта
unsigned char I2C_RecieveLastByte(void)
{
unsigned char dat;//переменная хранения значения	
TWCR |= (1<<TWINT)|(1<<TWEN)|(0<<TWEA);//чтение последнего байта
while (!(TWCR & (1<<TWINT)));//ожидание установки бита TWIN
dat = TWDR;//читаем регистр данных	
return dat;
}	
