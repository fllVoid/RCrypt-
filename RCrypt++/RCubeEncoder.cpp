#include <vector>
#include <io.h>
#include <fstream>
#include <thread>
#include "RCubeEncoder.h"
#include "Cube4Bit.h"
#include "Cube1Bit.h"
#include "sha256.h"

const string RCubeEncoder::_movesLetters = "BCDEFLMRSU";

RCubeEncoder::RCubeEncoder(ostream* output, function<void(double)> printProgress) {
	_output = output;
	_printProgress = printProgress;
}

bool RCubeEncoder::EncryptFile4BitMode(string sourcePath, string resultPath, string key)
{
	keyAndSeed tupleKeySeed = HandleKey(key);
	CubeBytes cubeBytes;
	Cube4Bit c4(false, tupleKeySeed.seed, tupleKeySeed.key);
	bool cancelSource = false;
	char* buffer = new char[_bufferSize] {0};
	int blocksize = c4.blockSize;
	char* block = new char[blocksize] {0};
	thread* t = nullptr;
	try
	{
		if (FileExists(resultPath.c_str()))
			remove(resultPath.c_str());
		const long long filesize = GetFileSize(sourcePath);
		long long bytesLeft = filesize;

		auto prntprogress = [&]() {
			while (!cancelSource) {
				_printProgress((filesize - bytesLeft) / (double)filesize);
				this_thread::sleep_for(chrono::milliseconds(100));
			}
			_printProgress((filesize - bytesLeft) / (double)filesize);
		};
		ifstream in;
		ofstream out;
		if (!FileExists(sourcePath.c_str()))
			throw runtime_error("Error: Source file does not exist.");
		in.open(sourcePath, ios_base::binary);
		out.open(resultPath, ios_base::binary);
		t = new thread(prntprogress);
		while (bytesLeft > 0) {
			long long read;
			if (bytesLeft >= _bufferSize) {
				read = _bufferSize;
				in.read(buffer, _bufferSize);
			}
			else {
				read = bytesLeft;
				in.read(buffer, bytesLeft);
			}

			for (int i = 0; i < read; i += blocksize) {
				int j;
				for (j = 0; j < blocksize && i + j < read; j++)
					block[j] = buffer[i + j];
				InitBlock(&cubeBytes, block);
				c4.Init(&cubeBytes);
				out.write(c4.DoScramble(), blocksize);
				if (bytesLeft == blocksize || j != blocksize) {
					for (int i = 0; i < 23; i++)
						block[i] = 255;
					block[23] = (char)(blocksize - j);
					InitBlock(&cubeBytes, block);
					c4.Init(&cubeBytes);
					char* tmp = c4.DoScramble();
					out.write(tmp, blocksize);
				}
				bytesLeft -= j;
			}
		}
		cancelSource = true;
		in.close();
		out.close();
	}
	catch (const exception ex)
	{
		cancelSource = true;
		if (t) {
			t->join();
			delete t;
		}
		cout << ex.what();
		delete[] block;
		delete[] buffer;
		return false;
	}
	if (t) {
		t->join();
		delete t;
	}
	delete[] block;
	delete[] buffer;
	return true;
}

bool RCubeEncoder::DecryptFile4BitMode(string sourcePath, string resultPath, string key)
{
	keyAndSeed tupleKeySeed = HandleKey(key);
	CubeBytes cubeBytes;
	Cube4Bit c4(true, tupleKeySeed.seed, tupleKeySeed.key);
	bool cancelSource = false;
	char* buffer = new char[_bufferSize] {0};
	int blocksize = c4.blockSize;
	char* block = new char[blocksize] {0};
	thread* t = nullptr;

	try
	{
		if (FileExists(resultPath.c_str()))
			remove(resultPath.c_str());
		const long long filesize = GetFileSize(sourcePath);
		long long bytesLeft = filesize;

		auto prntprogress = [&]() {
			while (!cancelSource) {
				_printProgress((filesize - bytesLeft) / (double)filesize);
				this_thread::sleep_for(chrono::milliseconds(100));
			}
			_printProgress((filesize - bytesLeft) / (double)filesize);
		};
		ifstream in;
		ofstream out;
		in.open(sourcePath, ios_base::binary);
		out.open(resultPath, ios_base::binary);
		t = new thread(prntprogress);
		while (bytesLeft > 0) {
			long long read;
			if (bytesLeft >= _bufferSize) {
				read = _bufferSize;
				in.read(buffer, _bufferSize);
			}
			else {
				read = bytesLeft;
				in.read(buffer, bytesLeft);
			}

			for (int i = 0; i < read; i += blocksize) {
				bytesLeft -= blocksize;
				int j;
				for (j = 0; j < blocksize && i + j < read; j++)
					block[j] = buffer[i + j];
				InitBlock(&cubeBytes, block);
				c4.Init(&cubeBytes);
				c4.DoScramble(block);
				if (bytesLeft - blocksize > 0)
					out.write(block, blocksize);
				else {
					char* lastPeace = new char[blocksize] {0};
					for (int k = 0; k < blocksize && i + k < read; k++)
						lastPeace[k] = buffer[i + j + k];
					InitBlock(&cubeBytes, lastPeace);
					c4.Init(&cubeBytes);
					c4.DoScramble(lastPeace);
					int bytesToWrite;
					if ((lastPeace[23] & 0b11100000) != 0)
						bytesToWrite = 0;
					else
						bytesToWrite = blocksize - lastPeace[23] % 25;
					out.write(block, bytesToWrite);
					bytesLeft = 0;
					delete[] lastPeace;
				}
			}
		}
		cancelSource = true;
		in.close();
		out.close();
	}
	catch (const exception ex)
	{
		cancelSource = true;
		if (t) {
			t->join();
			delete t;
		}
		cout << ex.what();
		delete[] block;
		delete[] buffer;
		return false;
	}
	if (t) {
		t->join();
		delete t;
	}
	delete[] block;
	delete[] buffer;
	return true;
}

