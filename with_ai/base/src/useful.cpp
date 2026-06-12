#include "useful.h"
#include <string>
#include <random>
//----------------- Math -----------------

float genRandomFloat(const float& a, const float& b)
{
    static std::mt19937 rng(std::random_device{}());
    //static std::mt19937 rng(1);
    std::uniform_real_distribution<float> dist(a, b); 
    return dist(rng);
}
int genRandomInt(int a, int b) {
    if (a > b) {
        int temp = a;
        a = b;
        b = temp;
    }
    return a + rand() % (b - a + 1);
}




//----------------- console manipulation -----------------
CONSOLE_SCREEN_BUFFER_INFO getConsoleInfo(const bool& show)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO   info;
    if (!GetConsoleScreenBufferInfo(hConsole, &info))
    {
        std::cerr << "error while getting console info";
        exit(1);
    }
    if (show)
    {
        std::cout << "dwSize : x: " << info.dwSize.X << "y : " << info.dwSize.Y << "\n";
        std::cout << "dwCursorPosition : x:" << info.dwCursorPosition.X << "y:" << info.dwCursorPosition.Y << "\n";
        std::cout << "wAttributes : " << info.wAttributes << "\n";
        std::cout << "srWindow : top: " << info.srWindow.Top << " left: " << info.srWindow.Left << " bottom: " << info.srWindow.Bottom << " right: " << info.srWindow.Right << "\n";
        std::cout << "dwMaximumWindowSize : x: " << info.dwMaximumWindowSize.X << "y : " << info.dwMaximumWindowSize.Y << "\n";
    }
    return info;
}

void setConsoleToFullScreen()
{
    HWND Hwnd = GetForegroundWindow();
    //HWND Hwnd = GetConsoleWindow();
    ShowWindow(Hwnd, SW_MAXIMIZE);
}

void activateVP()
{
    DWORD consoleMode;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleMode(hConsole, &consoleMode))
    {
        SetConsoleMode(hConsole, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    else
    {
        std::cerr << "error while activating vp";
        exit(1);
    }
}

void setCursorPos(const int& x, const int& y) {
    std::cout << "\x1b[" << y << ";" << x << "H";
}
void setCursorPos(COORD pos) {
    std::cout << "\x1b[" << pos.Y << ";" << pos.X << "H";
}

void setConsoleWindowSize(const int& width, const int& height) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT sr;
    sr.Left = 0;
    sr.Top = 0;
    sr.Right = width - 1;
    sr.Bottom = height - 1;
    SetConsoleWindowInfo(hConsole, TRUE, &sr);
}

COORD getConsoleCenter()
{
    COORD size = { getConsoleInfo().srWindow.Right + 1 - getConsoleInfo().srWindow.Left, getConsoleInfo().srWindow.Bottom + 1 - getConsoleInfo().srWindow.Top };
    COORD center = { round(size.X / 2.0), round(size.Y / 2.0) };
    return center;
}

bool isConsoleResized() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    static int previousWidth = csbi.dwSize.X;
    static int previousHeight = csbi.dwSize.Y;
    if (previousWidth != csbi.dwSize.X || previousHeight != csbi.dwSize.Y) {
        previousWidth = csbi.dwSize.X;
        previousHeight = csbi.dwSize.Y;
        return true;
    }
    return false;
}

void clearConsole(const clearMode& x)
{
    if (x == RESET_CURSOR) std::cout << "\x1b[H";
    else if (x == RESET_ALL_CELL)
    {
        HANDLE                     hStdOut;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD                      count;
        DWORD                      cellCount;
        COORD                      homeCoords = { 0, 0 };

        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hStdOut == INVALID_HANDLE_VALUE) return;

        if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
        cellCount = csbi.dwSize.X * csbi.dwSize.Y;


        if (!FillConsoleOutputCharacter(hStdOut, (TCHAR)' ', cellCount, homeCoords, &count)) return;
        if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count)) return;

        SetConsoleCursorPosition(hStdOut, homeCoords);
    }
    else if (x == RESET_CURSOR_AND_FILL_WITH_SPACE)
    {
        std::string str = "";
        int width = getConsoleInfo().srWindow.Right;
        int height = getConsoleInfo().srWindow.Bottom;
        setCursorPos(1, 1);
        for (int i = 0; i <= height; i++)
        {
            str += std::string(width, ' ');
            if (i != height) str += "\n";
        }
        std::cout << str;
    }
    else if (x == SYSTEM_CLEAR)
    {
        system("cls");
    }
}

void clearConsoleInputBuffer()
{
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD numEvents;
    INPUT_RECORD* buffer;
    GetNumberOfConsoleInputEvents(handle, &numEvents);
    buffer = new INPUT_RECORD[numEvents];
    ReadConsoleInput(handle, buffer, numEvents, &numEvents);
    delete[] buffer;
}

