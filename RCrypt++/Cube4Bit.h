#pragma once
#include <string>
#include <vector>
#include "CubeBytes.h"

class Cube4Bit
{
public:
    Cube4Bit(bool decrypt, unsigned long long seed, std::string scramble);

    ~Cube4Bit();

    void Init(CubeBytes* bytes);

    char* DoScramble();

    void DoScramble(char* bytes);

    static const int blockSize = 24;

private:
	CubeBytes _bytes;
	int _offset;
	char _resultBytes[blockSize];
	std::vector<void(Cube4Bit::*)()> _moves;
	bool _decryptMode;
	//private Random _rnd;
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

	void MoveSideClockwise(SideBytes *side);

	void MoveSideCounterClockwise(SideBytes *side);
};