bool RCubeEncoder::EncryptFile1BitMode(string sourcePath, string resultPath, string key)
{
	keyAndSeed tupleKeySeed = HandleKey(key);
	Cube1Bit c1(false, tupleKeySeed.seed, tupleKeySeed.key);
	bool cancelSource = false;
	char* buffer = new char[_bufferSize] {0};
	int blocksize = c1.blockSize;
	char* block;
	block = new char[blocksize] {0};
	thread* t = nullptr;
	try
	{
		if (FileExists(resultPath.c_str()))
			remove(resultPath.c_str());
		const long long filesize = GetFileSize(sourcePath);
		long long bytesLeft = filesize;

		auto prntprogress = [&]() {
			while (!cancelSource) {
				_printProgress((filesize - bytesLeft) / (double)filesize);
				this_thread::sleep_for(chrono::milliseconds(100));
			}
			_printProgress((filesize - bytesLeft) / (double)filesize);
		};
		ifstream in;
		ofstream out;
		in.open(sourcePath, ios_base::binary);
		out.open(resultPath, ios_base::binary);
		t = new thread(prntprogress);
		c1.Init(block);
		while (bytesLeft > 0) {
			long long read;
			if (bytesLeft >= _bufferSize) {
				read = _bufferSize;
				in.read(buffer, _bufferSize);
			}
			else {
				read = bytesLeft;
				in.read(buffer, bytesLeft);
			}

			for (int i = 0; i < read; i += blocksize) {
				int j;
				for (j = 0; j < blocksize && i + j < read; j++)
					block[j] = buffer[i + j];
				out.write(c1.DoScramble(), blocksize);
				if (bytesLeft == blocksize || j != blocksize) {
					for (int i = 0; i < 5; i++)
						block[i] = 255;
					block[5] = (char)(blocksize - j);
					c1.DoScramble();
					out.write(block, blocksize);
				}
				bytesLeft -= j;
			}
		}
		cancelSource = true;
		in.close();
		out.close();
	}
	catch (const exception ex)
	{
		cancelSource = true;
		if (t) {
			t->join();
			delete t;
		}
		cout << ex.what();
		delete[] block;
		delete[] buffer;
		return false;
	}
	if (t) {
		t->join();
		delete t;
	}
	delete[] block;
	delete[] buffer;
	return true;
}