void setAlternateBuffer()
{
    std::cout << "\x1b[?1049h";
}
void setMainBuffer()
{
    std::cout << "\x1b[?1049l";
}
void setCursorInvisible()
{
    std::cout << "\x1b[?25l";
}
void setCursorVisible()
{
    std::cout << "\x1b[?25h";
}




//------------------- display function -------------------
int getVisibleStringLenght(const std::string& text)
{
    int length = 0;
    bool escape = false;
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == '\x1B')
        {
            escape = true;
        }
        else if (escape && text[i] == 'm')
        {
            escape = false;
        }
        else if (!escape)
        {
            length++;
        }
    }
    return length;
}

std::string setColor(const int& r, const int& g, const int b)
{
    return "\x1b[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";

}
std::string resetColor()
{
    return "\x1b[0m";
}


void printCentered(const std::string& text, const int& xoffset, const int& yoffset, const SMALL_RECT& root) {
    COORD center;

    if (root.Left == -1)
    {
        center = getConsoleCenter();
    }
    else
    {
        center.X = root.Left + round((root.Right - root.Left) / 2.0);
        center.Y = root.Top + round((root.Bottom - root.Top) / 2.0);
    }

    int length = getVisibleStringLenght(text);
    int x = center.X - length / 2 + xoffset;
    int y = center.Y + yoffset;
    setCursorPos(x, y);
    std::cout << text;
}
void printCenteredX(const std::string& text, const int& xoffset, const int& y, const SMALL_RECT& root) {
    int _x;
    int _y;

    if (root.Left == -1)
    {
        _x = getConsoleCenter().X;
        _y = y;
    }
    else
    {
        _x = root.Left + round((root.Right - root.Left) / 2.0);
        _y = y + root.Top;
    }

    int length = getVisibleStringLenght(text);
    setCursorPos(_x - length / 2 + xoffset, _y);
    std::cout << text;
}
void printCenteredY(const std::string& text, const int& x, const int& yoffset, const SMALL_RECT& root) {
    int _x;
    int _y;

    if (root.Left == -1)
    {
        _x = x;
        _y = getConsoleCenter().Y;
    }
    else
    {
        _x = x + root.Left;
        _y = root.Top + round((root.Bottom - root.Top) / 2.0);
    }

    setCursorPos(_x, _y + yoffset);
    std::cout << text;
}
void printAtPos(const std::string& text, const int& x, const int& y, const SMALL_RECT& root)
{
    int _x;
    int _y;
    if (root.Left == -1)
    {
        _x = x;
        _y = y;
    }
    else
    {
        _x = root.Left + x;
        _y = root.Top + y;
    }

    setCursorPos(_x, _y);
    std::cout << text;
}
void printAtPos(const char& text, const int& x, const int& y, const SMALL_RECT& root)
{
    int _x;
    int _y;

    if (root.Left == -1)
    {
        _x = x;
        _y = y;
    }
    else
    {
        _x = root.Left + x;
        _y = root.Top + y;
    }

    setCursorPos(_x, _y);
    std::cout << text;
}

SMALL_RECT drawRectangle(const int& width, const int& height, const int& r, const int& g, const int& b)
{
    COORD center = getConsoleCenter();
    SMALL_RECT rect;
    rect.Left = round(center.X - (width / 2.0));
    rect.Right = round(center.X + (width / 2.0) - 1);
    rect.Top = round(center.Y - (height / 2.0));
    rect.Bottom = round(center.Y + (height / 2.0) - 1);

    if (rect.Top < 1)
    {
        int yoffset = 1 - rect.Top;
        rect.Top = 1;
        rect.Bottom += yoffset;
    }

    if (width < 2 || height < 2)
    {
        std::cerr << "rectangle size too small";
        exit(1);
    }

    const char* borderCharH = u8"─";
    const char* borderCharV = u8"│";

    setCursorPos(1, 1);
    std::cout << "\x1b[38;2;" << r << ";" << g << ";" << b << "m";

    std::string topHorizontalBorder;
    topHorizontalBorder += u8"┌";
    for (int i = 1; i < width-1; ++i) {
        topHorizontalBorder += borderCharH;
    }
    topHorizontalBorder += u8"┐";

    std::string bottomHorizontalBorder;
    bottomHorizontalBorder += u8"└";
    for (int i = 1; i < width - 1; ++i) {
        bottomHorizontalBorder += borderCharH;
    }
    bottomHorizontalBorder += u8"┘";

    printAtPos(topHorizontalBorder, rect.Left, rect.Top);
    for (int i = rect.Top + 1; i < rect.Bottom; i++)
    {
        printAtPos(borderCharV, rect.Left, i);
        printAtPos(borderCharV, rect.Right, i);
    }
    printAtPos(bottomHorizontalBorder, rect.Left, rect.Bottom);
    std::cout << "\x1b[0m";
    return rect;
}
COORD getRectCenter(const SMALL_RECT& rect)
{
    COORD center;
    center.X = rect.Left + (rect.Right - rect.Left) / 2;
    center.Y = rect.Top + (rect.Bottom - rect.Top) / 2;
    return center;
}

