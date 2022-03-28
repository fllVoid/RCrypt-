#include <windows.h>
#include <iomanip>
#include "ConsoleDialog.h"

ConsoleDialog::ConsoleDialog() {

	streambuf* buf = cout.rdbuf();
	ostream mout(buf);
	int i = 0;
	auto f = [&](double progress) {
		COORD p;
		p = getxy();
		p.X = 0;
		gotoxy(p);
		printf("\t\t\t\t\t\t");
		gotoxy(p);
		printf("Progress: %06.2lf %%.", progress * 100);
	};
	encoder = new RCubeEncoder(&mout, f);
}

ConsoleDialog::~ConsoleDialog() {
	delete encoder;
}

void ConsoleDialog::Start()
{
	const string forgotArgMessage = "You didn't enter an argument";
	const string failedMessage = "Failed to execute command";
	const string successMessage = "Done!";
	const string helpMessage = "\t'enc1' command to encrypt file in 1-bit mode: enc <filepath_(source)> <filepath_(result)> <key>\n\t'enc4' command to encrypt file in 4-bit mode: enc <filepath_(source)> <filepath_(result)> <key>\n\t'dec1' command to decrypt file in 1-bit mode: dec <filepath_(source)> <filepath_(result)> <key>\n\t'dec4' command to decrypt file in 4-bit mode: dec <filepath_(source)> <filepath_(result)> <key>";

	cout << "Welcome to RCrypt" << endl;
	while (true)
	{
		cout << ">>";
		string input;
		getline(cin, input);
		if (input.size() == 0)
			continue;
		auto command = split(input, " ");
		bool success = false;

		if (!command.at(0).compare("enc1") && command.size() == 4) {
			success = encoder->EncryptFile1BitMode(command.at(1), command.at(2), command.at(3));
		}
		else if (!command.at(0).compare("dec1") && command.size() == 4) {
			success = encoder->DecryptFile1BitMode(command.at(1), command.at(2), command.at(3));
		}
		else if (!command.at(0).compare("enc4") && command.size() == 4) {
			success = encoder->EncryptFile4BitMode(command.at(1), command.at(2), command.at(3));
		}
		else if (!command.at(0).compare("dec4") && command.size() == 4) {
			success = encoder->DecryptFile4BitMode(command.at(1), command.at(2), command.at(3));
		}
		else if (!command.at(0).compare("help")) {
			cout << helpMessage << endl;
			goto cont;
		}
		else if (!command.at(0).compare("clrscr")) {
			system("cls");
			goto cont;
		}
		else if (!command.at(0).compare("exit")) {
			cursorvisible(true);
			break;
		}
		else {
			cout << "Unknown command" << endl;
			goto cont;
		}
		if (success)
			cout << endl << endl << successMessage << endl;
		else
			cout << endl << endl << failedMessage << endl;
		cont:
		cursorvisible(true);
	}
}

void ConsoleDialog::gotoxy(COORD coord) {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

COORD ConsoleDialog::getxy()
{
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi))
	{
		return cbsi.dwCursorPosition;
	}
	else
	{
		// The function failed. Call GetLastError() for details.
		COORD invalid = { 0, 0 };
		return invalid;
	}
}


void ConsoleDialog::cursorvisible(bool visible)
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = visible;
	SetConsoleCursorInfo(consoleHandle, &info);
}

vector<string> ConsoleDialog::split(string input, string delimiter)
{
	vector<string> strs;
	size_t pos = 0;
	string token;
	while ((pos = input.find(delimiter)) != string::npos) {
		token = input.substr(0, pos);
		strs.push_back(token);
		input.erase(0, pos + delimiter.length());
	}
	strs.push_back(input);
	return strs;
}