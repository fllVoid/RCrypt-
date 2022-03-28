#pragma once
#include <vector>
#include <windows.h>
#include "RCubeEncoder.h"

class ConsoleDialog
{
public:
    ConsoleDialog();
    ~ConsoleDialog();

    void Start();

private:
    void gotoxy(COORD coord);
    COORD getxy();
    void cursorvisible(bool visible);
    vector<string> split(string input, string delimiter);

    RCubeEncoder *encoder;
};