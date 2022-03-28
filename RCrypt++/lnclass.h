
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream> 

// Количество цифр после запятой
#define NEG_ACC 100
// Количество цифр до запятой
#define POS_ACC 100
// Макрос, выцепляющий i-ю цифру (положительные i - 
// цифры после запятой, отрицательные - до)
#define DIGIT(longnum, i) ( (longnum).data[(i)+NEG_ACC] )

class LongNumObj
{
public:
	LongNumObj();
	// конвертирует длинную чиселку в строку
	std::string LNtoStr();
	// переводит короткое число в длинное
	static LongNumObj toLN(int n);
	

	// сравнение ПО МОДУЛЮ двух длинных чисел
	// возвращает 1 если |a|>|b|; -1 если |a|<|b|
	// и возвращает 0 если они равны
	static int LNabscmp(LongNumObj *a, LongNumObj *b);

	// сравнение двух длинных чисел
	// возвращает 1 если a>b ; -1 если a<b
	// и возвращает 0 если они равны
	static int LNcmp(LongNumObj* a, LongNumObj* b);

	// возвращает ln+addment
	// результат сохраняется в ln
	static void LNaddLN(LongNumObj *ln, LongNumObj addment);

	// возвращает ln-subtractment
	// результат сохраняется в ln
	static void LNsubLN(LongNumObj* ln, LongNumObj subtractment);

	// умножает ln на multiplier
	// результат сохраняется в ln
	static void LNmulLN(LongNumObj* ln, LongNumObj multiplier);

	static void LNpow(LongNumObj* ln, int pow);

	// сдвигает ln на displacement десятичных разрядов влево
	// может сдвигать вправо, если displacement<0
	static void LNshlLN(LongNumObj* ln, int displacement);

	// возвращает в ln значение ln/dividor
	static void LNdivLN(LongNumObj* ln, LongNumObj divider);

	void flush();

private:
	char data[NEG_ACC + POS_ACC]; 
	char sign;

	char inline digit(int i);
};
