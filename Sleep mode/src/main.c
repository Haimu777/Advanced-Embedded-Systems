#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
volatile unsigned char wake_up = 0;
void LCD_Enable(void)
{
    PORTB |= (1 << PB3);
    PORTB &= ~(1 << PB3);
}
void LCD_Send4Bit(unsigned char data)
{
    PORTC &= 0xF0;
    PORTC |= (data & 0x0F);
    LCD_Enable();
}
void LCD_Command(unsigned char command)
{
    PORTB &= ~(1 << PB2);
    LCD_Send4Bit(command >> 4);
    LCD_Send4Bit(command & 0x0F);
    if (command == 0x01 || command == 0x02)
        _delay_ms(2);
    else
        _delay_us(50);
}
void LCD_Data(unsigned char data)
{
    PORTB |= (1 << PB2);
    LCD_Send4Bit(data >> 4);
    LCD_Send4Bit(data & 0x0F);
    _delay_us(50);
}
void LCD_Init(void)
{
    DDRB |= (1 << PB2);
    DDRB |= (1 << PB3);
    DDRC |= (1 << PC0);
    DDRC |= (1 << PC1);
    DDRC |= (1 << PC2);
    DDRC |= (1 << PC3);
    _delay_ms(20);
    PORTB &= ~(1 << PB2);
    LCD_Send4Bit(0x03);
    _delay_ms(5);
    LCD_Send4Bit(0x03);
    _delay_us(150);
    LCD_Send4Bit(0x03);
    _delay_us(150);
    LCD_Send4Bit(0x02);
    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
    _delay_ms(2);
}
void LCD_Clear(void)
{
    LCD_Command(0x01);
    _delay_ms(2);
}
void LCD_String(const char *text)
{
    while (*text)
    {
        LCD_Data(*text);
        text++;
    }
}
void LCD_SetCursor(unsigned char row, unsigned char column)
{
    if (row == 0)
        LCD_Command(0x80 + column);
    else
        LCD_Command(0xC0 + column);
}
ISR(INT0_vect)
{
    wake_up = 1;
}
void INT0_Init(void)
{
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);
    EICRA &= ~(1 << ISC01);
    EICRA &= ~(1 << ISC00);
    EIFR |= (1 << INTF0);
    EIMSK |= (1 << INT0);
}
void Sleep_Init(void)
{
    SMCR &= ~(1 << SM2);
    SMCR |= (1 << SM1);
    SMCR &= ~(1 << SM0);
}
void Enter_Sleep(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("ENTERING SLEEP");
    LCD_SetCursor(1, 0);
    LCD_String("POWER-DOWN");
    PORTB &= ~(1 << PB0);
    PORTB &= ~(1 << PB1);
    wake_up = 0;
    EIFR |= (1 << INTF0);
    SMCR |= (1 << SE);
    sleep_cpu();
    SMCR &= ~(1 << SE);
}
int main(void)
{
    DDRB |= (1 << PB0);
    DDRB |= (1 << PB1);
    PORTB &= ~(1 << PB0);
    PORTB &= ~(1 << PB1);
    LCD_Init();
    INT0_Init();
    Sleep_Init();
    sei();
    while (1)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_String("SYSTEM ACTIVE");
        LCD_SetCursor(1, 0);
        LCD_String("NORMAL MODE");
        PORTB |= (1 << PB0);
        _delay_ms(500);
        Enter_Sleep();
        if (wake_up == 1)
        {
            PORTB |= (1 << PB0);
            PORTB |= (1 << PB1);
            PORTB &= ~(1 << PB1);
            LCD_Clear();
            LCD_SetCursor(0, 0);
            LCD_String("MCU AWAKE");
            LCD_SetCursor(1, 0);
            LCD_String("INT0 WAKE-UP");
            _delay_ms(100);
            wake_up = 0;
            while (!(PIND & (1 << PD2)))
            {
            }
            _delay_ms(50);
        }
    }
}