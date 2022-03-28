#include "Cube1Bit.h"
#include <functional>
#include <map>
#include "ScrambleHelper.h"

using namespace std;
typedef std::function<void()> Action;

Cube1Bit::Cube1Bit(bool decrypt, unsigned long long seed, string scramble)
{
    _bytes = new char[blockSize] {0};
    _offset = 1;
    _scramble = scramble;
    SetScramble(scramble);
    InitFirstVector(seed);
    if (decrypt)
    {
        _decryptMode = decrypt;
        _scramble = ScrambleHelper::ReverseScramble(_scramble);
        SetScramble(_scramble);
    }
    _startNumIndex = _decryptMode ? _msize(_nums) - 1 : 0;
    _offset = _decryptMode ? -1 : 1;
    delete[] _bytes;
}

Cube1Bit::~Cube1Bit() {
    if (_nums)
        delete[] _nums;
}

void Cube1Bit::Init(char* bytes)
{
    _bytes = bytes;
}

char* Cube1Bit::DoScramble()
{
    _numIndex = _startNumIndex;
    int count = _moves.size();
    if (_decryptMode)
        SaveBytes(_tmpBytes);
    for (int i = 0; i < count; ++i)
    {
        (this->*_moves[i])();
    }
    char* tmpArr = _decryptMode ? _tmpBytes : _bytes;
    for (int i = 0; i < _moves.size(); ++i)
        _nums[i] = tmpArr[i % blockSize];
    return _bytes;
}

//void Cube1Bit::DoScramble(char* bytes)
//{
//    _numIndex = _startNumIndex;
//    int count = _moves.size();
//    if (_decryptMode)
//        SaveBytes(_tmpBytes);
//    for (int i = 0; i < count; ++i)
//    {
//        (this->*_moves[i])();
//    }
//    SaveBytes(bytes);
//    char* tmpArr = _decryptMode ? _tmpBytes : bytes;
//    //rewrite vector
//    for (int i = 0; i < _moves.size(); ++i)
//        _nums[i] = tmpArr[i % blockSize];
//}

void Cube1Bit::SetScramble(string scramble)
{
    _moves.clear();
    int i = 0;
    auto addMove = [&](void(Cube1Bit::* move)(), void(Cube1Bit::* antiMove)())
    {
        if (i + 1 == scramble.length())
            _moves.push_back(move);
        else if (scramble[i + 1] == '\'')
        {
            _moves.push_back(antiMove);
            ++i;
        }
        else if (scramble[i + 1] == '2')
        {
            if (_decryptMode)
            {
                _moves.push_back(antiMove);
                _moves.push_back(antiMove);
            }
            else
            {
                _moves.push_back(move);
                _moves.push_back(move);
            }
            ++i;
        }
        else
            _moves.push_back(move);
    };
    map<char, Action> dict;

    dict['R'] = [&]() {addMove(&Cube1Bit::R, &Cube1Bit::AntiR); };
    dict['L'] = [&]() {addMove(&Cube1Bit::L, &Cube1Bit::AntiL); };
    dict['U'] = [&]() {addMove(&Cube1Bit::U, &Cube1Bit::AntiU); };
    dict['D'] = [&]() {addMove(&Cube1Bit::D, &Cube1Bit::AntiD); };
    dict['F'] = [&]() {addMove(&Cube1Bit::F, &Cube1Bit::AntiF); };
    dict['B'] = [&]() {addMove(&Cube1Bit::B, &Cube1Bit::AntiB); };
    dict['S'] = [&]() {addMove(&Cube1Bit::S, &Cube1Bit::AntiS); };
    dict['E'] = [&]() {addMove(&Cube1Bit::E, &Cube1Bit::AntiE); };
    dict['M'] = [&]() {addMove(&Cube1Bit::M, &Cube1Bit::AntiM); };
    dict['C'] = [&]() {addMove(&Cube1Bit::C, &Cube1Bit::AntiC); };
    for (i = 0; i < scramble.length(); ++i)
    {
        if (dict.count(scramble[i]))
        {
            dict[scramble[i]]();
        }
    }
}

void Cube1Bit::SaveBytes(char* bytes)
{
    bytes[0] = _bytes[0];
    bytes[1] = _bytes[1];
    bytes[2] = _bytes[2];
    bytes[3] = _bytes[3];
    bytes[4] = _bytes[4];
    bytes[5] = _bytes[5];
}

void Cube1Bit::InitFirstVector(unsigned long long seed)
{
    delete[] _nums;
    _nums = new char[_moves.size()]{ 0 };
    for (int i = 0; i < blockSize; ++i)
    {
        _bytes[i] = (char)(seed % 256);
        seed = seed / 256 * 2333;
    }
    DoScramble();
}

