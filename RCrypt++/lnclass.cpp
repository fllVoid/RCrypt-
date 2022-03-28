#include <vector>
#include "lnclass.h"

using namespace std;


LongNumObj::LongNumObj() {
    // при инициализации записываем ноль
    flush();
}

void LongNumObj::flush() {
    // функция обнуления числа
    int i;
    for (i = 0; i < NEG_ACC + POS_ACC; i++)
        data[i] = 0;
    sign = 1;
}

inline char LongNumObj::digit(int i)
{
    return data[i + NEG_ACC];
}


// конвертирует длинную чиселку в строку
string LongNumObj::LNtoStr()
{
    if (sign == -1)cout << '-';
    // существует два типа цифр 0 (ноль):
    // ведущие нули, которые не следует выводить
    // и нули в середине числа, которые нужно вывести
    // (замыкающие нули - это другая история)
    // переменная flag определяет встретили ли мы какую-либо
    // значащую цифру при движении от старших разрядов к младшим
    // если у нас были значащие цифры и мы наткнулись на ноль, то
    // его надо вывести, т.к. он в середине числа
    vector<char> result;
    int i, j, flag = 0;
    for (i = POS_ACC - 1; i >= 0; i--)
        if (flag || digit(i) != 0) {
            flag = 1;
            result.push_back((char)(digit(i) + '0'));
        }
    // если значащих цифр нет, надо хоть что-то вывести :)
    if (!flag)result.push_back('0');
    // проверяем, есть ли дробная часть у числа, и запоминаем
    // в j ее конец ...
    j = 0;
    for (i = -NEG_ACC; i < 0; i++)
        if (digit(i) != 0) {
            j = i;
            break;
        }
    // ... если есть, то выводим
    if (j != 0) {
        result.push_back('.');
        for (i = -1; i >= j; i--)
            result.push_back((char)(digit(i) + '0'));
    }

    char* chars = new char[result.size()];
    for (int i = 0; i < result.size(); i++) {
        chars[i] = result.at(i);
    }
    return string(chars, result.size());
}

// переводит короткое число в длинное
LongNumObj LongNumObj::toLN(int n)
{
    //int f = 1 + 2;
    LongNumObj *ress = new LongNumObj;
    LongNumObj res = *ress;
    delete ress;
    if (n < 0) {
        res.sign = -1;
        n = -n;
    }
    else res.sign = 1;
    int i = 0, p10 = 1;
    // вычисляем степень 10, соответствующую максимальному
    // разряду короткого целого
    while ((p10 * 10) / 10 == p10) {
        p10 *= 10;
        i++;
        bool bl = (p10 * 10) / 10 == p10;
        //cout << bl;
    }
    // поразрядно выделяем цифры короткого целого
    for (; i >= 0; i--) {
        //res.data[i + NEG_ACC] = n / 10;
        DIGIT(res, i) = n / p10;
        n %= p10;
        p10 /= 10;
        //cout << "v";
    }
    //cout << "y";
    return res;
}

// сравнение ПО МОДУЛЮ двух длинных чисел
// возвращает 1 если |a|>|b|; -1 если |a|<|b|
// и возвращает 0 если они равны
int LongNumObj::LNabscmp(LongNumObj* a, LongNumObj* b)
{
    int i;
    // сравниваем по разрядам, начиная со старших
    for (i = POS_ACC - 1; i >= -NEG_ACC; i--) {
        if (a->digit(i) == b->digit(i))continue;
        if (a->digit(i) > b->digit(i))return 1;
        if (a->digit(i) < b->digit(i))return -1;
    }
    return 0;
}

// сравнение двух длинных чисел
// возвращает 1 если a>b ; -1 если a<b
// и возвращает 0 если они равны
int LongNumObj::LNcmp(LongNumObj* a, LongNumObj* b)
{
    if (a->sign == 1 && b->sign == -1)return 1;
    if (a->sign == -1 && b->sign == 1)return -1;
    if (a->sign == 1 && b->sign == 1)return LNabscmp(a, b);
    if (a->sign == -1 && b->sign == -1)return -LNabscmp(a, b);
    return 0;//never executed
}


// возвращает ln+addment
// результат сохраняется в ln
void LongNumObj::LNaddLN(LongNumObj* ln, LongNumObj addment)
{
    int i, p;
    // если числа одного знака
    if (ln->sign == addment.sign) {
        // в p сохраняем перенос единицы при сложении столбиком
        p = 0;
        // поразрядно складываем
        for (i = -NEG_ACC; i < POS_ACC; i++) {
            p += ln->digit(i) + addment.digit(i);
            DIGIT(*ln, i) = p % 10;
            p /= 10;
        }
    }
    else {
        // сюда попадаем если числа разных знаков
        // если ln<0 то переставим их местами
        if (ln->sign == -1) {
            LongNumObj dummy = *ln;
            *ln = addment;
            addment = dummy;
        }
        int cmp = LNabscmp(ln, &addment);
        if (cmp == 0)
            // если числа равны по модулю и при этом
            // разных знаков, то их сумма - ноль
            ln->flush();
        else if (cmp > 0) {
            addment.sign = 1;
            LNsubLN(ln, addment);
        }
        else { // cmp < 0
            addment.sign = 1;
            LNsubLN(&addment, *ln);
            *ln = addment;
            ln->sign = -1;
        }
    }
}

