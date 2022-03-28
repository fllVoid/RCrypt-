#include <functional>
#include <map>
#include "Cube4Bit.h"
#include "ScrambleHelper.h"

using namespace std;
typedef std::function<void()> Action;

Cube4Bit::Cube4Bit(bool decrypt, unsigned long long seed, string scramble)
{
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
}

Cube4Bit::~Cube4Bit() {
    if (_nums)
        delete[] _nums;
}

void Cube4Bit::Init(CubeBytes* bytes)
{
	_bytes = *bytes;
}

char* Cube4Bit::DoScramble()
{
	_numIndex = _startNumIndex;
	int count = _moves.size();
	if (_decryptMode)
		SaveBytes(_tmpBytes);
	for (int i = 0; i < count; ++i)
	{
		(this->*_moves[i])();
	}
	SaveBytes(_resultBytes);
	char* tmpArr = _decryptMode ? _tmpBytes : _resultBytes;
	for (int i = 0; i < _moves.size(); ++i)
		_nums[i] = tmpArr[i % blockSize];
	return _resultBytes;
}

void Cube4Bit::DoScramble(char* bytes)
{
	_numIndex = _startNumIndex;
	int count = _moves.size();
	if (_decryptMode)
		SaveBytes(_tmpBytes);
	for (int i = 0; i < count; ++i)
	{
		(this->*_moves[i])();
	}
	SaveBytes(bytes);
	char* tmpArr = _decryptMode ? _tmpBytes : bytes;
	//rewrite vector
	for (int i = 0; i < _moves.size(); ++i)
		_nums[i] = tmpArr[i % blockSize];
}

void Cube4Bit::SetScramble(string scramble)
{
    //scramble = "R";
	_moves.clear();
	int i = 0;
	auto addMove = [&](void(Cube4Bit::* move)(), void(Cube4Bit::* antiMove)())
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

	dict['R'] = [&]() {addMove(&Cube4Bit::R, &Cube4Bit::AntiR); };
	dict['L'] = [&]() {addMove(&Cube4Bit::L, &Cube4Bit::AntiL); };
	dict['U'] = [&]() {addMove(&Cube4Bit::U, &Cube4Bit::AntiU); };
	dict['D'] = [&]() {addMove(&Cube4Bit::D, &Cube4Bit::AntiD); };
	dict['F'] = [&]() {addMove(&Cube4Bit::F, &Cube4Bit::AntiF); };
	dict['B'] = [&]() {addMove(&Cube4Bit::B, &Cube4Bit::AntiB); };
	dict['S'] = [&]() {addMove(&Cube4Bit::S, &Cube4Bit::AntiS); };
	dict['E'] = [&]() {addMove(&Cube4Bit::E, &Cube4Bit::AntiE); };
	dict['M'] = [&]() {addMove(&Cube4Bit::M, &Cube4Bit::AntiM); };
	dict['C'] = [&]() {addMove(&Cube4Bit::C, &Cube4Bit::AntiC); };
	for (i = 0; i < scramble.length(); ++i)
	{
		if (dict.count(scramble[i]))
		{
			dict[scramble[i]]();
		}
	}
}

void Cube4Bit::SaveBytes(char* bytes)
{
	bytes[0] = _bytes.U.b1;
	bytes[1] = _bytes.U.b2;
	bytes[2] = _bytes.U.b3;
	bytes[3] = _bytes.U.b4;

	bytes[4] = _bytes.L.b1;
	bytes[5] = _bytes.L.b2;
	bytes[6] = _bytes.L.b3;
	bytes[7] = _bytes.L.b4;

	bytes[8] = _bytes.F.b1;
	bytes[9] = _bytes.F.b2;
	bytes[10] = _bytes.F.b3;
	bytes[11] = _bytes.F.b4;

	bytes[12] = _bytes.R.b1;
	bytes[13] = _bytes.R.b2;
	bytes[14] = _bytes.R.b3;
	bytes[15] = _bytes.R.b4;

	bytes[16] = _bytes.B.b1;
	bytes[17] = _bytes.B.b2;
	bytes[18] = _bytes.B.b3;
	bytes[19] = _bytes.B.b4;

	bytes[20] = _bytes.D.b1;
	bytes[21] = _bytes.D.b2;
	bytes[22] = _bytes.D.b3;
	bytes[23] = _bytes.D.b4;
}

void Cube4Bit::InitFirstVector(unsigned long long seed)
{
	delete[] _nums;
    _nums = new char[_moves.size()]{ 0 };
    char* rndSequence = new char[blockSize] {0};
	for (int i = 0; i < blockSize; ++i)
	{
		rndSequence[i] = (char)(seed % 256);
		seed = seed / 256 * 2333;
	}
	SideBytes u{ rndSequence[0], rndSequence[1], rndSequence[2], rndSequence[3] };
	SideBytes l{ rndSequence[4], rndSequence[5], rndSequence[6], rndSequence[7] };
	SideBytes f{ rndSequence[8], rndSequence[9], rndSequence[10], rndSequence[11] };
	SideBytes r{ rndSequence[12], rndSequence[13], rndSequence[14], rndSequence[15] };
	SideBytes b{ rndSequence[16], rndSequence[17], rndSequence[18], rndSequence[19] };
	SideBytes d{ rndSequence[20], rndSequence[21], rndSequence[22], rndSequence[23] };
	_bytes.U = u;
	_bytes.L = l;
	_bytes.F = f;
	_bytes.R = r;
	_bytes.B = b;
	_bytes.D = d;
	DoScramble();
	delete[] rndSequence;
}