bool RCubeEncoder::DecryptFile1BitMode(string sourcePath, string resultPath, string key)
{
	keyAndSeed tupleKeySeed = HandleKey(key);
	Cube1Bit c1(true, tupleKeySeed.seed, tupleKeySeed.key);
	bool cancelSource = false;
	char* buffer = new char[_bufferSize] {0};
	int blocksize = c1.blockSize;
	char* block = new char[blocksize] {0};
	thread* t = nullptr;

	try
	{
		if (FileExists(resultPath.c_str()))
			remove(resultPath.c_str());
		const long long filesize = GetFileSize(sourcePath);
		long long bytesLeft = filesize;

		auto prntprogress = [&]() {
			while (!cancelSource) {
				_printProgress((filesize - bytesLeft) / (double)filesize);
				this_thread::sleep_for(chrono::milliseconds(100));
			}
			_printProgress((filesize - bytesLeft) / (double)filesize);
		};
		ifstream in;
		ofstream out;
		in.open(sourcePath, ios_base::binary);
		out.open(resultPath, ios_base::binary);
		t = new thread(prntprogress);
		c1.Init(block);
		while (bytesLeft > 0) {
			long long read;
			if (bytesLeft >= _bufferSize) {
				read = _bufferSize;
				in.read(buffer, _bufferSize);
			}
			else {
				read = bytesLeft;
				in.read(buffer, bytesLeft);
			}

			for (int i = 0; i < read; i += blocksize) {
				bytesLeft -= blocksize;
				int j;
				for (j = 0; j < blocksize && i + j < read; j++)
					block[j] = buffer[i + j];
				c1.DoScramble();
				if (bytesLeft - blocksize > 0)
					out.write(block, blocksize);
				else {
					char* lastPeace = new char[blocksize] {0};
					for (int k = 0; k < blocksize && i + k < read; k++)
						lastPeace[k] = buffer[i + j + k];
					c1.Init(lastPeace);
					c1.DoScramble();
					int bytesToWrite;
					if ((lastPeace[5] & 0b11111000) != 0)
						bytesToWrite = 0;
					else
						bytesToWrite = blocksize - lastPeace[5] % 7;
					out.write(block, bytesToWrite);
					bytesLeft = 0;
					delete[] lastPeace;
				}
			}
		}
		cancelSource = true;
		in.close();
		out.close();
	}
	catch (const exception ex)
	{
		cancelSource = true;
		if (t) {
			t->join();
			delete t;
		}
		cout << ex.what();
		delete[] block;
		delete[] buffer;
		return false;
	}
	if (t) {
		t->join();
		delete t;
	}
	delete[] block;
	delete[] buffer;
	return true;
}

keyAndSeed RCubeEncoder::HandleKey(string key)
{
	string hex = sha256(key);
	string hash = HexToDec(hex);
	vector<char> result;
	for (int i = 0; i < _keySize; ++i)
	{
		result.push_back(_movesLetters[hash[i] - '0']);
		if (hash[hash.length() - 1 - i] > '4')
			result.push_back('\'');
	}
	string seedSource = hash.substr(_keySize, hash.length() - _keySize * 2);
	unsigned long long l1 = stoull(seedSource.substr(0, 19));
	unsigned long long l2 = stoull(seedSource.substr(19));

	auto seed = l1 * l2;

	keyAndSeed ks{ string(result.begin(), result.end()) , seed };
	return ks;
}

string RCubeEncoder::HexToDec(string hex) {
	int base = 16;
	int pow = hex.length() - 1;
	int nol = 0;
	LongNumObj bignum = LongNumObj::toLN(nol);
	LongNumObj tmpnum = LongNumObj::toLN(nol);
	for (int i = 0; i < hex.length(); i++) {
		LongNumObj lpow = LongNumObj::toLN(base);
		LongNumObj::LNpow(&lpow, pow);
		pow--;
		switch (hex[i])
		{
		case '0':
			break;
		case '1':
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case '2':
			nol = 2;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case '3':
			nol = 3;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case '4':
			nol = 4;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case '5':
			nol = 5;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case '6':
			nol = 6;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case '7':
			nol = 7;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case '8':
			nol = 8;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case '9':
			nol = 9;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case 'A':
		case 'a':
			nol = 10;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));

			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case 'B':
		case 'b':
			nol = 11;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case 'C':
		case 'c':
			nol = 12;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case 'D':
		case 'd':
			nol = 13;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case 'E':
		case 'e':
			nol = 14;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		case 'F':
		case 'f':
			nol = 15;
			LongNumObj::LNmulLN(&lpow, LongNumObj::toLN(nol));
			LongNumObj::LNaddLN(&bignum, lpow);
			break;
		default:
			break;
		}
	}
	return bignum.LNtoStr();
}

bool RCubeEncoder::FileExists(const char* fname)
{
	return _access(fname, 0) != -1;
}

long long RCubeEncoder::GetFileSize(string file) {
	std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

void RCubeEncoder::InitBlock(CubeBytes *bytes, char* block) {
	bytes->U.b1 = block[0];
	bytes->U.b2 = block[1];
	bytes->U.b3 = block[2];
	bytes->U.b4 = block[3];

	bytes->L.b1 = block[4];
	bytes->L.b2 = block[5];
	bytes->L.b3 = block[6];
	bytes->L.b4 = block[7];

	bytes->F.b1 = block[8];
	bytes->F.b2 = block[9];
	bytes->F.b3 = block[10];
	bytes->F.b4 = block[11];

	bytes->R.b1 = block[12];
	bytes->R.b2 = block[13];
	bytes->R.b3 = block[14];
	bytes->R.b4 = block[15];

	bytes->B.b1 = block[16];
	bytes->B.b2 = block[17];
	bytes->B.b3 = block[18];
	bytes->B.b4 = block[19];

	bytes->D.b1 = block[20];
	bytes->D.b2 = block[21];
	bytes->D.b3 = block[22];
	bytes->D.b4 = block[23];
}