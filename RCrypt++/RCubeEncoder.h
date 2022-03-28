#pragma once
#include <iostream>
#include <string>
#include <functional>


#include "lnclass.h"
#include "keyAndSeed.h"
#include "CubeBytes.h"

using namespace std;
class RCubeEncoder
{
public:
    RCubeEncoder(ostream* output, function<void(double)> printProgress);

    bool EncryptFile4BitMode(string sourcePath, string resultPath, string key);
    bool DecryptFile4BitMode(string sourcePath, string resultPath, string key);

    bool EncryptFile1BitMode(string sourcePath, string resultPath, string key);
    bool DecryptFile1BitMode(string sourcePath, string resultPath, string key);
private:
    static const int _keySize = 24;
	static const int _bufferSize = 4800;
    static const string _movesLetters;
	ostream *_output;
	function <void(double)> _printProgress;

    keyAndSeed HandleKey(string key);

    string HexToDec(string hex);

    void PrintProgress(bool* isCancellationRequsted);

    bool FileExists(const char* fname);

    void InitBlock(CubeBytes *bytes, char* block);

    long long GetFileSize(string file);
};