// возвращает ln-subtractment
// результат сохраняется в ln
void LongNumObj::LNsubLN(LongNumObj* ln, LongNumObj subtractment)
{
    // пробуем сделать так:   a-(-b) == a+b
    if (subtractment.sign == -1) {
        subtractment.sign = 1;
        LNaddLN(ln, subtractment);
        return;
    }

    // здесь пробуем так:   (-a)-b == -(a+b)
    if (ln->sign == -1) {
        ln->sign = 1;
        LNaddLN(ln, subtractment);
        ln->sign = -ln->sign;
        return;
    }

    // здесь ln->sign == subtractment.sign == 1
    int cmp = LNabscmp(ln, &subtractment);
    if (cmp == 0) {
        ln->flush();
        return;
    }

    // тут если a>0, b>0, a<b, то a-b == -(b-a)
    // причем b-a>0
    if (cmp < 0) {
        LongNumObj dummy = *ln;
        *ln = subtractment;
        subtractment = dummy;
        LNsubLN(ln, subtractment);
        ln->sign = -ln->sign;
        return;
    }

    // а вот здесь честно вычитаем по разрядам
    int i, p = 0;
    for (i = -NEG_ACC; i < POS_ACC; i++) {
        p = ln->digit(i) - subtractment.digit(i) - p;
        DIGIT(*ln, i) = (p + 10) % 10;
        if (p < 0)p = 1; else p = 0;
    }
}

// умножает ln на multiplier
// результат сохраняется в ln
void LongNumObj::LNmulLN(LongNumObj* ln, LongNumObj multiplier)
{
    ln->sign *= multiplier.sign;
    int i, j, k, p;
    LongNumObj res;

    // честно умножаем столбиком, и пока кидаем все в res
    for (i = -NEG_ACC; i < POS_ACC; i++) {
        // перебираем только такие j, что разряд, куда
        // помещается произведение i-й цифры первого числа на
        // j-ю второго был в пределах NEG_ACC..POS_ACC
        j = -NEG_ACC - i - 1;
        if (j < -NEG_ACC)j = -NEG_ACC;
        for (; i + j < POS_ACC && j < POS_ACC; j++) {
            p = ln->digit(i) * multiplier.digit(j);
            k = i + j;
            // распихиваем p по разрядам (это нужно если p>=10)
            while (p > 0 && k < POS_ACC) {
                if (k >= -NEG_ACC && k < POS_ACC)
                    DIGIT(res, k) += p % 10;
                p /= 10;
                if (DIGIT(res, k) >= 10) {
                    p += DIGIT(res, k) / 10;
                    DIGIT(res, k) %= 10;
                }
                k++;
            }
        }
    }
    res.sign = ln->sign;
    *ln = res;
}

void LongNumObj::LNpow(LongNumObj* ln, int pow) {
    LongNumObj tmp = *ln;
    if (pow == 0) {
        *ln = LongNumObj::toLN(1);
        return;
    }
    for (int i = 1; i < pow; i++) {
        LNmulLN(ln, tmp);
    }
}

// сдвигает ln на displacement десятичных разрядов влево
// может сдвигать вправо, если displacement<0
void LongNumObj::LNshlLN(LongNumObj* ln, int displacement)
{
    int i;
    LongNumObj res;
    for (i = -NEG_ACC; i < POS_ACC; i++)
        if (
            i - displacement >= -NEG_ACC &&
            i - displacement < POS_ACC
            )
            DIGIT(res, i) = ln->digit(i - displacement);
    *ln = res;
}

// возвращает в ln значение ln/dividor
void LongNumObj::LNdivLN(LongNumObj* ln, LongNumObj divider)
{
    LongNumObj res, zero;
    res.flush(); zero.flush();
    int res_sign = ln->sign * divider.sign;
    ln->sign = divider.sign = 1;
    int p_ln, p_divider;
    // если кто-то нуль, то нечего считать
    if (LNcmp(ln, &zero) == 0) return;
    if (LNcmp(&divider, &zero) == 0) {
        // делить на ноль ни в коем случае нельзя!
        ln->flush();
        return;
    }
    // найдем порядки чисел 
    int i, j;
    for (i = POS_ACC - 1; i >= -NEG_ACC; i--)
        if (ln->digit(i) != 0) {
            p_ln = i;
            break;
        }
    for (i = POS_ACC - 1; i >= -NEG_ACC; i--)
        if (divider.digit(i) != 0) {
            p_divider = i;
            break;
        }
    // вычислим как надо сдвинуть делитель, чтобы его порядок
    // совпал с делимым
    i = p_ln - p_divider;
    if (i > POS_ACC) i = POS_ACC - 1 - p_divider;
    // сдвигаем...
    LNshlLN(&divider, i);
    LongNumObj tempnum;
    // по разрядам вычисляем частное
    for (; i > -NEG_ACC; i--) {
        tempnum.flush();
        // подбираем такое j и tempnum == j*divider,
        // где divider уже сдвинут на i разрядов
        // чтобы j*divider <= ln и (j+1)*divider > ln
        // в таком случае j - это цифра i-го разряда частного
        j = 0;
        do {
            LNaddLN(&tempnum, divider);
            j++;
        } while (LNabscmp(&tempnum, ln) <= 0);
        LNsubLN(&tempnum, divider);
        j--;
        // ок, подобрали j, теперь вычитаем из ln tempnum
        // (как при делении столбиком)
        LNsubLN(ln, tempnum);
        DIGIT(res, i) = j;

        // сдвигаем вправо на один разряд для вычисления следущей цифры
        LNshlLN(&divider, -1);
    }
    res.sign = res_sign;
    *ln = res;
}