void Cube1Bit::R()
{
    char maskGet = 0b10010100;
    char maskClear = 0b01101011;
    char tus, tfs, tbs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        tds = (char)(_bytes[5] - mud);

        _bytes[4] = (char)(tbs & maskClear | maskGet & tus);

        _bytes[5] = (char)(tds & maskClear | maskGet & tbs);

        _bytes[2] = (char)(tfs & maskClear | maskGet & tds);

        _bytes[0] = (char)(tus & maskClear | maskGet & tfs);
    }
    else
    {
        tus = _bytes[0];
        tfs = _bytes[2];
        tbs = _bytes[4];
        tds = _bytes[5];

        _bytes[4] = (char)((tbs & maskClear | maskGet & tus) + mud);

        _bytes[5] = (char)((tds & maskClear | maskGet & tbs) + mud);

        _bytes[2] = (char)((tfs & maskClear | maskGet & tds) + mud);

        _bytes[0] = (char)((tus & maskClear | maskGet & tfs) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(3);
}

void Cube1Bit::L()
{
    char maskGet = 0b00101001;
    char maskClear = 0b11010110;//check
    char tus, tfs, tbs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        tds = (char)(_bytes[5] - mud);
        _bytes[2] = (char)(tfs & maskClear | maskGet & tus);

        _bytes[5] = (char)(tds & maskClear | maskGet & tfs);

        _bytes[4] = (char)(tbs & maskClear | maskGet & tds);

        _bytes[0] = (char)(tus & maskClear | maskGet & tbs);
    }
    else
    {
        tus = _bytes[0];
        tfs = _bytes[2];
        tbs = _bytes[4];
        tds = _bytes[5];
        _bytes[2] = (char)((tfs & maskClear | maskGet & tus) + mud);

        _bytes[5] = (char)((tds & maskClear | maskGet & tfs) + mud);

        _bytes[4] = (char)((tbs & maskClear | maskGet & tds) + mud);

        _bytes[0] = (char)((tus & maskClear | maskGet & tbs) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(1);
}

void Cube1Bit::F()
{
    char maskClearForR = 0b11010110;
    char maskClearForD = 0b11111000;
    char maskClearForL = 0b01101011;
    char maskClearForU = 0b00011111;
    char tus, tls, trs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tls = (char)(_bytes[1] - mud);
        trs = (char)(_bytes[3] - mud);
        tds = (char)(_bytes[5] - mud);
        _bytes[3] = (char)(trs & maskClearForR | tus >> 5 & 0b00000001 | tus >> 3 & 0b00001000 | tus >> 2 & 0b00100000);

        _bytes[5] = (char)(tds & maskClearForD | trs >> 5 & 0b00000001 | trs >> 2 & 0b00000010 | trs << 2 & 0b00000100);

        _bytes[1] = (char)(tls & maskClearForL | tds << 2 & 0b00000100 | tds << 3 & 0b00010000 | tds << 5 & 0b10000000);

        _bytes[0] = (char)(tus & maskClearForU | tls << 5 & 0b10000000 | tls << 2 & 0b01000000 | tls >> 2 & 0b00100000);
    }
    else
    {
        tus = _bytes[0];
        tls = _bytes[1];
        trs = _bytes[3];
        tds = _bytes[5];
        _bytes[3] = (char)((trs & maskClearForR | tus >> 5 & 0b00000001 | tus >> 3 & 0b00001000 | tus >> 2 & 0b00100000) + mud);

        _bytes[5] = (char)((tds & maskClearForD | trs >> 5 & 0b00000001 | trs >> 2 & 0b00000010 | trs << 2 & 0b00000100) + mud);

        _bytes[1] = (char)((tls & maskClearForL | tds << 2 & 0b00000100 | tds << 3 & 0b00010000 | tds << 5 & 0b10000000) + mud);

        _bytes[0] = (char)((tus & maskClearForU | tls << 5 & 0b10000000 | tls << 2 & 0b01000000 | tls >> 2 & 0b00100000) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(2);
}

void Cube1Bit::B()
{
    char maskClearForR = 0b01101011;
    char maskClearForD = 0b00011111;
    char maskClearForL = 0b11010110;
    char maskClearForU = 0b11111000;
    char tus, tls, trs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tls = (char)(_bytes[1] - mud);
        trs = (char)(_bytes[3] - mud);
        tds = (char)(_bytes[5] - mud);
        _bytes[3] = (char)(trs & maskClearForR | tds >> 5 & 0b00000100 | tds >> 2 & 0b00010000 | tds << 2 & 0b10000000);

        _bytes[5] = (char)(tds & maskClearForD | tls << 2 & 0b10000000 | tls << 3 & 0b01000000 | tls << 5 & 0b00100000);

        _bytes[1] = (char)(tls & maskClearForL | tus << 5 & 0b00100000 | tus << 2 & 0b00001000 | tus >> 2 & 0b00000001);

        _bytes[0] = (char)(tus & maskClearForU | trs >> 2 & 0b00000001 | trs >> 3 & 0b00000010 | trs >> 5 & 0b00000100);
    }
    else
    {
        tus = _bytes[0];
        tls = _bytes[1];
        trs = _bytes[3];
        tds = _bytes[5];
        _bytes[3] = (char)((trs & maskClearForR | tds >> 5 & 0b00000100 | tds >> 2 & 0b00010000 | tds << 2 & 0b10000000) + mud);

        _bytes[5] = (char)((tds & maskClearForD | tls << 2 & 0b10000000 | tls << 3 & 0b01000000 | tls << 5 & 0b00100000) + mud);

        _bytes[1] = (char)((tls & maskClearForL | tus << 5 & 0b00100000 | tus << 2 & 0b00001000 | tus >> 2 & 0b00000001) + mud);

        _bytes[0] = (char)((tus & maskClearForU | trs >> 2 & 0b00000001 | trs >> 3 & 0b00000010 | trs >> 5 & 0b00000100) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(4);
}

void Cube1Bit::U()
{
    char maskClearForRFL = 0b11111000;
    char maskClearForB = 0b00011111;
    char trs, tls, tfs, tbs;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        trs = (char)(_bytes[3] - mud);
        tls = (char)(_bytes[1] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        _bytes[3] = (char)(trs & maskClearForRFL | tbs >> 7 & 0b00000001 | tbs >> 5 & 0b00000010 | tbs >> 3 & 0b00000100);

        _bytes[4] = (char)(tbs & maskClearForB | tls << 7 & 0b10000000 | tls << 5 & 0b01000000 | tls << 3 & 0b00100000);

        _bytes[1] = (char)(tls & maskClearForRFL | tfs & 0b00000111);

        _bytes[2] = (char)(tfs & maskClearForRFL | trs & 0b00000111);
    }
    else
    {
        trs = _bytes[3];
        tls = _bytes[1];
        tfs = _bytes[2];
        tbs = _bytes[4];
        _bytes[3] = (char)((trs & maskClearForRFL | tbs >> 7 & 0b00000001 | tbs >> 5 & 0b00000010 | tbs >> 3 & 0b00000100) + mud);

        _bytes[4] = (char)((tbs & maskClearForB | tls << 7 & 0b10000000 | tls << 5 & 0b01000000 | tls << 3 & 0b00100000) + mud);

        _bytes[1] = (char)((tls & maskClearForRFL | tfs & 0b00000111) + mud);

        _bytes[2] = (char)((tfs & maskClearForRFL | trs & 0b00000111) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(0);
}

void Cube1Bit::D()
{
    char maskClearForRFL = 0b00011111;
    char maskClearForB = 0b11111000;
    char trs, tls, tfs, tbs;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        trs = (char)(_bytes[3] - mud);
        tls = (char)(_bytes[1] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        _bytes[1] = (char)(tls & maskClearForRFL | tbs << 7 & 0b10000000 | tbs << 5 & 0b01000000 | tbs << 3 & 0b00100000);

        _bytes[4] = (char)(tbs & maskClearForB | trs >> 7 & 0b00000001 | trs >> 5 & 0b00000010 | trs >> 3 & 0b00000100);

        _bytes[3] = (char)(trs & maskClearForRFL | tfs & 0b11100000);

        _bytes[2] = (char)(tfs & maskClearForRFL | tls & 0b11100000);
    }
    else
    {
        trs = _bytes[3];
        tls = _bytes[1];
        tfs = _bytes[2];
        tbs = _bytes[4];
        _bytes[1] = (char)((tls & maskClearForRFL | tbs << 7 & 0b10000000 | tbs << 5 & 0b01000000 | tbs << 3 & 0b00100000) + mud);

        _bytes[4] = (char)((tbs & maskClearForB | trs >> 7 & 0b00000001 | trs >> 5 & 0b00000010 | trs >> 3 & 0b00000100) + mud);

        _bytes[3] = (char)((trs & maskClearForRFL | tfs & 0b11100000) + mud);

        _bytes[2] = (char)((tfs & maskClearForRFL | tls & 0b11100000) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(5);
}

void Cube1Bit::AntiR()
{
    char maskGet = 0b10010100;
    char maskClear = 0b01101011;
    char tus, tfs, tbs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        tds = (char)(_bytes[5] - mud);

        _bytes[2] = (char)(tfs & maskClear | maskGet & tus);

        _bytes[5] = (char)(tds & maskClear | maskGet & tfs);

        _bytes[4] = (char)(tbs & maskClear | maskGet & tds);

        _bytes[0] = (char)(tus & maskClear | maskGet & tbs);
    }
    else
    {
        tus = _bytes[0];
        tfs = _bytes[2];
        tbs = _bytes[4];
        tds = _bytes[5];

        _bytes[2] = (char)((tfs & maskClear | maskGet & tus) + mud);

        _bytes[5] = (char)((tds & maskClear | maskGet & tfs) + mud);

        _bytes[4] = (char)((tbs & maskClear | maskGet & tds) + mud);

        _bytes[0] = (char)((tus & maskClear | maskGet & tbs) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(3);
}

void Cube1Bit::AntiL()
{
    char maskGet = 0b00101001;
    char maskClear = 0b11010110;//check
    char tus, tfs, tbs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        tds = (char)(_bytes[5] - mud);
        _bytes[2] = (char)(tfs & maskClear | maskGet & tds);

        _bytes[0] = (char)(tus & maskClear | maskGet & tfs);

        _bytes[4] = (char)(tbs & maskClear | maskGet & tus);

        _bytes[5] = (char)(tds & maskClear | maskGet & tbs);
    }
    else
    {
        tus = _bytes[0];
        tfs = _bytes[2];
        tbs = _bytes[4];
        tds = _bytes[5];
        _bytes[2] = (char)((tfs & maskClear | maskGet & tds) + mud);

        _bytes[0] = (char)((tus & maskClear | maskGet & tfs) + mud);

        _bytes[4] = (char)((tbs & maskClear | maskGet & tus) + mud);

        _bytes[5] = (char)((tds & maskClear | maskGet & tbs) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(1);
}

void Cube1Bit::AntiF()
{
    char maskClearForR = 0b11010110;
    char maskClearForD = 0b11111000;
    char maskClearForL = 0b01101011;
    char maskClearForU = 0b00011111;
    char tus, tls, trs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tls = (char)(_bytes[1] - mud);
        trs = (char)(_bytes[3] - mud);
        tds = (char)(_bytes[5] - mud);

        _bytes[1] = (char)(tls & maskClearForL | tus << 2 & 0b10000000 | tus >> 2 & 0b00010000 | tus >> 5 & 0b00000100);

        _bytes[5] = (char)(tds & maskClearForD | tls >> 5 & 0b00000100 | tls >> 3 & 0b00000010 | tls >> 2 & 0b00000001);

        _bytes[3] = (char)(trs & maskClearForR | tds >> 2 & 0b00000001 | tds << 2 & 0b00001000 | tds << 5 & 0b00100000);

        _bytes[0] = (char)(tus & maskClearForU | trs << 5 & 0b00100000 | trs << 3 & 0b01000000 | trs << 2 & 0b10000000);
    }
    else
    {
        tus = _bytes[0];
        tls = _bytes[1];
        trs = _bytes[3];
        tds = _bytes[5];

        _bytes[1] = (char)((tls & maskClearForL | tus << 2 & 0b10000000 | tus >> 2 & 0b00010000 | tus >> 5 & 0b00000100) + mud);

        _bytes[5] = (char)((tds & maskClearForD | tls >> 5 & 0b00000100 | tls >> 3 & 0b00000010 | tls >> 2 & 0b00000001) + mud);

        _bytes[3] = (char)((trs & maskClearForR | tds >> 2 & 0b00000001 | tds << 2 & 0b00001000 | tds << 5 & 0b00100000) + mud);

        _bytes[0] = (char)((tus & maskClearForU | trs << 5 & 0b00100000 | trs << 3 & 0b01000000 | trs << 2 & 0b10000000) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(2);
}

void Cube1Bit::AntiB()
{
    char maskClearForR = 0b01101011;
    char maskClearForD = 0b00011111;
    char maskClearForL = 0b11010110;
    char maskClearForU = 0b11111000;
    char tus, tls, trs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tls = (char)(_bytes[1] - mud);
        trs = (char)(_bytes[3] - mud);
        tds = (char)(_bytes[5] - mud);
        _bytes[3] = (char)(trs & maskClearForR | tus << 5 & 0b10000000 | tus << 3 & 0b00010000 | tus << 2 & 0b00000100);

        _bytes[5] = (char)(tds & maskClearForD | trs >> 2 & 0b00100000 | trs << 2 & 0b01000000 | trs << 5 & 0b10000000);

        _bytes[1] = (char)(tls & maskClearForL | tds >> 5 & 0b00000001 | tds >> 3 & 0b00001000 | tds >> 2 & 0b00100000);

        _bytes[0] = (char)(tus & maskClearForU | tls << 2 & 0b00000100 | tls >> 2 & 0b00000010 | tls >> 5 & 0b00000001);
    }
    else
    {
        tus = _bytes[0];
        tls = _bytes[1];
        trs = _bytes[3];
        tds = _bytes[5];
        _bytes[3] = (char)((trs & maskClearForR | tus << 5 & 0b10000000 | tus << 3 & 0b00010000 | tus << 2 & 0b00000100) + mud);

        _bytes[5] = (char)((tds & maskClearForD | trs >> 2 & 0b00100000 | trs << 2 & 0b01000000 | trs << 5 & 0b10000000) + mud);

        _bytes[1] = (char)((tls & maskClearForL | tds >> 5 & 0b00000001 | tds >> 3 & 0b00001000 | tds >> 2 & 0b00100000) + mud);

        _bytes[0] = (char)((tus & maskClearForU | tls << 2 & 0b00000100 | tls >> 2 & 0b00000010 | tls >> 5 & 0b00000001) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(4);
}

void Cube1Bit::AntiU()
{
    char maskClearForRFL = 0b11111000;
    char maskClearForB = 0b00011111;
    char trs, tls, tfs, tbs;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        trs = (char)(_bytes[3] - mud);
        tls = (char)(_bytes[1] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        _bytes[1] = (char)(tls & maskClearForRFL | tbs >> 3 & 0b00000100 | tbs >> 5 & 0b00000010 | tbs >> 7 & 0b00000001);

        _bytes[2] = (char)(tfs & maskClearForRFL | tls & 0b00000111);

        _bytes[3] = (char)(trs & maskClearForRFL | tfs & 0b00000111);

        _bytes[4] = (char)(tbs & maskClearForB | trs << 3 & 0b00100000 | trs << 5 & 0b01000000 | trs << 7 & 0b10000000);
    }
    else
    {
        trs = _bytes[3];
        tls = _bytes[1];
        tfs = _bytes[2];
        tbs = _bytes[4];
        _bytes[1] = (char)((tls & maskClearForRFL | tbs >> 3 & 0b00000100 | tbs >> 5 & 0b00000010 | tbs >> 7 & 0b00000001) + mud);

        _bytes[2] = (char)((tfs & maskClearForRFL | tls & 0b00000111) + mud);

        _bytes[3] = (char)((trs & maskClearForRFL | tfs & 0b00000111) + mud);

        _bytes[4] = (char)((tbs & maskClearForB | trs << 3 & 0b00100000 | trs << 5 & 0b01000000 | trs << 7 & 0b10000000) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(0);
}

void Cube1Bit::AntiD()
{
    char maskClearForRFL = 0b00011111;
    char maskClearForB = 0b11111000;
    char trs, tls, tfs, tbs;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        trs = (char)(_bytes[3] - mud);
        tls = (char)(_bytes[1] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        _bytes[1] = (char)(tls & maskClearForRFL | tfs & 0b11100000);

        _bytes[4] = (char)(tbs & maskClearForB | tls >> 3 & 0b00000100 | tls >> 5 & 0b00000010 | tls >> 7 & 0b00000001);

        _bytes[3] = (char)(trs & maskClearForRFL | tbs << 3 & 0b00100000 | tbs << 5 & 0b01000000 | tbs << 7 & 0b10000000);

        _bytes[2] = (char)(tfs & maskClearForRFL | trs & 0b11100000);
    }
    else
    {
        trs = _bytes[3];
        tls = _bytes[1];
        tfs = _bytes[2];
        tbs = _bytes[4];
        _bytes[1] = (char)((tls & maskClearForRFL | tfs & 0b11100000) + mud);

        _bytes[4] = (char)((tbs & maskClearForB | tls >> 3 & 0b00000100 | tls >> 5 & 0b00000010 | tls >> 7 & 0b00000001) + mud);

        _bytes[3] = (char)((trs & maskClearForRFL | tbs << 3 & 0b00100000 | tbs << 5 & 0b01000000 | tbs << 7 & 0b10000000) + mud);

        _bytes[2] = (char)((tfs & maskClearForRFL | trs & 0b11100000) + mud);
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(5);
}

void Cube1Bit::M()
{
    char maskClear = 0b10111101;
    char maskGet = 0b01000010;
    char tus, tfs, tbs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        tds = (char)(_bytes[5] - mud);

        _bytes[0] = (char)(tus & maskClear | tbs & maskGet);

        _bytes[4] = (char)(tbs & maskClear | tds & maskGet);

        _bytes[5] = (char)(tds & maskClear | tfs & maskGet);

        _bytes[2] = (char)(tfs & maskClear | tus & maskGet);
    }
    else
    {
        tus = _bytes[0];
        tfs = _bytes[2];
        tbs = _bytes[4];
        tds = _bytes[5];

        _bytes[0] = (char)((tus & maskClear | tbs & maskGet) + mud);

        _bytes[4] = (char)((tbs & maskClear | tds & maskGet) + mud);

        _bytes[5] = (char)((tds & maskClear | tfs & maskGet) + mud);

        _bytes[2] = (char)((tfs & maskClear | tus & maskGet) + mud);
    }
    _numIndex += _offset;
}

void Cube1Bit::AntiM()
{
    char maskClear = 0b10111101;
    char maskGet = 0b01000010;
    char tus, tfs, tbs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        tds = (char)(_bytes[5] - mud);

        _bytes[0] = (char)(tus & maskClear | tfs & maskGet);

        _bytes[4] = (char)(tbs & maskClear | tus & maskGet);

        _bytes[5] = (char)(tds & maskClear | tbs & maskGet);

        _bytes[2] = (char)(tfs & maskClear | tds & maskGet);
    }
    else
    {
        tus = _bytes[0];
        tfs = _bytes[2];
        tbs = _bytes[4];
        tds = _bytes[5];

        _bytes[0] = (char)((tus & maskClear | tfs & maskGet) + mud);

        _bytes[4] = (char)((tbs & maskClear | tus & maskGet) + mud);

        _bytes[5] = (char)((tds & maskClear | tbs & maskGet) + mud);

        _bytes[2] = (char)((tfs & maskClear | tds & maskGet) + mud);
    }
    _numIndex += _offset;
}

void Cube1Bit::S()
{
    char maskClearUD = 0b11100111;
    char maskClearRL = 0b10111101;
    char tus, tls, trs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tls = (char)(_bytes[1] - mud);
        trs = (char)(_bytes[3] - mud);
        tds = (char)(_bytes[5] - mud);

        _bytes[0] = (char)(tus & maskClearUD | tls << 3 & 0b00010000 | tls >> 3 & 0b00001000);

        _bytes[3] = (char)(trs & maskClearRL | tus << 2 & 0b01000000 | tus >> 2 & 0b00000010);

        _bytes[5] = (char)(tds & maskClearUD | trs >> 3 & 0b00001000 | trs << 3 & 0b00010000);

        _bytes[1] = (char)(tls & maskClearRL | tds >> 2 & 0b00000010 | tds << 2 & 0b01000000);
    }
    else
    {
        tus = _bytes[0];
        tls = _bytes[1];
        trs = _bytes[3];
        tds = _bytes[5];

        _bytes[0] = (char)((tus & maskClearUD | tls << 3 & 0b00010000 | tls >> 3 & 0b00001000) + mud);

        _bytes[3] = (char)((trs & maskClearRL | tus << 2 & 0b01000000 | tus >> 2 & 0b00000010) + mud);

        _bytes[5] = (char)((tds & maskClearUD | trs >> 3 & 0b00001000 | trs << 3 & 0b00010000) + mud);

        _bytes[1] = (char)((tls & maskClearRL | tds >> 2 & 0b00000010 | tds << 2 & 0b01000000) + mud);
    }
    _numIndex += _offset;
}

void Cube1Bit::AntiS()
{
    char maskClearUD = 0b11100111;
    char maskClearRL = 0b10111101;
    char tus, tls, trs, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = (char)(_bytes[0] - mud);
        tls = (char)(_bytes[1] - mud);
        trs = (char)(_bytes[3] - mud);
        tds = (char)(_bytes[5] - mud);
        _bytes[0] = (char)(tus & maskClearUD | trs << 2 & 0b00001000 | trs >> 2 & 0b00010000);

        _bytes[1] = (char)(tls & maskClearRL | tus << 3 & 0b01000000 | tus >> 3 & 0b00000010);

        _bytes[5] = (char)(tds & maskClearUD | tls >> 2 & 0b00010000 | tls << 2 & 0b00001000);

        _bytes[3] = (char)(trs & maskClearRL | tds >> 3 & 0b00000010 | tds << 3 & 0b01000000);
    }
    else
    {
        tus = _bytes[0];
        tls = _bytes[1];
        trs = _bytes[3];
        tds = _bytes[5];
        _bytes[0] = (char)((tus & maskClearUD | trs << 2 & 0b00001000 | trs >> 2 & 0b00010000) + mud);

        _bytes[1] = (char)((tls & maskClearRL | tus << 3 & 0b01000000 | tus >> 3 & 0b00000010) + mud);

        _bytes[5] = (char)((tds & maskClearUD | tls >> 2 & 0b00010000 | tls << 2 & 0b00001000) + mud);

        _bytes[3] = (char)((trs & maskClearRL | tds >> 3 & 0b00000010 | tds << 3 & 0b01000000) + mud);
    }
    _numIndex += _offset;
}

void Cube1Bit::E()
{
    char maskClear = 0b11100111;
    char maskGetRFL = 0b00011000;
    char trs, tls, tfs, tbs;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        trs = (char)(_bytes[3] - mud);
        tls = (char)(_bytes[1] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);

        _bytes[2] = (char)(tfs & maskClear | tls & maskGetRFL);

        _bytes[3] = (char)(trs & maskClear | tfs & maskGetRFL);

        _bytes[4] = (char)(tbs & maskClear | trs >> 1 & 0b00001000 | trs << 1 & 0b00010000);

        _bytes[1] = (char)(tls & maskClear | tbs << 1 & 0b00010000 | tbs >> 1 & 0b00001000);
    }
    else
    {
        trs = _bytes[3];
        tls = _bytes[1];
        tfs = _bytes[2];
        tbs = _bytes[4];

        _bytes[2] = (char)((tfs & maskClear | tls & maskGetRFL) + mud);

        _bytes[3] = (char)((trs & maskClear | tfs & maskGetRFL) + mud);

        _bytes[4] = (char)((tbs & maskClear | trs >> 1 & 0b00001000 | trs << 1 & 0b00010000) + mud);

        _bytes[1] = (char)((tls & maskClear | tbs << 1 & 0b00010000 | tbs >> 1 & 0b00001000) + mud);
    }
    _numIndex += _offset;
}

void Cube1Bit::AntiE()
{
    char maskClear = 0b11100111;
    char maskGetRFL = 0b00011000;
    char trs, tls, tfs, tbs;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        trs = (char)(_bytes[3] - mud);
        tls = (char)(_bytes[1] - mud);
        tfs = (char)(_bytes[2] - mud);
        tbs = (char)(_bytes[4] - mud);
        _bytes[2] = (char)(tfs & maskClear | trs & maskGetRFL);

        _bytes[1] = (char)(tls & maskClear | tfs & maskGetRFL);

        _bytes[4] = (char)(tbs & maskClear | tls << 1 & 0b00010000 | tls >> 1 & 0b00001000);

        _bytes[3] = (char)(trs & maskClear | tbs >> 1 & 0b00001000 | tbs << 1 & 0b00010000);
    }
    else
    {
        trs = _bytes[3];
        tls = _bytes[1];
        tfs = _bytes[2];
        tbs = _bytes[4];
        _bytes[2] = (char)((tfs & maskClear | trs & maskGetRFL) + mud);

        _bytes[1] = (char)((tls & maskClear | tfs & maskGetRFL) + mud);

        _bytes[4] = (char)((tbs & maskClear | tls << 1 & 0b00010000 | tls >> 1 & 0b00001000) + mud);

        _bytes[3] = (char)((trs & maskClear | tbs >> 1 & 0b00001000 | tbs << 1 & 0b00010000) + mud);
    }
    _numIndex += _offset;
}

void Cube1Bit::C()
{
    char maskClear = 0b01011010;
    char trs, tls, tfs, tbs, tus, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = _bytes[0];
        tls = _bytes[1];
        tfs = (char)(_bytes[2] - mud);
        trs = _bytes[3];
        tbs = (char)(_bytes[4] - mud);
        tds = _bytes[5];

        _bytes[2] = (char)(tfs & maskClear | tus >> 5 & 0b00000101 | tds << 5 & 0b10100000);

        _bytes[0] = (char)(tus & maskClear | tls & 0b00000001 | tls << 3 & 0b00100000
            | trs & 0b00000100 | trs << 7 & 0b10000000);

        _bytes[4] = (char)(tbs & maskClear | tds >> 5 & 0b00000101 | tus << 5 & 0b10100000);

        _bytes[5] = (char)(tds & maskClear | trs & 0b10000000 | trs >> 3 & 0b00000100
            | tls & 0b00100000 | tls >> 7 & 0b00000001);

        _bytes[1] = (char)(tls & maskClear | tbs >> 5 & 0b00000001 | tbs << 5 & 0b00100000
            | tfs << 2 & 0b10000100);

        _bytes[3] = (char)(trs & maskClear | tbs >> 5 & 0b00000100 | tbs << 5 & 0b10000000
            | tfs >> 2 & 0b00100001);
    }
    else
    {
        tus = _bytes[0];
        tls = _bytes[1];
        tfs = _bytes[2];
        trs = _bytes[3];
        tbs = _bytes[4];
        tds = _bytes[5];

        _bytes[2] = (char)((tfs & maskClear | tus >> 5 & 0b00000101 | tds << 5 & 0b10100000) + mud);//

        _bytes[0] = (char)(tus & maskClear | tls & 0b00000001 | tls << 3 & 0b00100000
            | trs & 0b00000100 | trs << 7 & 0b10000000);//

        _bytes[4] = (char)((tbs & maskClear | tds >> 5 & 0b00000101 | tus << 5 & 0b10100000) + mud);//

        _bytes[5] = (char)(tds & maskClear | trs & 0b10000000 | trs >> 3 & 0b00000100
            | tls & 0b00100000 | tls >> 7 & 0b00000001);//

        _bytes[1] = (char)(tls & maskClear | tbs >> 5 & 0b00000001 | tbs << 5 & 0b00100000
            | tfs << 2 & 0b10000100);

        _bytes[3] = (char)(trs & maskClear | tbs >> 5 & 0b00000100 | tbs << 5 & 0b10000000
            | tfs >> 2 & 0b00100001);
    }
    _numIndex += _offset;
}

void Cube1Bit::AntiC()
{
    char maskClear = 0b01011010;
    char trs, tls, tfs, tbs, tus, tds;
    char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        tus = _bytes[0];
        tls = _bytes[1];
        tfs = (char)(_bytes[2] - mud);
        trs = _bytes[3];
        tbs = (char)(_bytes[4] - mud);
        tds = _bytes[5];

        _bytes[2] = (char)(tfs & maskClear | trs << 2 & 0b10000100 | tls >> 2 & 0b00100001);

        _bytes[0] = (char)(tus & maskClear | tbs >> 5 & 0b00000101 | tfs << 5 & 0b10100000);

        _bytes[4] = (char)(tbs & maskClear | trs << 5 & 0b10000000 | trs >> 5 & 0b00000100
            | tls >> 5 & 0b00000001 | tls << 5 & 0b00100000);

        _bytes[5] = (char)(tds & maskClear | tfs >> 5 & 0b00000101 | tbs << 5 & 0b10100000);

        _bytes[1] = (char)(tls & maskClear | tus & 0b00000001 | tus >> 3 & 0b00000100
            | tds & 0b00100000 | tds << 7 & 0b10000000);

        _bytes[3] = (char)(trs & maskClear | tds & 0b10000000 | tds << 3 & 0b00100000
            | tus & 0b00000100 | tus >> 7 & 0b00000001);
    }
    else
    {
        tus = _bytes[0];
        tls = _bytes[1];
        tfs = _bytes[2];
        trs = _bytes[3];
        tbs = _bytes[4];
        tds = _bytes[5];

        _bytes[2] = (char)((tfs & maskClear | trs << 2 & 0b10000100 | tls >> 2 & 0b00100001) + mud);//

        _bytes[0] = (char)(tus & maskClear | tbs >> 5 & 0b00000101 | tfs << 5 & 0b10100000);//

        _bytes[4] = (char)((tbs & maskClear | trs << 5 & 0b10000000 | trs >> 5 & 0b00000100
            | tls >> 5 & 0b00000001 | tls << 5 & 0b00100000) + mud);//

        _bytes[5] = (char)(tds & maskClear | tfs >> 5 & 0b00000101 | tbs << 5 & 0b10100000);//

        _bytes[1] = (char)(tls & maskClear | tus & 0b00000001 | tus >> 3 & 0b00000100
            | tds & 0b00100000 | tds << 7 & 0b10000000);//

        _bytes[3] = (char)(trs & maskClear | tds & 0b10000000 | tds << 3 & 0b00100000
            | tus & 0b00000100 | tus >> 7 & 0b00000001);
    }
    _numIndex += _offset;
}

void Cube1Bit::MoveSideClockwise(int sideIndex)
{
    char ts = _bytes[sideIndex];
    _bytes[sideIndex] = (char)(ts << 2 & 0b01000100 | ts << 3 & 0b00010000 | ts << 5 & 0b10000000
        | ts >> 2 & 0b00100010 | ts >> 3 & 0b00001000 | ts >> 5 & 0b00000001);
}

void Cube1Bit::MoveSideCounterClockwise(int sideIndex)
{
    char ts = _bytes[sideIndex];
    _bytes[sideIndex] = (char)(ts >> 2 & 0b00010001 | ts >> 3 & 0b00000010 | ts >> 5 & 0b00000100
        | ts << 2 & 0b10001000 | ts << 3 & 0b01000000 | ts << 5 & 0b00100000);
}