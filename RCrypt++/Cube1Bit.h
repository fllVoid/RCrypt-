#pragma once
#include <string>
#include <vector>

class Cube1Bit
{
public:
    Cube1Bit(bool decrypt, unsigned long long seed, std::string scramble);
    ~Cube1Bit();
    void Init(char* bytes);
    char* DoScramble();
    static const int blockSize = 6;

private:
    char *_bytes;
    int _offset;
    std::vector<void(Cube1Bit::*)()> _moves;
    bool _decryptMode;
    char* _nums;
    char _tmpBytes[blockSize];
    int _numIndex;
    int _startNumIndex;
    std::string _scramble;

    void SetScramble(std::string scramble);

    void SaveBytes(char* bytes);

    void InitFirstVector(unsigned long long seed);

    void R();

    void L();

    void F();

    void B();

    void U();

    void D();

    void AntiR();

    void AntiL();

    void AntiF();

    void AntiB();

    void AntiU();

    void AntiD();

    void M();

    void AntiM();

    void S();

    void AntiS();

    void E();

    void AntiE();

    void C();

    void AntiC();

    void MoveSideClockwise(int sideIndex);

    void MoveSideCounterClockwise(int sideIndex);
};