void Cube4Bit::R()
{
    SideBytes tus;
    SideBytes tfs;
    SideBytes tbs;
    SideBytes tds;
    unsigned char mud = _nums[_numIndex];
    if (_decryptMode)
    {
        _bytes.U.b2 -= mud;
        _bytes.U.b3 -= mud;
        _bytes.U.b4 -= mud;

        _bytes.F.b2 -= mud;
        _bytes.F.b3 -= mud;
        _bytes.F.b4 -= mud;

        _bytes.B.b2 -= mud;
        _bytes.B.b3 -= mud;
        _bytes.B.b4 -= mud;

        _bytes.D.b2 -= mud;
        _bytes.D.b3 -= mud;
        _bytes.D.b4 -= mud;
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;

        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b3 &= 0b11110000;
        _bytes.U.b4 &= 0b00001111;
        _bytes.U.b2 |= (unsigned char)(tfs.b2 & 0b00001111);
        _bytes.U.b3 |= (unsigned char)(tfs.b3 & 0b00001111);
        _bytes.U.b4 |= (unsigned char)(tfs.b4 & 0b11110000);

        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b3 &= 0b11110000;
        _bytes.B.b4 &= 0b00001111;
        _bytes.B.b2 |= (unsigned char)(tus.b2 & 0b00001111);
        _bytes.B.b3 |= (unsigned char)(tus.b3 & 0b00001111);
        _bytes.B.b4 |= (unsigned char)(tus.b4 & 0b11110000);

        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b3 &= 0b11110000;
        _bytes.D.b4 &= 0b00001111;
        _bytes.D.b2 |= (unsigned char)(tbs.b2 & 0b00001111);
        _bytes.D.b3 |= (unsigned char)(tbs.b3 & 0b00001111);
        _bytes.D.b4 |= (unsigned char)(tbs.b4 & 0b11110000);

        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b3 &= 0b11110000;
        _bytes.F.b4 &= 0b00001111;
        _bytes.F.b2 |= (unsigned char)(tds.b2 & 0b00001111);
        _bytes.F.b3 |= (unsigned char)(tds.b3 & 0b00001111);
        _bytes.F.b4 |= (unsigned char)(tds.b4 & 0b11110000);
    }
    else
    {
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b3 &= 0b11110000;
        _bytes.U.b4 &= 0b00001111;
        _bytes.U.b2 |= (unsigned char)(tfs.b2 & 0b00001111);
        _bytes.U.b3 |= (unsigned char)(tfs.b3 & 0b00001111);
        _bytes.U.b4 |= (unsigned char)(tfs.b4 & 0b11110000);
        _bytes.U.b2 += mud;
        _bytes.U.b3 += mud;
        _bytes.U.b4 += mud;

        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b3 &= 0b11110000;
        _bytes.B.b4 &= 0b00001111;
        _bytes.B.b2 |= (unsigned char)(tus.b2 & 0b00001111);
        _bytes.B.b3 |= (unsigned char)(tus.b3 & 0b00001111);
        _bytes.B.b4 |= (unsigned char)(tus.b4 & 0b11110000);
        _bytes.B.b2 += mud;
        _bytes.B.b3 += mud;
        _bytes.B.b4 += mud;

        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b3 &= 0b11110000;
        _bytes.D.b4 &= 0b00001111;
        _bytes.D.b2 |= (unsigned char)(tbs.b2 & 0b00001111);
        _bytes.D.b3 |= (unsigned char)(tbs.b3 & 0b00001111);
        _bytes.D.b4 |= (unsigned char)(tbs.b4 & 0b11110000);
        _bytes.D.b2 += mud;
        _bytes.D.b3 += mud;
        _bytes.D.b4 += mud;

        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b3 &= 0b11110000;
        _bytes.F.b4 &= 0b00001111;
        _bytes.F.b2 |= (unsigned char)(tds.b2 & 0b00001111);
        _bytes.F.b3 |= (unsigned char)(tds.b3 & 0b00001111);
        _bytes.F.b4 |= (unsigned char)(tds.b4 & 0b11110000);
        _bytes.F.b2 += mud;
        _bytes.F.b3 += mud;
        _bytes.F.b4 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(&_bytes.R);
}

void Cube4Bit::L()
{
    SideBytes tus;
    SideBytes tfs;
    SideBytes tbs;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b1 -= mud;
        _bytes.U.b2 -= mud;
        _bytes.U.b3 -= mud;

        _bytes.F.b1 -= mud;
        _bytes.F.b2 -= mud;
        _bytes.F.b3 -= mud;

        _bytes.B.b1 -= mud;
        _bytes.B.b2 -= mud;
        _bytes.B.b3 -= mud;

        _bytes.D.b1 -= mud;
        _bytes.D.b2 -= mud;
        _bytes.D.b3 -= mud;
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;

        _bytes.U.b1 &= 0b11110000;
        _bytes.U.b2 &= 0b00001111;
        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b1 |= (unsigned char)(tbs.b1 & 0b00001111);
        _bytes.U.b2 |= (unsigned char)(tbs.b2 & 0b11110000);
        _bytes.U.b3 |= (unsigned char)(tbs.b3 & 0b11110000);

        _bytes.F.b1 &= 0b11110000;
        _bytes.F.b2 &= 0b00001111;
        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b1 |= (unsigned char)(tus.b1 & 0b00001111);
        _bytes.F.b2 |= (unsigned char)(tus.b2 & 0b11110000);
        _bytes.F.b3 |= (unsigned char)(tus.b3 & 0b11110000);

        _bytes.D.b1 &= 0b11110000;
        _bytes.D.b2 &= 0b00001111;
        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b1 |= (unsigned char)(tfs.b1 & 0b00001111);
        _bytes.D.b2 |= (unsigned char)(tfs.b2 & 0b11110000);
        _bytes.D.b3 |= (unsigned char)(tfs.b3 & 0b11110000);

        _bytes.B.b1 &= 0b11110000;
        _bytes.B.b2 &= 0b00001111;
        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b1 |= (unsigned char)(tds.b1 & 0b00001111);
        _bytes.B.b2 |= (unsigned char)(tds.b2 & 0b11110000);
        _bytes.B.b3 |= (unsigned char)(tds.b3 & 0b11110000);
    }
    else
    {
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;
        _bytes.U.b1 &= 0b11110000;
        _bytes.U.b2 &= 0b00001111;
        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b1 |= (unsigned char)(tbs.b1 & 0b00001111);
        _bytes.U.b2 |= (unsigned char)(tbs.b2 & 0b11110000);
        _bytes.U.b3 |= (unsigned char)(tbs.b3 & 0b11110000);
        _bytes.U.b1 += mud;
        _bytes.U.b2 += mud;
        _bytes.U.b3 += mud;

        _bytes.F.b1 &= 0b11110000;
        _bytes.F.b2 &= 0b00001111;
        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b1 |= (unsigned char)(tus.b1 & 0b00001111);
        _bytes.F.b2 |= (unsigned char)(tus.b2 & 0b11110000);
        _bytes.F.b3 |= (unsigned char)(tus.b3 & 0b11110000);
        _bytes.F.b1 += mud;
        _bytes.F.b2 += mud;
        _bytes.F.b3 += mud;

        _bytes.D.b1 &= 0b11110000;
        _bytes.D.b2 &= 0b00001111;
        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b1 |= (unsigned char)(tfs.b1 & 0b00001111);
        _bytes.D.b2 |= (unsigned char)(tfs.b2 & 0b11110000);
        _bytes.D.b3 |= (unsigned char)(tfs.b3 & 0b11110000);
        _bytes.D.b1 += mud;
        _bytes.D.b2 += mud;
        _bytes.D.b3 += mud;

        _bytes.B.b1 &= 0b11110000;
        _bytes.B.b2 &= 0b00001111;
        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b1 |= (unsigned char)(tds.b1 & 0b00001111);
        _bytes.B.b2 |= (unsigned char)(tds.b2 & 0b11110000);
        _bytes.B.b3 |= (unsigned char)(tds.b3 & 0b11110000);
        _bytes.B.b1 += mud;
        _bytes.B.b2 += mud;
        _bytes.B.b3 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(&_bytes.L);
}

void Cube4Bit::F()
{
    SideBytes tus;
    SideBytes trs;
    SideBytes tds;
    SideBytes tls;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b3 -= mud;
        _bytes.U.b4 -= mud;

        _bytes.R.b1 -= mud;
        _bytes.R.b2 -= mud;
        _bytes.R.b3 -= mud;

        _bytes.L.b2 -= mud;
        _bytes.L.b3 -= mud;
        _bytes.L.b4 -= mud;

        _bytes.D.b1 -= mud;
        _bytes.D.b2 -= mud;
        tus = _bytes.U;
        trs = _bytes.R;
        tds = _bytes.D;
        tls = _bytes.L;

        _bytes.R.b1 &= 0b11110000;
        _bytes.R.b2 &= 0b00001111;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b1 |= (unsigned char)(tus.b3 >> 4 & 0b00001111);
        _bytes.R.b2 |= (unsigned char)(tus.b4 << 4 & 0b11110000);
        _bytes.R.b3 |= (unsigned char)(tus.b4 & 0b11110000);

        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b1 &= 0b00000000;
        _bytes.D.b2 |= (unsigned char)(trs.b1 & 0b00001111);
        _bytes.D.b1 |= (unsigned char)(trs.b2 & 0b11110000);
        _bytes.D.b1 |= (unsigned char)(trs.b3 >> 4 & 0b00001111);

        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b3 &= 0b11110000;
        _bytes.L.b4 &= 0b00001111;
        _bytes.L.b2 |= (unsigned char)(tds.b1 & 0b00001111);
        _bytes.L.b3 |= (unsigned char)(tds.b1 >> 4 & 0b00001111);
        _bytes.L.b4 |= (unsigned char)(tds.b2 << 4 & 0b11110000);

        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b4 &= 0b00000000;
        _bytes.U.b3 |= (unsigned char)(tls.b4 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(tls.b3 & 0b00001111);
        _bytes.U.b4 |= (unsigned char)(tls.b2 << 4 & 0b11110000);
    }
    else
    {
        tus = _bytes.U;
        trs = _bytes.R;
        tds = _bytes.D;
        tls = _bytes.L;
        _bytes.R.b1 &= 0b11110000;
        _bytes.R.b2 &= 0b00001111;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b1 |= (unsigned char)(tus.b3 >> 4 & 0b00001111);
        _bytes.R.b2 |= (unsigned char)(tus.b4 << 4 & 0b11110000);
        _bytes.R.b3 |= (unsigned char)(tus.b4 & 0b11110000);
        _bytes.R.b1 += mud;
        _bytes.R.b2 += mud;
        _bytes.R.b3 += mud;

        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b1 &= 0b00000000;
        _bytes.D.b2 |= (unsigned char)(trs.b1 & 0b00001111);
        _bytes.D.b1 |= (unsigned char)(trs.b2 & 0b11110000);
        _bytes.D.b1 |= (unsigned char)(trs.b3 >> 4 & 0b00001111);
        _bytes.D.b1 += mud;
        _bytes.D.b2 += mud;

        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b3 &= 0b11110000;
        _bytes.L.b4 &= 0b00001111;
        _bytes.L.b2 |= (unsigned char)(tds.b1 & 0b00001111);
        _bytes.L.b3 |= (unsigned char)(tds.b1 >> 4 & 0b00001111);
        _bytes.L.b4 |= (unsigned char)(tds.b2 << 4 & 0b11110000);
        _bytes.L.b4 += mud;
        _bytes.L.b2 += mud;
        _bytes.L.b3 += mud;

        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b4 &= 0b00000000;
        _bytes.U.b3 |= (unsigned char)(tls.b4 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(tls.b3 & 0b00001111);
        _bytes.U.b4 |= (unsigned char)(tls.b2 << 4 & 0b11110000);
        _bytes.U.b3 += mud;
        _bytes.U.b4 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(&_bytes.F);
}

void Cube4Bit::B()
{
    SideBytes tus;
    SideBytes trs;
    SideBytes tds;
    SideBytes tls;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.D.b3 -= mud;
        _bytes.D.b4 -= mud;

        _bytes.L.b1 -= mud;
        _bytes.L.b2 -= mud;
        _bytes.L.b3 -= mud;

        _bytes.R.b2 -= mud;
        _bytes.R.b3 -= mud;
        _bytes.R.b4 -= mud;

        _bytes.U.b1 -= mud;
        _bytes.U.b2 -= mud;
        tus = _bytes.U;
        trs = _bytes.R;
        tds = _bytes.D;
        tls = _bytes.L;

        _bytes.L.b1 &= 0b11110000;
        _bytes.L.b2 &= 0b00001111;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b1 |= (unsigned char)(tus.b2 & 0b00001111);
        _bytes.L.b2 |= (unsigned char)(tus.b1 & 0b11110000);
        _bytes.L.b3 |= (unsigned char)(tus.b1 << 4 & 0b11110000);

        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b4 &= 0b00000000;
        _bytes.D.b3 |= (unsigned char)(tls.b1 << 4 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(tls.b2 >> 4 & 0b00001111);
        _bytes.D.b4 |= (unsigned char)(tls.b3 & 0b11110000);

        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b3 &= 0b11110000;
        _bytes.R.b4 &= 0b00001111;
        _bytes.R.b2 |= (unsigned char)(tds.b4 >> 4 & 0b00001111);
        _bytes.R.b3 |= (unsigned char)(tds.b4 & 0b00001111);
        _bytes.R.b4 |= (unsigned char)(tds.b3 & 0b11110000);

        _bytes.U.b1 &= 0b00000000;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b1 |= (unsigned char)(trs.b2 & 0b00001111);
        _bytes.U.b1 |= (unsigned char)(trs.b3 << 4 & 0b11110000);
        _bytes.U.b2 |= (unsigned char)(trs.b4 >> 4 & 0b00001111);
    }
    else
    {
        tus = _bytes.U;
        trs = _bytes.R;
        tds = _bytes.D;
        tls = _bytes.L;
        _bytes.L.b1 &= 0b11110000;
        _bytes.L.b2 &= 0b00001111;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b1 |= (unsigned char)(tus.b2 & 0b00001111);
        _bytes.L.b2 |= (unsigned char)(tus.b1 & 0b11110000);
        _bytes.L.b3 |= (unsigned char)(tus.b1 << 4 & 0b11110000);
        _bytes.L.b1 += mud;
        _bytes.L.b2 += mud;
        _bytes.L.b3 += mud;

        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b4 &= 0b00000000;
        _bytes.D.b3 |= (unsigned char)(tls.b1 << 4 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(tls.b2 >> 4 & 0b00001111);
        _bytes.D.b4 |= (unsigned char)(tls.b3 & 0b11110000);
        _bytes.D.b4 += mud;
        _bytes.D.b3 += mud;

        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b3 &= 0b11110000;
        _bytes.R.b4 &= 0b00001111;
        _bytes.R.b2 |= (unsigned char)(tds.b4 >> 4 & 0b00001111);
        _bytes.R.b3 |= (unsigned char)(tds.b4 & 0b00001111);
        _bytes.R.b4 |= (unsigned char)(tds.b3 & 0b11110000);
        _bytes.R.b4 += mud;
        _bytes.R.b2 += mud;
        _bytes.R.b3 += mud;

        _bytes.U.b1 &= 0b00000000;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b1 |= (unsigned char)(trs.b2 & 0b00001111);
        _bytes.U.b1 |= (unsigned char)(trs.b3 << 4 & 0b11110000);
        _bytes.U.b2 |= (unsigned char)(trs.b4 >> 4 & 0b00001111);
        _bytes.U.b1 += mud;
        _bytes.U.b2 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(&_bytes.B);
}

void Cube4Bit::U()
{
    SideBytes tfs;
    SideBytes tls;
    SideBytes tbs;
    SideBytes trs;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.B.b3 -= mud;
        _bytes.B.b4 -= mud;

        _bytes.R.b1 -= mud;
        _bytes.R.b2 -= mud;

        _bytes.F.b1 -= mud;
        _bytes.F.b2 -= mud;

        _bytes.L.b1 -= mud;
        _bytes.L.b2 -= mud;
        tfs = _bytes.F;
        tls = _bytes.L;
        tbs = _bytes.B;
        trs = _bytes.R;

        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b1 = (unsigned char)(tfs.b1);
        _bytes.L.b2 |= (unsigned char)(tfs.b2 & 0b00001111);

        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b4 &= 0b00000000;
        _bytes.B.b3 |= (unsigned char)(tls.b2 << 4 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(tls.b1 >> 4 & 0b00001111);
        _bytes.B.b4 |= (unsigned char)(tls.b1 << 4 & 0b11110000);

        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b1 &= 0b00000000;
        _bytes.R.b2 |= (unsigned char)(tbs.b3 >> 4 & 0b00001111);
        _bytes.R.b1 |= (unsigned char)(tbs.b4 << 4 & 0b11110000);
        _bytes.R.b1 |= (unsigned char)(tbs.b4 >> 4 & 0b00001111);

        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b1 = (unsigned char)(trs.b1);
        _bytes.F.b2 |= (unsigned char)(trs.b2 & 0b00001111);
    }
    else
    {
        tfs = _bytes.F;
        tls = _bytes.L;
        tbs = _bytes.B;
        trs = _bytes.R;

        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b1 = (unsigned char)(tfs.b1);
        _bytes.L.b2 |= (unsigned char)(tfs.b2 & 0b00001111);
        _bytes.L.b1 += mud;
        _bytes.L.b2 += mud;

        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b4 &= 0b00000000;
        _bytes.B.b3 |= (unsigned char)(tls.b2 << 4 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(tls.b1 >> 4 & 0b00001111);
        _bytes.B.b4 |= (unsigned char)(tls.b1 << 4 & 0b11110000);
        _bytes.B.b3 += mud;
        _bytes.B.b4 += mud;

        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b1 &= 0b00000000;
        _bytes.R.b2 |= (unsigned char)(tbs.b3 >> 4 & 0b00001111);
        _bytes.R.b1 |= (unsigned char)(tbs.b4 << 4 & 0b11110000);
        _bytes.R.b1 |= (unsigned char)(tbs.b4 >> 4 & 0b00001111);
        _bytes.R.b1 += mud;
        _bytes.R.b2 += mud;

        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b1 = (unsigned char)(trs.b1);
        _bytes.F.b2 |= (unsigned char)(trs.b2 & 0b00001111);
        _bytes.F.b1 += mud;
        _bytes.F.b2 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(&_bytes.U);
}

void Cube4Bit::D()
{
    SideBytes tfs;
    SideBytes tls;
    SideBytes tbs;
    SideBytes trs;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.B.b1 -= mud;
        _bytes.B.b2 -= mud;

        _bytes.R.b3 -= mud;
        _bytes.R.b4 -= mud;

        _bytes.F.b3 -= mud;
        _bytes.F.b4 -= mud;

        _bytes.L.b3 -= mud;
        _bytes.L.b4 -= mud;
        tfs = _bytes.F;
        tls = _bytes.L;
        tbs = _bytes.B;
        trs = _bytes.R;

        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b4 &= 0b00000000;
        _bytes.L.b3 |= (unsigned char)(tbs.b2 << 4 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tbs.b1 >> 4 & 0b00001111);
        _bytes.L.b4 |= (unsigned char)(tbs.b1 << 4 & 0b11110000);

        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b4 = (unsigned char)(tls.b4);
        _bytes.F.b3 |= (unsigned char)(tls.b3 & 0b11110000);

        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b4 = (unsigned char)(tfs.b4);
        _bytes.R.b3 |= (unsigned char)(tfs.b3 & 0b11110000);

        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b1 &= 0b00000000;
        _bytes.B.b2 |= (unsigned char)(trs.b3 >> 4 & 0b00001111);
        _bytes.B.b1 |= (unsigned char)(trs.b4 << 4 & 0b11110000);
        _bytes.B.b1 |= (unsigned char)(trs.b4 >> 4 & 0b00001111);
    }
    else
    {
        tfs = _bytes.F;
        tls = _bytes.L;
        tbs = _bytes.B;
        trs = _bytes.R;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b4 &= 0b00000000;
        _bytes.L.b3 |= (unsigned char)(tbs.b2 << 4 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tbs.b1 >> 4 & 0b00001111);
        _bytes.L.b4 |= (unsigned char)(tbs.b1 << 4 & 0b11110000);
        _bytes.L.b3 += mud;
        _bytes.L.b4 += mud;

        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b4 = (unsigned char)(tls.b4);
        _bytes.F.b3 |= (unsigned char)(tls.b3 & 0b11110000);
        _bytes.F.b3 += mud;
        _bytes.F.b4 += mud;

        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b4 = (unsigned char)(tfs.b4);
        _bytes.R.b3 |= (unsigned char)(tfs.b3 & 0b11110000);
        _bytes.R.b3 += mud;
        _bytes.R.b4 += mud;

        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b1 &= 0b00000000;
        _bytes.B.b2 |= (unsigned char)(trs.b3 >> 4 & 0b00001111);
        _bytes.B.b1 |= (unsigned char)(trs.b4 << 4 & 0b11110000);
        _bytes.B.b1 |= (unsigned char)(trs.b4 >> 4 & 0b00001111);
        _bytes.B.b1 += mud;
        _bytes.B.b2 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideClockwise(&_bytes.D);
}

void Cube4Bit::AntiR()
{
    SideBytes tus;
    SideBytes tfs;
    SideBytes tbs;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b2 -= mud;
        _bytes.U.b3 -= mud;
        _bytes.U.b4 -= mud;

        _bytes.F.b2 -= mud;
        _bytes.F.b3 -= mud;
        _bytes.F.b4 -= mud;

        _bytes.B.b2 -= mud;
        _bytes.B.b3 -= mud;
        _bytes.B.b4 -= mud;

        _bytes.D.b2 -= mud;
        _bytes.D.b3 -= mud;
        _bytes.D.b4 -= mud;
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;

        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b3 &= 0b11110000;
        _bytes.U.b4 &= 0b00001111;
        _bytes.U.b2 |= (unsigned char)(tbs.b2 & 0b00001111);
        _bytes.U.b3 |= (unsigned char)(tbs.b3 & 0b00001111);
        _bytes.U.b4 |= (unsigned char)(tbs.b4 & 0b11110000);

        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b3 &= 0b11110000;
        _bytes.B.b4 &= 0b00001111;
        _bytes.B.b2 |= (unsigned char)(tds.b2 & 0b00001111);
        _bytes.B.b3 |= (unsigned char)(tds.b3 & 0b00001111);
        _bytes.B.b4 |= (unsigned char)(tds.b4 & 0b11110000);

        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b3 &= 0b11110000;
        _bytes.D.b4 &= 0b00001111;
        _bytes.D.b2 |= (unsigned char)(tfs.b2 & 0b00001111);
        _bytes.D.b3 |= (unsigned char)(tfs.b3 & 0b00001111);
        _bytes.D.b4 |= (unsigned char)(tfs.b4 & 0b11110000);

        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b3 &= 0b11110000;
        _bytes.F.b4 &= 0b00001111;
        _bytes.F.b2 |= (unsigned char)(tus.b2 & 0b00001111);
        _bytes.F.b3 |= (unsigned char)(tus.b3 & 0b00001111);
        _bytes.F.b4 |= (unsigned char)(tus.b4 & 0b11110000);
    }
    else
    {
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b3 &= 0b11110000;
        _bytes.U.b4 &= 0b00001111;
        _bytes.U.b2 |= (unsigned char)(tbs.b2 & 0b00001111);
        _bytes.U.b3 |= (unsigned char)(tbs.b3 & 0b00001111);
        _bytes.U.b4 |= (unsigned char)(tbs.b4 & 0b11110000);

        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b3 &= 0b11110000;
        _bytes.B.b4 &= 0b00001111;
        _bytes.B.b2 |= (unsigned char)(tds.b2 & 0b00001111);
        _bytes.B.b3 |= (unsigned char)(tds.b3 & 0b00001111);
        _bytes.B.b4 |= (unsigned char)(tds.b4 & 0b11110000);

        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b3 &= 0b11110000;
        _bytes.D.b4 &= 0b00001111;
        _bytes.D.b2 |= (unsigned char)(tfs.b2 & 0b00001111);
        _bytes.D.b3 |= (unsigned char)(tfs.b3 & 0b00001111);
        _bytes.D.b4 |= (unsigned char)(tfs.b4 & 0b11110000);

        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b3 &= 0b11110000;
        _bytes.F.b4 &= 0b00001111;
        _bytes.F.b2 |= (unsigned char)(tus.b2 & 0b00001111);
        _bytes.F.b3 |= (unsigned char)(tus.b3 & 0b00001111);
        _bytes.F.b4 |= (unsigned char)(tus.b4 & 0b11110000);

        _bytes.U.b2 += mud;
        _bytes.U.b3 += mud;
        _bytes.U.b4 += mud;

        _bytes.B.b2 += mud;
        _bytes.B.b3 += mud;
        _bytes.B.b4 += mud;

        _bytes.D.b2 += mud;
        _bytes.D.b3 += mud;
        _bytes.D.b4 += mud;

        _bytes.F.b2 += mud;
        _bytes.F.b3 += mud;
        _bytes.F.b4 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(&_bytes.R);
}

void Cube4Bit::AntiL()
{
    SideBytes tus;
    SideBytes tfs;
    SideBytes tbs;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b1 -= mud;
        _bytes.U.b2 -= mud;
        _bytes.U.b3 -= mud;

        _bytes.F.b1 -= mud;
        _bytes.F.b2 -= mud;
        _bytes.F.b3 -= mud;

        _bytes.B.b1 -= mud;
        _bytes.B.b2 -= mud;
        _bytes.B.b3 -= mud;

        _bytes.D.b1 -= mud;
        _bytes.D.b2 -= mud;
        _bytes.D.b3 -= mud;
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;

        _bytes.U.b1 &= 0b11110000;
        _bytes.U.b2 &= 0b00001111;
        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b1 |= (unsigned char)(tfs.b1 & 0b00001111);
        _bytes.U.b2 |= (unsigned char)(tfs.b2 & 0b11110000);
        _bytes.U.b3 |= (unsigned char)(tfs.b3 & 0b11110000);

        _bytes.F.b1 &= 0b11110000;
        _bytes.F.b2 &= 0b00001111;
        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b1 |= (unsigned char)(tds.b1 & 0b00001111);
        _bytes.F.b2 |= (unsigned char)(tds.b2 & 0b11110000);
        _bytes.F.b3 |= (unsigned char)(tds.b3 & 0b11110000);

        _bytes.D.b1 &= 0b11110000;
        _bytes.D.b2 &= 0b00001111;
        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b1 |= (unsigned char)(tbs.b1 & 0b00001111);
        _bytes.D.b2 |= (unsigned char)(tbs.b2 & 0b11110000);
        _bytes.D.b3 |= (unsigned char)(tbs.b3 & 0b11110000);

        _bytes.B.b1 &= 0b11110000;
        _bytes.B.b2 &= 0b00001111;
        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b1 |= (unsigned char)(tus.b1 & 0b00001111);
        _bytes.B.b2 |= (unsigned char)(tus.b2 & 0b11110000);
        _bytes.B.b3 |= (unsigned char)(tus.b3 & 0b11110000);
    }
    else
    {
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;
        _bytes.U.b1 &= 0b11110000;
        _bytes.U.b2 &= 0b00001111;
        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b1 |= (unsigned char)(tfs.b1 & 0b00001111);
        _bytes.U.b2 |= (unsigned char)(tfs.b2 & 0b11110000);
        _bytes.U.b3 |= (unsigned char)(tfs.b3 & 0b11110000);

        _bytes.F.b1 &= 0b11110000;
        _bytes.F.b2 &= 0b00001111;
        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b1 |= (unsigned char)(tds.b1 & 0b00001111);
        _bytes.F.b2 |= (unsigned char)(tds.b2 & 0b11110000);
        _bytes.F.b3 |= (unsigned char)(tds.b3 & 0b11110000);

        _bytes.D.b1 &= 0b11110000;
        _bytes.D.b2 &= 0b00001111;
        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b1 |= (unsigned char)(tbs.b1 & 0b00001111);
        _bytes.D.b2 |= (unsigned char)(tbs.b2 & 0b11110000);
        _bytes.D.b3 |= (unsigned char)(tbs.b3 & 0b11110000);

        _bytes.B.b1 &= 0b11110000;
        _bytes.B.b2 &= 0b00001111;
        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b1 |= (unsigned char)(tus.b1 & 0b00001111);
        _bytes.B.b2 |= (unsigned char)(tus.b2 & 0b11110000);
        _bytes.B.b3 |= (unsigned char)(tus.b3 & 0b11110000);

        _bytes.U.b1 += mud;
        _bytes.U.b2 += mud;
        _bytes.U.b3 += mud;

        _bytes.F.b1 += mud;
        _bytes.F.b2 += mud;
        _bytes.F.b3 += mud;

        _bytes.D.b1 += mud;
        _bytes.D.b2 += mud;
        _bytes.D.b3 += mud;

        _bytes.B.b1 += mud;
        _bytes.B.b2 += mud;
        _bytes.B.b3 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(&_bytes.L);
}

void Cube4Bit::AntiF()
{
    SideBytes tus;
    SideBytes trs;
    SideBytes tds;
    SideBytes tls;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b3 -= mud;
        _bytes.U.b4 -= mud;

        _bytes.R.b1 -= mud;
        _bytes.R.b2 -= mud;
        _bytes.R.b3 -= mud;

        _bytes.L.b2 -= mud;
        _bytes.L.b3 -= mud;
        _bytes.L.b4 -= mud;

        _bytes.D.b1 -= mud;
        _bytes.D.b2 -= mud;
        tus = _bytes.U;
        trs = _bytes.R;
        tds = _bytes.D;
        tls = _bytes.L;
        _bytes.R.b1 &= 0b11110000;
        _bytes.R.b2 &= 0b00001111;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b1 |= (unsigned char)(tds.b2 & 0b00001111);
        _bytes.R.b2 |= (unsigned char)(tds.b1 & 0b11110000);
        _bytes.R.b3 |= (unsigned char)(tds.b1 << 4 & 0b11110000);

        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b1 &= 0b00000000;
        _bytes.D.b2 |= (unsigned char)(tls.b4 >> 4 & 0b00001111);
        _bytes.D.b1 |= (unsigned char)(tls.b3 << 4 & 0b11110000);
        _bytes.D.b1 |= (unsigned char)(tls.b2 & 0b00001111);

        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b3 &= 0b11110000;
        _bytes.L.b4 &= 0b00001111;
        _bytes.L.b2 |= (unsigned char)(tus.b4 >> 4 & 0b00001111);
        _bytes.L.b3 |= (unsigned char)(tus.b4 & 0b00001111);
        _bytes.L.b4 |= (unsigned char)(tus.b3 & 0b11110000);

        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b4 &= 0b00000000;
        _bytes.U.b3 |= (unsigned char)(trs.b1 << 4 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(trs.b2 >> 4 & 0b00001111);
        _bytes.U.b4 |= (unsigned char)(trs.b3 & 0b11110000);
    }
    else
    {
        tus = _bytes.U;
        trs = _bytes.R;
        tds = _bytes.D;
        tls = _bytes.L;
        _bytes.R.b1 &= 0b11110000;
        _bytes.R.b2 &= 0b00001111;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b1 |= (unsigned char)(tds.b2 & 0b00001111);
        _bytes.R.b2 |= (unsigned char)(tds.b1 & 0b11110000);
        _bytes.R.b3 |= (unsigned char)(tds.b1 << 4 & 0b11110000);

        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b1 &= 0b00000000;
        _bytes.D.b2 |= (unsigned char)(tls.b4 >> 4 & 0b00001111);
        _bytes.D.b1 |= (unsigned char)(tls.b3 << 4 & 0b11110000);
        _bytes.D.b1 |= (unsigned char)(tls.b2 & 0b00001111);

        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b3 &= 0b11110000;
        _bytes.L.b4 &= 0b00001111;
        _bytes.L.b2 |= (unsigned char)(tus.b4 >> 4 & 0b00001111);
        _bytes.L.b3 |= (unsigned char)(tus.b4 & 0b00001111);
        _bytes.L.b4 |= (unsigned char)(tus.b3 & 0b11110000);

        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b4 &= 0b00000000;
        _bytes.U.b3 |= (unsigned char)(trs.b1 << 4 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(trs.b2 >> 4 & 0b00001111);
        _bytes.U.b4 |= (unsigned char)(trs.b3 & 0b11110000);
        _bytes.R.b1 += mud;
        _bytes.R.b2 += mud;
        _bytes.R.b3 += mud;

        _bytes.D.b1 += mud;
        _bytes.D.b2 += mud;

        _bytes.L.b4 += mud;
        _bytes.L.b2 += mud;
        _bytes.L.b3 += mud;

        _bytes.U.b3 += mud;
        _bytes.U.b4 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(&_bytes.F);
}

void Cube4Bit::AntiB()
{
    SideBytes tus;
    SideBytes trs;
    SideBytes tds;
    SideBytes tls;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.D.b3 -= mud;
        _bytes.D.b4 -= mud;

        _bytes.L.b1 -= mud;
        _bytes.L.b2 -= mud;
        _bytes.L.b3 -= mud;

        _bytes.R.b2 -= mud;
        _bytes.R.b3 -= mud;
        _bytes.R.b4 -= mud;

        _bytes.U.b1 -= mud;
        _bytes.U.b2 -= mud;
        tus = _bytes.U;
        trs = _bytes.R;
        tds = _bytes.D;
        tls = _bytes.L;
        _bytes.L.b1 &= 0b11110000;
        _bytes.L.b2 &= 0b00001111;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b1 |= (unsigned char)(tds.b3 >> 4 & 0b00001111);
        _bytes.L.b2 |= (unsigned char)(tds.b4 << 4 & 0b11110000);
        _bytes.L.b3 |= (unsigned char)(tds.b4 & 0b11110000);

        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b4 &= 0b00000000;
        _bytes.D.b3 |= (unsigned char)(trs.b4 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(trs.b3 & 0b00001111);
        _bytes.D.b4 |= (unsigned char)(trs.b2 << 4 & 0b11110000);

        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b3 &= 0b11110000;
        _bytes.R.b4 &= 0b00001111;
        _bytes.R.b2 |= (unsigned char)(tus.b1 & 0b00001111);
        _bytes.R.b3 |= (unsigned char)(tus.b1 >> 4 & 0b00001111);
        _bytes.R.b4 |= (unsigned char)(tus.b2 << 4 & 0b11110000);

        _bytes.U.b1 &= 0b00000000;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b1 |= (unsigned char)(tls.b3 >> 4 & 0b00001111);
        _bytes.U.b1 |= (unsigned char)(tls.b2 & 0b11110000);
        _bytes.U.b2 |= (unsigned char)(tls.b1 & 0b00001111);
    }
    else
    {
        tus = _bytes.U;
        trs = _bytes.R;
        tds = _bytes.D;
        tls = _bytes.L;
        _bytes.L.b1 &= 0b11110000;
        _bytes.L.b2 &= 0b00001111;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b1 |= (unsigned char)(tds.b3 >> 4 & 0b00001111);
        _bytes.L.b2 |= (unsigned char)(tds.b4 << 4 & 0b11110000);
        _bytes.L.b3 |= (unsigned char)(tds.b4 & 0b11110000);

        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b4 &= 0b00000000;
        _bytes.D.b3 |= (unsigned char)(trs.b4 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(trs.b3 & 0b00001111);
        _bytes.D.b4 |= (unsigned char)(trs.b2 << 4 & 0b11110000);

        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b3 &= 0b11110000;
        _bytes.R.b4 &= 0b00001111;
        _bytes.R.b2 |= (unsigned char)(tus.b1 & 0b00001111);
        _bytes.R.b3 |= (unsigned char)(tus.b1 >> 4 & 0b00001111);
        _bytes.R.b4 |= (unsigned char)(tus.b2 << 4 & 0b11110000);

        _bytes.U.b1 &= 0b00000000;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b1 |= (unsigned char)(tls.b3 >> 4 & 0b00001111);
        _bytes.U.b1 |= (unsigned char)(tls.b2 & 0b11110000);
        _bytes.U.b2 |= (unsigned char)(tls.b1 & 0b00001111);
        _bytes.L.b1 += mud;
        _bytes.L.b2 += mud;
        _bytes.L.b3 += mud;

        _bytes.D.b4 += mud;
        _bytes.D.b3 += mud;

        _bytes.R.b4 += mud;
        _bytes.R.b2 += mud;
        _bytes.R.b3 += mud;

        _bytes.U.b1 += mud;
        _bytes.U.b2 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(&_bytes.B);
}

void Cube4Bit::AntiU()
{
    SideBytes tfs;
    SideBytes tls;
    SideBytes tbs;
    SideBytes trs;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.B.b3 -= mud;
        _bytes.B.b4 -= mud;

        _bytes.R.b1 -= mud;
        _bytes.R.b2 -= mud;

        _bytes.F.b1 -= mud;
        _bytes.F.b2 -= mud;

        _bytes.L.b1 -= mud;
        _bytes.L.b2 -= mud;
        tfs = _bytes.F;
        tls = _bytes.L;
        tbs = _bytes.B;
        trs = _bytes.R;
        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b1 &= 0b00000000;
        _bytes.L.b2 |= (unsigned char)(tbs.b3 >> 4 & 0b00001111);
        _bytes.L.b1 |= (unsigned char)(tbs.b4 << 4 & 0b11110000);
        _bytes.L.b1 |= (unsigned char)(tbs.b4 >> 4 & 0b00001111);

        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b4 &= 0b00000000;
        _bytes.B.b3 |= (unsigned char)(trs.b2 << 4 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(trs.b1 >> 4 & 0b00001111);
        _bytes.B.b4 |= (unsigned char)(trs.b1 << 4 & 0b11110000);

        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b1 = (unsigned char)(tfs.b1);
        _bytes.R.b2 |= (unsigned char)(tfs.b2 & 0b00001111);

        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b1 = (unsigned char)(tls.b1);
        _bytes.F.b2 |= (unsigned char)(tls.b2 & 0b00001111);
    }
    else
    {
        tfs = _bytes.F;
        tls = _bytes.L;
        tbs = _bytes.B;
        trs = _bytes.R;
        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b1 &= 0b00000000;
        _bytes.L.b2 |= (unsigned char)(tbs.b3 >> 4 & 0b00001111);
        _bytes.L.b1 |= (unsigned char)(tbs.b4 << 4 & 0b11110000);
        _bytes.L.b1 |= (unsigned char)(tbs.b4 >> 4 & 0b00001111);

        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b4 &= 0b00000000;
        _bytes.B.b3 |= (unsigned char)(trs.b2 << 4 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(trs.b1 >> 4 & 0b00001111);
        _bytes.B.b4 |= (unsigned char)(trs.b1 << 4 & 0b11110000);

        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b1 = (unsigned char)(tfs.b1);
        _bytes.R.b2 |= (unsigned char)(tfs.b2 & 0b00001111);

        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b1 = (unsigned char)(tls.b1);
        _bytes.F.b2 |= (unsigned char)(tls.b2 & 0b00001111);
        _bytes.L.b1 += mud;
        _bytes.L.b2 += mud;

        _bytes.B.b3 += mud;
        _bytes.B.b4 += mud;

        _bytes.R.b1 += mud;
        _bytes.R.b2 += mud;

        _bytes.F.b1 += mud;
        _bytes.F.b2 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(&_bytes.U);
}

void Cube4Bit::AntiD()
{
    SideBytes tfs;
    SideBytes tls;
    SideBytes tbs;
    SideBytes trs;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.B.b1 -= mud;
        _bytes.B.b2 -= mud;

        _bytes.R.b3 -= mud;
        _bytes.R.b4 -= mud;

        _bytes.F.b3 -= mud;
        _bytes.F.b4 -= mud;

        _bytes.L.b3 -= mud;
        _bytes.L.b4 -= mud;
        tfs = _bytes.F;
        tls = _bytes.L;
        tbs = _bytes.B;
        trs = _bytes.R;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b4 &= 0b00000000;
        _bytes.R.b3 |= (unsigned char)(tbs.b2 << 4 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tbs.b1 >> 4 & 0b00001111);
        _bytes.R.b4 |= (unsigned char)(tbs.b1 << 4 & 0b11110000);

        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b4 = (unsigned char)(trs.b4);
        _bytes.F.b3 |= (unsigned char)(trs.b3 & 0b11110000);

        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b4 = (unsigned char)(tfs.b4);
        _bytes.L.b3 |= (unsigned char)(tfs.b3 & 0b11110000);

        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b1 &= 0b00000000;
        _bytes.B.b2 |= (unsigned char)(tls.b3 >> 4 & 0b00001111);
        _bytes.B.b1 |= (unsigned char)(tls.b4 << 4 & 0b11110000);
        _bytes.B.b1 |= (unsigned char)(tls.b4 >> 4 & 0b00001111);
    }
    else
    {
        tfs = _bytes.F;
        tls = _bytes.L;
        tbs = _bytes.B;
        trs = _bytes.R;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b4 &= 0b00000000;
        _bytes.R.b3 |= (unsigned char)(tbs.b2 << 4 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tbs.b1 >> 4 & 0b00001111);
        _bytes.R.b4 |= (unsigned char)(tbs.b1 << 4 & 0b11110000);

        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b4 = (unsigned char)(trs.b4);
        _bytes.F.b3 |= (unsigned char)(trs.b3 & 0b11110000);

        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b4 = (unsigned char)(tfs.b4);
        _bytes.L.b3 |= (unsigned char)(tfs.b3 & 0b11110000);

        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b1 &= 0b00000000;
        _bytes.B.b2 |= (unsigned char)(tls.b3 >> 4 & 0b00001111);
        _bytes.B.b1 |= (unsigned char)(tls.b4 << 4 & 0b11110000);
        _bytes.B.b1 |= (unsigned char)(tls.b4 >> 4 & 0b00001111);
        _bytes.L.b3 += mud;
        _bytes.L.b4 += mud;

        _bytes.F.b3 += mud;
        _bytes.F.b4 += mud;

        _bytes.R.b3 += mud;
        _bytes.R.b4 += mud;

        _bytes.B.b1 += mud;
        _bytes.B.b2 += mud;
    }
    _numIndex += _offset;
    //---------------------
    MoveSideCounterClockwise(&_bytes.D);
}

void Cube4Bit::M()
{
    SideBytes tus;
    SideBytes tfs;
    SideBytes tbs;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b1 -= mud;
        _bytes.U.b4 -= mud;

        _bytes.F.b1 -= mud;
        _bytes.F.b4 -= mud;

        _bytes.D.b1 -= mud;
        _bytes.D.b4 -= mud;

        _bytes.B.b1 -= mud;
        _bytes.B.b4 -= mud;
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;

        _bytes.U.b1 &= 0b00001111;
        _bytes.U.b4 &= 0b11110000;
        _bytes.U.b1 |= (unsigned char)(tbs.b1 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(tbs.b4 & 0b00001111);

        _bytes.F.b1 &= 0b00001111;
        _bytes.F.b4 &= 0b11110000;
        _bytes.F.b1 |= (unsigned char)(tus.b1 & 0b11110000);
        _bytes.F.b4 |= (unsigned char)(tus.b4 & 0b00001111);

        _bytes.D.b1 &= 0b00001111;
        _bytes.D.b4 &= 0b11110000;
        _bytes.D.b1 |= (unsigned char)(tfs.b1 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(tfs.b4 & 0b00001111);

        _bytes.B.b1 &= 0b00001111;
        _bytes.B.b4 &= 0b11110000;
        _bytes.B.b1 |= (unsigned char)(tds.b1 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(tds.b4 & 0b00001111);
    }
    else
    {
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;
        _bytes.U.b1 &= 0b00001111;
        _bytes.U.b4 &= 0b11110000;
        _bytes.U.b1 |= (unsigned char)(tbs.b1 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(tbs.b4 & 0b00001111);
        _bytes.U.b1 += mud;
        _bytes.U.b4 += mud;

        _bytes.F.b1 &= 0b00001111;
        _bytes.F.b4 &= 0b11110000;
        _bytes.F.b1 |= (unsigned char)(tus.b1 & 0b11110000);
        _bytes.F.b4 |= (unsigned char)(tus.b4 & 0b00001111);
        _bytes.F.b1 += mud;
        _bytes.F.b4 += mud;

        _bytes.D.b1 &= 0b00001111;
        _bytes.D.b4 &= 0b11110000;
        _bytes.D.b1 |= (unsigned char)(tfs.b1 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(tfs.b4 & 0b00001111);
        _bytes.D.b1 += mud;
        _bytes.D.b4 += mud;

        _bytes.B.b1 &= 0b00001111;
        _bytes.B.b4 &= 0b11110000;
        _bytes.B.b1 |= (unsigned char)(tds.b1 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(tds.b4 & 0b00001111);
        _bytes.B.b1 += mud;
        _bytes.B.b4 += mud;
    }
    _numIndex += _offset;
}

void Cube4Bit::AntiM()
{
    SideBytes tus;
    SideBytes tfs;
    SideBytes tbs;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b1 -= mud;
        _bytes.U.b4 -= mud;

        _bytes.F.b1 -= mud;
        _bytes.F.b4 -= mud;

        _bytes.D.b1 -= mud;
        _bytes.D.b4 -= mud;

        _bytes.B.b1 -= mud;
        _bytes.B.b4 -= mud;
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;
        _bytes.U.b1 &= 0b00001111;
        _bytes.U.b4 &= 0b11110000;
        _bytes.U.b1 |= (unsigned char)(tfs.b1 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(tfs.b4 & 0b00001111);

        _bytes.F.b1 &= 0b00001111;
        _bytes.F.b4 &= 0b11110000;
        _bytes.F.b1 |= (unsigned char)(tds.b1 & 0b11110000);
        _bytes.F.b4 |= (unsigned char)(tds.b4 & 0b00001111);

        _bytes.D.b1 &= 0b00001111;
        _bytes.D.b4 &= 0b11110000;
        _bytes.D.b1 |= (unsigned char)(tbs.b1 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(tbs.b4 & 0b00001111);

        _bytes.B.b1 &= 0b00001111;
        _bytes.B.b4 &= 0b11110000;
        _bytes.B.b1 |= (unsigned char)(tus.b1 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(tus.b4 & 0b00001111);
    }
    else
    {
        tus = _bytes.U;
        tfs = _bytes.F;
        tbs = _bytes.B;
        tds = _bytes.D;
        _bytes.U.b1 &= 0b00001111;
        _bytes.U.b4 &= 0b11110000;
        _bytes.U.b1 |= (unsigned char)(tfs.b1 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(tfs.b4 & 0b00001111);

        _bytes.F.b1 &= 0b00001111;
        _bytes.F.b4 &= 0b11110000;
        _bytes.F.b1 |= (unsigned char)(tds.b1 & 0b11110000);
        _bytes.F.b4 |= (unsigned char)(tds.b4 & 0b00001111);

        _bytes.D.b1 &= 0b00001111;
        _bytes.D.b4 &= 0b11110000;
        _bytes.D.b1 |= (unsigned char)(tbs.b1 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(tbs.b4 & 0b00001111);

        _bytes.B.b1 &= 0b00001111;
        _bytes.B.b4 &= 0b11110000;
        _bytes.B.b1 |= (unsigned char)(tus.b1 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(tus.b4 & 0b00001111);
        _bytes.U.b1 += mud;
        _bytes.U.b4 += mud;

        _bytes.F.b1 += mud;
        _bytes.F.b4 += mud;

        _bytes.D.b1 += mud;
        _bytes.D.b4 += mud;

        _bytes.B.b1 += mud;
        _bytes.B.b4 += mud;
    }
    _numIndex += _offset;
}

void Cube4Bit::S()
{
    SideBytes tus;
    SideBytes trs;
    SideBytes tls;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b2 -= mud;
        _bytes.U.b3 -= mud;

        _bytes.R.b1 -= mud;
        _bytes.R.b4 -= mud;

        _bytes.D.b2 -= mud;
        _bytes.D.b3 -= mud;

        _bytes.L.b1 -= mud;
        _bytes.L.b4 -= mud;
        tus = _bytes.U;
        trs = _bytes.R;
        tls = _bytes.L;
        tds = _bytes.D;

        _bytes.U.b2 &= 0b00001111;
        _bytes.U.b3 &= 0b11110000;
        _bytes.U.b2 |= (unsigned char)(tls.b4 << 4 & 0b11110000);
        _bytes.U.b3 |= (unsigned char)(tls.b1 >> 4 & 0b00001111);

        _bytes.R.b1 &= 0b00001111;
        _bytes.R.b4 &= 0b11110000;
        _bytes.R.b1 |= (unsigned char)(tus.b2 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tus.b3 & 0b00001111);

        _bytes.D.b2 &= 0b00001111;
        _bytes.D.b3 &= 0b11110000;
        _bytes.D.b2 |= (unsigned char)(trs.b4 << 4 & 0b11110000);
        _bytes.D.b3 |= (unsigned char)(trs.b1 >> 4 & 0b00001111);

        _bytes.L.b1 &= 0b00001111;
        _bytes.L.b4 &= 0b11110000;
        _bytes.L.b1 |= (unsigned char)(tds.b2 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tds.b3 & 0b00001111);
    }
    else
    {
        tus = _bytes.U;
        trs = _bytes.R;
        tls = _bytes.L;
        tds = _bytes.D;

        _bytes.U.b2 &= 0b00001111;
        _bytes.U.b3 &= 0b11110000;
        _bytes.U.b2 |= (unsigned char)(tls.b4 << 4 & 0b11110000);
        _bytes.U.b3 |= (unsigned char)(tls.b1 >> 4 & 0b00001111);
        _bytes.U.b2 += mud;
        _bytes.U.b3 += mud;

        _bytes.R.b1 &= 0b00001111;
        _bytes.R.b4 &= 0b11110000;
        _bytes.R.b1 |= (unsigned char)(tus.b2 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tus.b3 & 0b00001111);
        _bytes.R.b1 += mud;
        _bytes.R.b4 += mud;

        _bytes.D.b2 &= 0b00001111;
        _bytes.D.b3 &= 0b11110000;
        _bytes.D.b2 |= (unsigned char)(trs.b4 << 4 & 0b11110000);
        _bytes.D.b3 |= (unsigned char)(trs.b1 >> 4 & 0b00001111);
        _bytes.D.b2 += mud;
        _bytes.D.b3 += mud;

        _bytes.L.b1 &= 0b00001111;
        _bytes.L.b4 &= 0b11110000;
        _bytes.L.b1 |= (unsigned char)(tds.b2 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tds.b3 & 0b00001111);
        _bytes.L.b1 += mud;
        _bytes.L.b4 += mud;
    }
    _numIndex += _offset;
}

void Cube4Bit::AntiS()
{
    SideBytes tus;
    SideBytes trs;
    SideBytes tls;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.U.b2 -= mud;
        _bytes.U.b3 -= mud;

        _bytes.R.b1 -= mud;
        _bytes.R.b4 -= mud;

        _bytes.D.b2 -= mud;
        _bytes.D.b3 -= mud;

        _bytes.L.b1 -= mud;
        _bytes.L.b4 -= mud;
        tus = _bytes.U;
        trs = _bytes.R;
        tls = _bytes.L;
        tds = _bytes.D;
        _bytes.U.b2 &= 0b00001111;
        _bytes.U.b3 &= 0b11110000;
        _bytes.U.b2 |= (unsigned char)(trs.b1 & 0b11110000);
        _bytes.U.b3 |= (unsigned char)(trs.b4 & 0b00001111);

        _bytes.R.b1 &= 0b00001111;
        _bytes.R.b4 &= 0b11110000;
        _bytes.R.b1 |= (unsigned char)(tds.b3 << 4 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tds.b2 >> 4 & 0b00001111);

        _bytes.D.b2 &= 0b00001111;
        _bytes.D.b3 &= 0b11110000;
        _bytes.D.b2 |= (unsigned char)(tls.b1 & 0b11110000);
        _bytes.D.b3 |= (unsigned char)(tls.b4 & 0b00001111);

        _bytes.L.b1 &= 0b00001111;
        _bytes.L.b4 &= 0b11110000;
        _bytes.L.b1 |= (unsigned char)(tus.b3 << 4 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tus.b2 >> 4 & 0b00001111);
    }
    else
    {
        tus = _bytes.U;
        trs = _bytes.R;
        tls = _bytes.L;
        tds = _bytes.D;
        _bytes.U.b2 &= 0b00001111;
        _bytes.U.b3 &= 0b11110000;
        _bytes.U.b2 |= (unsigned char)(trs.b1 & 0b11110000);
        _bytes.U.b3 |= (unsigned char)(trs.b4 & 0b00001111);

        _bytes.R.b1 &= 0b00001111;
        _bytes.R.b4 &= 0b11110000;
        _bytes.R.b1 |= (unsigned char)(tds.b3 << 4 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tds.b2 >> 4 & 0b00001111);

        _bytes.D.b2 &= 0b00001111;
        _bytes.D.b3 &= 0b11110000;
        _bytes.D.b2 |= (unsigned char)(tls.b1 & 0b11110000);
        _bytes.D.b3 |= (unsigned char)(tls.b4 & 0b00001111);

        _bytes.L.b1 &= 0b00001111;
        _bytes.L.b4 &= 0b11110000;
        _bytes.L.b1 |= (unsigned char)(tus.b3 << 4 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tus.b2 >> 4 & 0b00001111);
        _bytes.U.b2 += mud;
        _bytes.U.b3 += mud;

        _bytes.R.b1 += mud;
        _bytes.R.b4 += mud;

        _bytes.D.b2 += mud;
        _bytes.D.b3 += mud;

        _bytes.L.b1 += mud;
        _bytes.L.b4 += mud;
    }
    _numIndex += _offset;
}

void Cube4Bit::E()
{
    SideBytes tfs;
    SideBytes trs;
    SideBytes tls;
    SideBytes tbs;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.F.b2 -= mud;
        _bytes.F.b3 -= mud;

        _bytes.R.b2 -= mud;
        _bytes.R.b3 -= mud;

        _bytes.B.b2 -= mud;
        _bytes.B.b3 -= mud;

        _bytes.L.b2 -= mud;
        _bytes.L.b3 -= mud;
        tfs = _bytes.F;
        trs = _bytes.R;
        tls = _bytes.L;
        tbs = _bytes.B;

        _bytes.F.b2 &= 0b00001111;
        _bytes.F.b3 &= 0b11110000;
        _bytes.F.b2 |= (unsigned char)(tls.b2 & 0b11110000);
        _bytes.F.b3 |= (unsigned char)(tls.b3 & 0b00001111);

        _bytes.R.b2 &= 0b00001111;
        _bytes.R.b3 &= 0b11110000;
        _bytes.R.b2 |= (unsigned char)(tfs.b2 & 0b11110000);
        _bytes.R.b3 |= (unsigned char)(tfs.b3 & 0b00001111);

        _bytes.B.b2 &= 0b00001111;
        _bytes.B.b3 &= 0b11110000;
        _bytes.B.b2 |= (unsigned char)(trs.b3 << 4 & 0b11110000);
        _bytes.B.b3 |= (unsigned char)(trs.b2 >> 4 & 0b00001111);

        _bytes.L.b2 &= 0b00001111;
        _bytes.L.b3 &= 0b11110000;
        _bytes.L.b2 |= (unsigned char)(tbs.b3 << 4 & 0b11110000);
        _bytes.L.b3 |= (unsigned char)(tbs.b2 >> 4 & 0b00001111);
    }
    else
    {
        tfs = _bytes.F;
        trs = _bytes.R;
        tls = _bytes.L;
        tbs = _bytes.B;
        _bytes.F.b2 &= 0b00001111;
        _bytes.F.b3 &= 0b11110000;
        _bytes.F.b2 |= (unsigned char)(tls.b2 & 0b11110000);
        _bytes.F.b3 |= (unsigned char)(tls.b3 & 0b00001111);
        _bytes.F.b2 += mud;
        _bytes.F.b3 += mud;

        _bytes.R.b2 &= 0b00001111;
        _bytes.R.b3 &= 0b11110000;
        _bytes.R.b2 |= (unsigned char)(tfs.b2 & 0b11110000);
        _bytes.R.b3 |= (unsigned char)(tfs.b3 & 0b00001111);
        _bytes.R.b2 += mud;
        _bytes.R.b3 += mud;

        _bytes.B.b2 &= 0b00001111;
        _bytes.B.b3 &= 0b11110000;
        _bytes.B.b2 |= (unsigned char)(trs.b3 << 4 & 0b11110000);
        _bytes.B.b3 |= (unsigned char)(trs.b2 >> 4 & 0b00001111);
        _bytes.B.b2 += mud;
        _bytes.B.b3 += mud;

        _bytes.L.b2 &= 0b00001111;
        _bytes.L.b3 &= 0b11110000;
        _bytes.L.b2 |= (unsigned char)(tbs.b3 << 4 & 0b11110000);
        _bytes.L.b3 |= (unsigned char)(tbs.b2 >> 4 & 0b00001111);
        _bytes.L.b2 += mud;
        _bytes.L.b3 += mud;
    }
    _numIndex += _offset;
}

void Cube4Bit::AntiE()
{
    SideBytes tfs;
    SideBytes trs;
    SideBytes tls;
    SideBytes tbs;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.F.b2 -= mud;
        _bytes.F.b3 -= mud;

        _bytes.R.b2 -= mud;
        _bytes.R.b3 -= mud;

        _bytes.B.b2 -= mud;
        _bytes.B.b3 -= mud;

        _bytes.L.b2 -= mud;
        _bytes.L.b3 -= mud;
        tfs = _bytes.F;
        trs = _bytes.R;
        tls = _bytes.L;
        tbs = _bytes.B;
        _bytes.F.b2 &= 0b00001111;
        _bytes.F.b3 &= 0b11110000;
        _bytes.F.b2 |= (unsigned char)(trs.b2 & 0b11110000);
        _bytes.F.b3 |= (unsigned char)(trs.b3 & 0b00001111);

        _bytes.L.b2 &= 0b00001111;
        _bytes.L.b3 &= 0b11110000;
        _bytes.L.b2 |= (unsigned char)(tfs.b2 & 0b11110000);
        _bytes.L.b3 |= (unsigned char)(tfs.b3 & 0b00001111);

        _bytes.B.b2 &= 0b00001111;
        _bytes.B.b3 &= 0b11110000;
        _bytes.B.b2 |= (unsigned char)(tls.b3 << 4 & 0b11110000);
        _bytes.B.b3 |= (unsigned char)(tls.b2 >> 4 & 0b00001111);

        _bytes.R.b2 &= 0b00001111;
        _bytes.R.b3 &= 0b11110000;
        _bytes.R.b2 |= (unsigned char)(tbs.b3 << 4 & 0b11110000);
        _bytes.R.b3 |= (unsigned char)(tbs.b2 >> 4 & 0b00001111);
    }
    else
    {
        tfs = _bytes.F;
        trs = _bytes.R;
        tls = _bytes.L;
        tbs = _bytes.B;
        _bytes.F.b2 &= 0b00001111;
        _bytes.F.b3 &= 0b11110000;
        _bytes.F.b2 |= (unsigned char)(trs.b2 & 0b11110000);
        _bytes.F.b3 |= (unsigned char)(trs.b3 & 0b00001111);

        _bytes.L.b2 &= 0b00001111;
        _bytes.L.b3 &= 0b11110000;
        _bytes.L.b2 |= (unsigned char)(tfs.b2 & 0b11110000);
        _bytes.L.b3 |= (unsigned char)(tfs.b3 & 0b00001111);

        _bytes.B.b2 &= 0b00001111;
        _bytes.B.b3 &= 0b11110000;
        _bytes.B.b2 |= (unsigned char)(tls.b3 << 4 & 0b11110000);
        _bytes.B.b3 |= (unsigned char)(tls.b2 >> 4 & 0b00001111);

        _bytes.R.b2 &= 0b00001111;
        _bytes.R.b3 &= 0b11110000;
        _bytes.R.b2 |= (unsigned char)(tbs.b3 << 4 & 0b11110000);
        _bytes.R.b3 |= (unsigned char)(tbs.b2 >> 4 & 0b00001111);
        _bytes.F.b2 += mud;
        _bytes.F.b3 += mud;

        _bytes.R.b2 += mud;
        _bytes.R.b3 += mud;

        _bytes.B.b2 += mud;
        _bytes.B.b3 += mud;

        _bytes.L.b2 += mud;
        _bytes.L.b3 += mud;
    }
    _numIndex += _offset;
}

void Cube4Bit::C()
{
    SideBytes tfs;
    SideBytes trs;
    SideBytes tls;
    SideBytes tbs;
    SideBytes tus;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.F.b1 -= mud;
        _bytes.F.b2 -= mud;
        _bytes.F.b3 -= mud;
        _bytes.F.b4 -= mud;

        _bytes.B.b1 -= mud;
        _bytes.B.b2 -= mud;
        _bytes.B.b3 -= mud;
        _bytes.B.b4 -= mud;

        tfs = _bytes.F;
        trs = _bytes.R;
        tls = _bytes.L;
        tbs = _bytes.B;
        tus = _bytes.U;
        tds = _bytes.D;
        _bytes.F.b1 &= 0b11110000;
        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b4 &= 0b00001111;
        _bytes.F.b1 |= (unsigned char)(tus.b3 >> 4 & 0b00001111);
        _bytes.F.b2 |= (unsigned char)(tus.b4 >> 4 & 0b00001111);
        _bytes.F.b3 |= (unsigned char)(tds.b1 << 4 & 0b11110000);
        _bytes.F.b4 |= (unsigned char)(tds.b2 << 4 & 0b11110000);

        _bytes.U.b1 &= 0b11110000;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b4 &= 0b00001111;
        _bytes.U.b1 |= (unsigned char)(tls.b1 & 0b00001111);
        _bytes.U.b2 |= (unsigned char)(trs.b2 & 0b00001111);
        _bytes.U.b3 |= (unsigned char)(tls.b2 << 4 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(trs.b1 << 4 & 0b11110000);

        _bytes.B.b1 &= 0b11110000;
        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b4 &= 0b00001111;
        _bytes.B.b1 |= (unsigned char)(tds.b3 >> 4 & 0b00001111);
        _bytes.B.b2 |= (unsigned char)(tds.b4 >> 4 & 0b00001111);
        _bytes.B.b3 |= (unsigned char)(tus.b1 << 4 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(tus.b2 << 4 & 0b11110000);

        _bytes.D.b1 &= 0b11110000;
        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b4 &= 0b00001111;
        _bytes.D.b1 |= (unsigned char)(tls.b4 >> 4 & 0b00001111);
        _bytes.D.b2 |= (unsigned char)(trs.b3 >> 4 & 0b00001111);
        _bytes.D.b3 |= (unsigned char)(tls.b3 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(trs.b4 & 0b11110000);

        _bytes.R.b1 &= 0b11110000;
        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b4 &= 0b00001111;
        _bytes.R.b1 |= (unsigned char)(tfs.b2 & 0b00001111);
        _bytes.R.b2 |= (unsigned char)(tbs.b4 >> 4 & 0b00001111);
        _bytes.R.b3 |= (unsigned char)(tfs.b4 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tbs.b2 << 4 & 0b11110000);

        _bytes.L.b1 &= 0b11110000;
        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b4 &= 0b00001111;
        _bytes.L.b1 |= (unsigned char)(tbs.b3 >> 4 & 0b00001111);
        _bytes.L.b2 |= (unsigned char)(tfs.b1 & 0b00001111);
        _bytes.L.b3 |= (unsigned char)(tbs.b1 << 4 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tfs.b3 & 0b11110000);
    }
    else
    {
        tfs = _bytes.F;
        trs = _bytes.R;
        tls = _bytes.L;
        tbs = _bytes.B;
        tus = _bytes.U;
        tds = _bytes.D;
        _bytes.F.b1 &= 0b11110000;
        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b4 &= 0b00001111;
        _bytes.F.b1 |= (unsigned char)(tus.b3 >> 4 & 0b00001111);
        _bytes.F.b2 |= (unsigned char)(tus.b4 >> 4 & 0b00001111);
        _bytes.F.b3 |= (unsigned char)(tds.b1 << 4 & 0b11110000);
        _bytes.F.b4 |= (unsigned char)(tds.b2 << 4 & 0b11110000);
        _bytes.F.b1 += mud;
        _bytes.F.b2 += mud;
        _bytes.F.b3 += mud;
        _bytes.F.b4 += mud;

        _bytes.U.b1 &= 0b11110000;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b4 &= 0b00001111;
        _bytes.U.b1 |= (unsigned char)(tls.b1 & 0b00001111);
        _bytes.U.b2 |= (unsigned char)(trs.b2 & 0b00001111);
        _bytes.U.b3 |= (unsigned char)(tls.b2 << 4 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(trs.b1 << 4 & 0b11110000);

        _bytes.B.b1 &= 0b11110000;
        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b4 &= 0b00001111;
        _bytes.B.b1 |= (unsigned char)(tds.b3 >> 4 & 0b00001111);
        _bytes.B.b2 |= (unsigned char)(tds.b4 >> 4 & 0b00001111);
        _bytes.B.b3 |= (unsigned char)(tus.b1 << 4 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(tus.b2 << 4 & 0b11110000);
        _bytes.B.b1 += mud;
        _bytes.B.b2 += mud;
        _bytes.B.b3 += mud;
        _bytes.B.b4 += mud;

        _bytes.D.b1 &= 0b11110000;
        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b4 &= 0b00001111;
        _bytes.D.b1 |= (unsigned char)(tls.b4 >> 4 & 0b00001111);
        _bytes.D.b2 |= (unsigned char)(trs.b3 >> 4 & 0b00001111);
        _bytes.D.b3 |= (unsigned char)(tls.b3 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(trs.b4 & 0b11110000);

        _bytes.R.b1 &= 0b11110000;
        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b4 &= 0b00001111;
        _bytes.R.b1 |= (unsigned char)(tfs.b2 & 0b00001111);
        _bytes.R.b2 |= (unsigned char)(tbs.b4 >> 4 & 0b00001111);
        _bytes.R.b3 |= (unsigned char)(tfs.b4 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tbs.b2 << 4 & 0b11110000);

        _bytes.L.b1 &= 0b11110000;
        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b4 &= 0b00001111;
        _bytes.L.b1 |= (unsigned char)(tbs.b3 >> 4 & 0b00001111);
        _bytes.L.b2 |= (unsigned char)(tfs.b1 & 0b00001111);
        _bytes.L.b3 |= (unsigned char)(tbs.b1 << 4 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tfs.b3 & 0b11110000);
    }
    _numIndex += _offset;
}

void Cube4Bit::AntiC()
{

    SideBytes tfs;
    SideBytes trs;
    SideBytes tls;
    SideBytes tbs;
    SideBytes tus;
    SideBytes tds;
    unsigned char mud = (unsigned char)(_nums[_numIndex]);
    if (_decryptMode)
    {
        _bytes.F.b1 -= mud;
        _bytes.F.b2 -= mud;
        _bytes.F.b3 -= mud;
        _bytes.F.b4 -= mud;

        _bytes.B.b1 -= mud;
        _bytes.B.b2 -= mud;
        _bytes.B.b3 -= mud;
        _bytes.B.b4 -= mud;

        tfs = _bytes.F;
        trs = _bytes.R;
        tls = _bytes.L;
        tbs = _bytes.B;
        tus = _bytes.U;
        tds = _bytes.D;
        _bytes.F.b1 &= 0b11110000;
        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b4 &= 0b00001111;
        _bytes.F.b1 |= (unsigned char)(tls.b2 & 0b00001111);
        _bytes.F.b2 |= (unsigned char)(trs.b1 & 0b00001111);
        _bytes.F.b3 |= (unsigned char)(tls.b4 & 0b11110000);
        _bytes.F.b4 |= (unsigned char)(trs.b3 & 0b11110000);

        _bytes.U.b1 &= 0b11110000;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b4 &= 0b00001111;
        _bytes.U.b1 |= (unsigned char)(tbs.b3 >> 4 & 0b00001111);
        _bytes.U.b2 |= (unsigned char)(tbs.b4 >> 4 & 0b00001111);
        _bytes.U.b3 |= (unsigned char)(tfs.b1 << 4 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(tfs.b2 << 4 & 0b11110000);

        _bytes.B.b1 &= 0b11110000;
        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b4 &= 0b00001111;
        _bytes.B.b1 |= (unsigned char)(tls.b3 >> 4 & 0b00001111);
        _bytes.B.b2 |= (unsigned char)(trs.b4 >> 4 & 0b00001111);
        _bytes.B.b3 |= (unsigned char)(tls.b1 << 4 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(trs.b2 << 4 & 0b11110000);

        _bytes.D.b1 &= 0b11110000;
        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b4 &= 0b00001111;
        _bytes.D.b1 |= (unsigned char)(tfs.b3 >> 4 & 0b00001111);
        _bytes.D.b2 |= (unsigned char)(tfs.b4 >> 4 & 0b00001111);
        _bytes.D.b3 |= (unsigned char)(tbs.b1 << 4 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(tbs.b2 << 4 & 0b11110000);

        _bytes.R.b1 &= 0b11110000;
        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b4 &= 0b00001111;
        _bytes.R.b1 |= (unsigned char)(tus.b4 >> 4 & 0b00001111);
        _bytes.R.b2 |= (unsigned char)(tus.b2 & 0b00001111);
        _bytes.R.b3 |= (unsigned char)(tds.b2 << 4 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tds.b4 & 0b11110000);

        _bytes.L.b1 &= 0b11110000;
        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b4 &= 0b00001111;
        _bytes.L.b1 |= (unsigned char)(tus.b1 & 0b00001111);
        _bytes.L.b2 |= (unsigned char)(tus.b3 >> 4 & 0b00001111);
        _bytes.L.b3 |= (unsigned char)(tds.b3 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tds.b1 << 4 & 0b11110000);
    }
    else
    {
        tfs = _bytes.F;
        trs = _bytes.R;
        tls = _bytes.L;
        tbs = _bytes.B;
        tus = _bytes.U;
        tds = _bytes.D;
        _bytes.F.b1 &= 0b11110000;
        _bytes.F.b2 &= 0b11110000;
        _bytes.F.b3 &= 0b00001111;
        _bytes.F.b4 &= 0b00001111;
        _bytes.F.b1 |= (unsigned char)(tls.b2 & 0b00001111);
        _bytes.F.b2 |= (unsigned char)(trs.b1 & 0b00001111);
        _bytes.F.b3 |= (unsigned char)(tls.b4 & 0b11110000);
        _bytes.F.b4 |= (unsigned char)(trs.b3 & 0b11110000);

        _bytes.U.b1 &= 0b11110000;
        _bytes.U.b2 &= 0b11110000;
        _bytes.U.b3 &= 0b00001111;
        _bytes.U.b4 &= 0b00001111;
        _bytes.U.b1 |= (unsigned char)(tbs.b3 >> 4 & 0b00001111);
        _bytes.U.b2 |= (unsigned char)(tbs.b4 >> 4 & 0b00001111);
        _bytes.U.b3 |= (unsigned char)(tfs.b1 << 4 & 0b11110000);
        _bytes.U.b4 |= (unsigned char)(tfs.b2 << 4 & 0b11110000);

        _bytes.B.b1 &= 0b11110000;
        _bytes.B.b2 &= 0b11110000;
        _bytes.B.b3 &= 0b00001111;
        _bytes.B.b4 &= 0b00001111;
        _bytes.B.b1 |= (unsigned char)(tls.b3 >> 4 & 0b00001111);
        _bytes.B.b2 |= (unsigned char)(trs.b4 >> 4 & 0b00001111);
        _bytes.B.b3 |= (unsigned char)(tls.b1 << 4 & 0b11110000);
        _bytes.B.b4 |= (unsigned char)(trs.b2 << 4 & 0b11110000);

        _bytes.D.b1 &= 0b11110000;
        _bytes.D.b2 &= 0b11110000;
        _bytes.D.b3 &= 0b00001111;
        _bytes.D.b4 &= 0b00001111;
        _bytes.D.b1 |= (unsigned char)(tfs.b3 >> 4 & 0b00001111);
        _bytes.D.b2 |= (unsigned char)(tfs.b4 >> 4 & 0b00001111);
        _bytes.D.b3 |= (unsigned char)(tbs.b1 << 4 & 0b11110000);
        _bytes.D.b4 |= (unsigned char)(tbs.b2 << 4 & 0b11110000);

        _bytes.R.b1 &= 0b11110000;
        _bytes.R.b2 &= 0b11110000;
        _bytes.R.b3 &= 0b00001111;
        _bytes.R.b4 &= 0b00001111;
        _bytes.R.b1 |= (unsigned char)(tus.b4 >> 4 & 0b00001111);
        _bytes.R.b2 |= (unsigned char)(tus.b2 & 0b00001111);
        _bytes.R.b3 |= (unsigned char)(tds.b2 << 4 & 0b11110000);
        _bytes.R.b4 |= (unsigned char)(tds.b4 & 0b11110000);

        _bytes.L.b1 &= 0b11110000;
        _bytes.L.b2 &= 0b11110000;
        _bytes.L.b3 &= 0b00001111;
        _bytes.L.b4 &= 0b00001111;
        _bytes.L.b1 |= (unsigned char)(tus.b1 & 0b00001111);
        _bytes.L.b2 |= (unsigned char)(tus.b3 >> 4 & 0b00001111);
        _bytes.L.b3 |= (unsigned char)(tds.b3 & 0b11110000);
        _bytes.L.b4 |= (unsigned char)(tds.b1 << 4 & 0b11110000);
        _bytes.F.b1 += mud;
        _bytes.F.b2 += mud;
        _bytes.F.b3 += mud;
        _bytes.F.b4 += mud;

        _bytes.B.b1 += mud;
        _bytes.B.b2 += mud;
        _bytes.B.b3 += mud;
        _bytes.B.b4 += mud;
    }
    _numIndex += _offset;
}

void Cube4Bit::MoveSideClockwise(SideBytes* side)
{
    SideBytes ts = *side;
    side->b1 = 0;
    side->b2 = 0;
    side->b3 = 0;
    side->b4 = 0;

    side->b1 |= (unsigned char)(ts.b3 >> 4 & 0b00001111);
    side->b1 |= (unsigned char)(ts.b2 & 0b11110000);

    side->b2 |= (unsigned char)(ts.b1 & 0b00001111);
    side->b2 |= (unsigned char)(ts.b4 << 4 & 0b11110000);

    side->b3 |= (unsigned char)(ts.b1 >> 4 & 0b00001111);
    side->b3 |= (unsigned char)(ts.b4 & 0b11110000);

    side->b4 |= (unsigned char)(ts.b3 & 0b00001111);
    side->b4 |= (unsigned char)(ts.b2 << 4 & 0b11110000);
}

void Cube4Bit::MoveSideCounterClockwise(SideBytes* side)
{
    SideBytes ts = *side;
    side->b1 = 0;
    side->b2 = 0;
    side->b3 = 0;
    side->b4 = 0;

    side->b1 |= (unsigned char)(ts.b2 & 0b00001111);
    side->b1 |= (unsigned char)(ts.b3 << 4 & 0b11110000);

    side->b2 |= (unsigned char)(ts.b4 >> 4 & 0b00001111);
    side->b2 |= (unsigned char)(ts.b1 & 0b11110000);

    side->b3 |= (unsigned char)(ts.b4 & 0b00001111);
    side->b3 |= (unsigned char)(ts.b1 << 4 & 0b11110000);

    side->b4 |= (unsigned char)(ts.b2 >> 4 & 0b00001111);
    side->b4 |= (unsigned char)(ts.b3 & 0b11110000);
}