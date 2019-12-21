#include "shader.h"
#include "model.h"
#include "camera.h"

#include <stb_image.h>
#include <helpers/RootDir.h>
                                                        //  define all required GLFW callback functions
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow * window);
unsigned int loadTexture(const char * path);

glm::vec3 rgbToFloats(int red, int green, int blue)     //  convert integer RGB value to appropriate float RGB value
{
    float newRed = float(red) / 255.0;
    float newGreen = float(green) / 255.0;
    float newBlue = float(blue) / 255.0;

    return glm::vec3(newRed, newGreen, newBlue);        //  return value as a vector
}

unsigned int window_width = 1024;                       //  initial width of window
unsigned int window_height = 600;                       //  initial height of window
glm::vec3 backgroundColor(rgbToFloats(0, 0, 0));        //  window background color

unique_ptr<Camera> cameraPtr(new Camera);               //  create camera instance and use default camera position

bool lampOrbit = false;                                 //  holds if lamp orbit is active
bool lampVisible = false;                               //  holds lamp object visibility
unsigned activeLamp = 0;

glm::vec3 lightPositionOne(-5.0f, 5.0f, 0.0f);
glm::vec3 lightPositionTwo(-5.0f, 5.0f, -6.0f);
glm::vec3 lightPositionThree(-5.0f, 5.0f, 6.0f);
glm::vec3 lightPositionFour(-3.5f, 10.0f, -0.0f);
glm::vec3 lightColorOne(150.0f, 150.0f, 150.0f);
glm::vec3 lightColorTwo(150.0f, 150.0f, 150.0f);
glm::vec3 lightColorThree(150.0f, 150.0f, 150.0f);
glm::vec3 lightColorFour(150.0f, 150.0f, 150.0f);

const int objectCount = 96;                             //  holds number of game objects(game board and figurines)
glm::vec3 positions[objectCount + 36];                  //  positions of all game objects(includes game board border)
bool isWhite[objectCount];                              //  colors of all game objects(includes game board border)
int highlight[objectCount];                             //  highlight indiced of all game objects

int clicked = -1;                                       //  holds actual index of mouse click selection

class Chess                                             //  represents implementation of chess game logic
{
private:
    bool blackTurn = false;                             //  holds if black turns on the turn

    string board[8];                                    //  holds board indices which indicates color move highlights
    string figurines[8];                                //  holds indices of all figurines
    string temp[8];                                     //  generic array used as a temporal storage
    string linear;                                      //  holds type of figurine on concrete
    string real;                                        //  holds real type of figurine places on a linear coordinate

    int selection[2];                                   //  holds selection coordinates of highlighted figure
    int lastSelection[2];                               //  holds destination coordinates of last movement
    int promotion = -1;                                 //  holds index of promoted pawn on the border line of game board

    bool blackCastlingKingside = true;                  //  holds if kingside castling is allowed due to the initial positions of figurines
    bool blackCastlingQueenside = true;                 //  holds if queenside castling is allowed due to the initial positions of figurines

    bool whiteCastlingKingside = true;                  //  holds if kingside castling is allowed due to the initial positions of figurines
    bool whiteCastlingQueenside = true;                 //  holds if queenside castling is allowed due to the initial positions of figurines

public:
    bool animationActive = false;                       //  tells if camera movement needs to be animated
    double previous = 0.0f;                             //  holds last saved timestamp value
    double actual = 0.0f;                               //  holds actual timestamp value

    Chess()                                             //  one and only class constructor
    {
        board[7] = "........";                          //  bwbwbwbw
        board[6] = "........";                          //  wbwbwbwb
        board[5] = "........";                          //  bwbwbwbw
        board[4] = "........";                          //  wbwbwbwb
        board[3] = "........";                          //  bwbwbwbw
        board[2] = "........";                          //  wbwbwbwb
        board[1] = "........";                          //  bwbwbwbw
        board[0] = "........";                          //  wbwbwbwb

        figurines[7] = "ph....HP";                      //  B  rp....PR  W
        figurines[6] = "og....GO";                      //  B  np....PN  W
        figurines[5] = "nf....FN";                      //  B  bp....PB  W
        figurines[4] = "me....EM";                      //  B  kp....PK  W
        figurines[3] = "ld....DL";                      //  B  qp....PQ  W
        figurines[2] = "kc....CK";                      //  B  bp....PB  W
        figurines[1] = "jb....BJ";                      //  B  np....PN  W
        figurines[0] = "ia....AI";                      //  B  rp....PR  W

        // figurines[7] = "p......P";                      //  B  rp....PR  W
        // figurines[6] = "........";                      //  B  np....PN  W
        // figurines[5] = "........";                      //  B  bp....PB  W
        // figurines[4] = "m......M";                      //  B  kp....PK  W
        // figurines[3] = "........";                      //  B  qp....PQ  W
        // figurines[2] = "........";                      //  B  bp....PB  W
        // figurines[1] = "........";                      //  B  np....PN  W
        // figurines[0] = "i......I";                      //  B  rp....PR  W

        linear = "ppppppppPPPPPPPPrnbqkbnrRNBQKBNR";
        real = "abcdefghABCDEFGHijklmnopIJKLMNOP";

        // linear = "................r...k..rR...K..R";
        // real = "abcdefghABCDEFGHijklmnopIJKLMNOP";

        selection[0] = selection[1] = -1;
        lastSelection[0] = lastSelection[1] = -1;
    }

    string getLinear()                                  //  linear string getter
    {
        return linear;
    }

    char toLinear(char figurine)                        //  get linear value of desired figurine
    {
        for (int i = 0; i < 32; i++)
        {
            if (real[i] == figurine)
                return linear[i];
        }

        return '.';
    }

    bool isUnderAttack(int index, int & i, int & j)     //  check if figurine on index is under the attack
    {
        index -= 64;                                    //  ignore game board blocks
        char ch = real[index];                          //  use mapping of index to real type of figurine

        for (j = 0; j < 8; j++)                         //  iterate through figurines array
        {
            for (i = 0; i < 8; i++)
            {
                if (ch == figurines[i][j])              //  concrete figurine found inside array
                {
                    if (board[i][j] == 'A')
                        return true;                    //  figurine is under attack

                    break;
                }
            }
        }

        return false;                                   //  figurine is not under attack
    }

    bool calculateCheck()                               //  check if king is under attack
    {
        if (blackTurn)
        {
            int kingX = 0;
            int kingY = 0;

            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)             //  iterate through figurines array
                {
                    temp[i][j] = toLinear(figurines[i][j]);

                    if (temp[i][j] == 'k')              //  king figurine has been found
                    {
                        kingX = j;                      //  save coordinates
                        kingY = i;

                        break;
                    }
                }
            }

            int x = kingX;
            int y = kingY;

            //  check horizontal or vertical attack

            while (x > 0)                               //  handle border of the game board
            {
                x--;

                if (temp[y][x] == 'Q' || temp[y][x] == 'R')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;

            while (x < 7)                               //  handle border of the game board
            {
                x++;

                if (temp[y][x] == 'Q' || temp[y][x] == 'R')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;

            while (y > 0)                               //  handle border of the game board
            {
                y--;

                if (temp[y][x] == 'Q' || temp[y][x] == 'R')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            y = kingY;

            while (y < 7)                               //  handle border of the game board
            {
                y++;

                if (temp[y][x] == 'Q' || temp[y][x] == 'R')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            y = kingY;

            //  check diagonal attack

            while (x > 0 && y > 0)                      //  handle border of the game board
            {
                x--;
                y--;

                if (temp[y][x] == 'Q' || temp[y][x] == 'B')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;
            y = kingY;

            while (x > 0 && y < 7)                      //  handle border of the game board
            {
                x--;
                y++;

                if (temp[y][x] == 'Q' || temp[y][x] == 'B')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;
            y = kingY;

            while (x < 7 && y > 0)                      //  handle border of the game board
            {
                x++;
                y--;

                if (temp[y][x] == 'Q' || temp[y][x] == 'B')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;
            y = kingY;

            while (x < 7 && y < 7)                      //  handle border of the game board
            {
                x++;
                y++;

                if (temp[y][x] == 'Q' || temp[y][x] == 'B')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;
            y = kingY;

            //  check attack by knight

            if (y && (x + 1) < 7 && temp[y - 1][x + 2] == 'N')
                return true;

            if (y < 7 && (x + 1) < 7 && temp[y + 1][x + 2] == 'N')
                return true;

            if (y && (x - 1) > 0 && temp[y - 1][x - 2] == 'N')
                return true;

            if (y < 7 && (x - 1) > 0 && temp[y + 1][x - 2] == 'N')
                return true;

            if ((y + 1) < 7 && x && temp[y + 2][x - 1] == 'N')
                return true;

            if ((y + 1) < 7 && x < 7 && temp[y + 2][x + 1] == 'N')
                return true;

            if ((y - 1) > 0 && x && temp[y - 2][x - 1] == 'N')
                return true;

            if ((y - 1) > 0 && x < 7 && temp[y - 2][x + 1] == 'N')
                return true;

            //  check attack by king

            if (y && temp[y - 1][x] == 'K')
                return true;

            if (y < 7 && temp[y + 1][x] == 'K')
                return true;

            if (x && temp[y][x - 1] == 'K')
                return true;

            if (x < 7 && temp[y][x + 1] == 'K')
                return true;

            if (y && x && temp[y - 1][x - 1] == 'K')
                return true;

            if (y && x < 7 && temp[y - 1][x + 1] == 'K')
                return true;

            if (y < 7 && x && temp[y + 1][x - 1] == 'K')
                return true;

            if (y < 7 && x < 7 && temp[y + 1][x + 1] == 'K')
                return true;

            //  check attack by pawn

            if (y && x < 7 && temp[y - 1][x + 1] == 'P')
                return true;

            if (y < 7 && x < 7 && temp[y + 1][x + 1] == 'P')
                return true;
        }
        else
        {
            int kingX = 0;
            int kingY = 0;

            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)             //  iterate through figurines array
                {
                    temp[i][j] = toLinear(figurines[i][j]);

                    if (temp[i][j] == 'K')              //  king figurine has been found
                    {
                        kingX = j;                      //  save coordinates
                        kingY = i;

                        break;
                    }
                }
            }

            int x = kingX;
            int y = kingY;

            //  check horizontal or vertical attack

            while (x > 0)                               //  handle border of the game board
            {
                x--;

                if (temp[y][x] == 'q' || temp[y][x] == 'r')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;

            while (x < 7)                               //  handle border of the game board
            {
                x++;

                if (temp[y][x] == 'q' || temp[y][x] == 'r')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;

            while (y > 0)                               //  handle border of the game board
            {
                y--;

                if (temp[y][x] == 'q' || temp[y][x] == 'r')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            y = kingY;

            while (y < 7)                               //  handle border of the game board
            {
                y++;

                if (temp[y][x] == 'q' || temp[y][x] == 'r')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            y = kingY;

            //  check diagonal attack

            while (x > 0 && y > 0)                      //  handle border of the game board
            {
                x--;
                y--;

                if (temp[y][x] == 'q' || temp[y][x] == 'b')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;
            y = kingY;

            while (x > 0 && y < 7)                      //  handle border of the game board
            {
                x--;
                y++;

                if (temp[y][x] == 'q' || temp[y][x] == 'b')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;
            y = kingY;

            while (x < 7 && y > 0)                      //  handle border of the game board
            {
                x++;
                y--;

                if (temp[y][x] == 'q' || temp[y][x] == 'b')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;
            y = kingY;

            while (x < 7 && y < 7)                      //  handle border of the game board
            {
                x++;
                y++;

                if (temp[y][x] == 'q' || temp[y][x] == 'b')     //  king is under attack
                    return true;
                else if (temp[y][x] != '.')             //  blocking figurine has been found
                    break;
            }

            x = kingX;
            y = kingY;

            //  check attack by knight

            if (y && (x + 1) < 7 && temp[y - 1][x + 2] == 'n')
                return true;

            if (y < 7 && (x + 1) < 7 && temp[y + 1][x + 2] == 'n')
                return true;

            if (y && (x - 1) > 0 && temp[y - 1][x - 2] == 'n')
                return true;

            if (y < 7 && (x - 1) > 0 && temp[y + 1][x - 2] == 'n')
                return true;

            if ((y + 1) < 7 && x && temp[y + 2][x - 1] == 'n')
                return true;

            if ((y + 1) < 7 && x < 7 && temp[y + 2][x + 1] == 'n')
                return true;

            if ((y - 1) > 0 && x && temp[y - 2][x - 1] == 'n')
                return true;

            if ((y - 1) > 0 && x < 7 && temp[y - 2][x + 1] == 'n')
                return true;

            //  check attack by king

            if (y && temp[y - 1][x] == 'k')
                return true;

            if (y < 7 && temp[y + 1][x] == 'k')
                return true;

            if (x && temp[y][x - 1] == 'k')
                return true;

            if (x < 7 && temp[y][x + 1] == 'k')
                return true;

            if (y && x && temp[y - 1][x - 1] == 'k')
                return true;

            if (y && x < 7 && temp[y - 1][x + 1] == 'k')
                return true;

            if (y < 7 && x && temp[y + 1][x - 1] == 'k')
                return true;

            if (y < 7 && x < 7 && temp[y + 1][x + 1] == 'k')
                return true;

            //  check attack by pawn

            if (y && x && temp[y - 1][x - 1] == 'p')
                return true;

            if (y < 7 && x && temp[y + 1][x - 1] == 'p')
                return true;
        }

        return false;
    }

    void refreshBoard(int index)                        //  handle click on concrete figurine with click index attribute
    {
        for (int j = 0; j < 8; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                board[i][j] = '.';                      //  clear previous state of game board highlights
            }
        }

        index -= 64;                                    //  ignore game board blocks
        char ch = real[index];                          //  use mapping of index to real type of figurine

        for (int j = 0; j < 8; j++)                     //  iterate through figurines array
        {
            for (int i = 0; i < 8; i++)
            {
                if (ch == figurines[i][j])              //  concrete figurine found inside array
                {
                    selection[0] = i;                   //  markup actual figurine selection
                    selection[1] = j;

                    ch = linear[index];                 //  use mapping to linear type of figurine

                    if (ch == 'p')                      //  black pawn selected by mouse
                    {
                        if (j == 1)                     //  pawn can move by two blocks on the first move
                        {
                            if (j < 6 && figurines[i][j + 1] == '.' && figurines[i][j + 2] == '.')
                            {
                                board[i][j + 2] = 'M';  //  move by two blocks
                            }
                        }

                        if (j < 7 && figurines[i][j + 1] == '.')
                        {
                            board[i][j + 1] = 'M';      //  move by one block
                        }

                        if (i && j < 7 && figurines[i - 1][j + 1] != '.' && isupper(figurines[i - 1][j + 1]))
                        {
                            board[i - 1][j + 1] = 'A';  //  attack
                        }

                        if (i < 7 && j < 7 && figurines[i + 1][j + 1] != '.' && isupper(figurines[i + 1][j + 1]))
                        {
                            board[i + 1][j + 1] = 'A';  //  attack
                        }

                        if (j == 4)
                        {
                            if (i && toLinear(figurines[i - 1][j]) == 'P')
                            {
                                if (i - 1 == lastSelection[0] && j == lastSelection[1])
                                    board[i - 1][j] = 'A';  //  En passant rule
                            }
                            else if (i < 7 && toLinear(figurines[i + 1][j]) == 'P')
                            {
                                if (i + 1 == lastSelection[0] && j == lastSelection[1])
                                    board[i + 1][j] = 'A';  //  En passant rule
                            }
                        }
                    }
                    else if (ch == 'P')                 //  white pawn
                    {
                        if (j == 6)                     //  pawn can move by two blocks on the first move
                        {
                            if (j - 2 >= 0 && figurines[i][j - 1] == '.' && figurines[i][j - 2] == '.')
                            {
                                board[i][j - 2] = 'M';  //  move by two blocks
                            }
                        }

                        if (j && figurines[i][j - 1] == '.')
                        {
                            board[i][j - 1] = 'M';      //  move
                        }

                        if (i && j && figurines[i - 1][j - 1] != '.' && islower(figurines[i - 1][j - 1]))
                        {
                            board[i - 1][j - 1] = 'A';  //  attack
                        }

                        if (i < 7 && j && figurines[i + 1][j - 1] != '.' && islower(figurines[i + 1][j - 1]))
                        {
                            board[i + 1][j - 1] = 'A';  //  attack
                        }

                        if (j == 3)
                        {
                            if (i && toLinear(figurines[i - 1][j]) == 'p')
                            {
                                if (i - 1 == lastSelection[0] && j == lastSelection[1])
                                    board[i - 1][j] = 'A';  //  En passant rule
                            }
                            else if (i < 7 && toLinear(figurines[i + 1][j]) == 'p')
                            {
                                if (i + 1 == lastSelection[0] && j == lastSelection[1])
                                    board[i + 1][j] = 'A';  //  En passant rule
                            }
                        }
                    }
                    else if (ch == 'r')                 //  black rook selected by mouse
                    {
                        int k = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7)
                            {
                                if (figurines[i][k + 1] == '.')     //  empty block on the coordinate
                                    board[i][k + 1] = 'M';  //  move
                                else if (isupper(figurines[i][k + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[i][k + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                        }

                        k = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k)
                            {
                                if (figurines[i][k - 1] == '.')     //  empty block on the coordinate
                                    board[i][k - 1] = 'M';  //  move
                                else if (isupper(figurines[i][k - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[i][k - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                        }

                        k = i;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7)
                            {
                                if (figurines[k + 1][j] == '.')     //  empty block on the coordinate
                                    board[k + 1][j] = 'M';  //  move
                                else if (isupper(figurines[k + 1][j]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][j] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                        }

                        k = i;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k)
                            {
                                if (figurines[k - 1][j] == '.')     //  empty block on the coordinate
                                    board[k - 1][j] = 'M';  //  move
                                else if (isupper(figurines[k - 1][j]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][j] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                        }
                    }
                    else if (ch == 'R')                 //  white rook selected by mouse
                    {
                        int k = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7)
                            {
                                if (figurines[i][k + 1] == '.')     //  empty block on the coordinate
                                    board[i][k + 1] = 'M';  //  move
                                else if (islower(figurines[i][k + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[i][k + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                        }

                        k = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k)
                            {
                                if (figurines[i][k - 1] == '.')     //  empty block on the coordinate
                                    board[i][k - 1] = 'M';  //  move
                                else if (islower(figurines[i][k - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[i][k - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                        }

                        k = i;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7)
                            {
                                if (figurines[k + 1][j] == '.')     //  empty block on the coordinate
                                    board[k + 1][j] = 'M';  //  move
                                else if (islower(figurines[k + 1][j]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][j] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                        }

                        k = i;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k)
                            {
                                if (figurines[k - 1][j] == '.')     //  empty block on the coordinate
                                    board[k - 1][j] = 'M';  //  move
                                else if (islower(figurines[k - 1][j]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][j] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                        }
                    }
                    else if (ch == 'b')                 //  black bishop selected by mouse
                    {
                        int k = i;
                        int l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7 && l < 7)
                            {
                                if (figurines[k + 1][l + 1] == '.')     //  empty block on the coordinate
                                    board[k + 1][l + 1] = 'M';  //  move
                                else if (isupper(figurines[k + 1][l + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][l + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                            l++;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7 && l)
                            {
                                if (figurines[k + 1][l - 1] == '.')     //  empty block on the coordinate
                                    board[k + 1][l - 1] = 'M';  //  move
                                else if (isupper(figurines[k + 1][l - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][l - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                            l--;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k && l < 7)
                            {
                                if (figurines[k - 1][l + 1] == '.')     //  empty block on the coordinate
                                    board[k - 1][l + 1] = 'M';  //  move
                                else if (isupper(figurines[k - 1][l + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][l + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                            l++;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k && l)
                            {
                                if (figurines[k - 1][l - 1] == '.')     //  empty block on the coordinate
                                    board[k - 1][l - 1] = 'M';  //  move
                                else if (isupper(figurines[k - 1][l - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][l - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                            l--;
                        }
                    }
                    else if (ch == 'B')                 //  white bishop selected by mouse
                    {
                        int k = i;
                        int l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7 && l < 7)
                            {
                                if (figurines[k + 1][l + 1] == '.')     //  empty block on the coordinate
                                    board[k + 1][l + 1] = 'M';  //  move
                                else if (islower(figurines[k + 1][l + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][l + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                            l++;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7 && l)
                            {
                                if (figurines[k + 1][l - 1] == '.')     //  empty block on the coordinate
                                    board[k + 1][l - 1] = 'M';  //  move
                                else if (islower(figurines[k + 1][l - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][l - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                            l--;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k && l < 7)
                            {
                                if (figurines[k - 1][l + 1] == '.')     //  empty block on the coordinate
                                    board[k - 1][l + 1] = 'M';  //  move
                                else if (islower(figurines[k - 1][l + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][l + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                            l++;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k && l)
                            {
                                if (figurines[k - 1][l - 1] == '.')     //  empty block on the coordinate
                                    board[k - 1][l - 1] = 'M';  //  move
                                else if (islower(figurines[k - 1][l - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][l - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                            l--;
                        }
                    }
                    else if (ch == 'q')                 //  black queen selected by mouse
                    {
                        int k = j;
                        int l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7)
                            {
                                if (figurines[i][k + 1] == '.')     //  empty block on the coordinate
                                    board[i][k + 1] = 'M';  //  move
                                else if (isupper(figurines[i][k + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[i][k + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                        }

                        k = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k)
                            {
                                if (figurines[i][k - 1] == '.')     //  empty block on the coordinate
                                    board[i][k - 1] = 'M';  //  move
                                else if (isupper(figurines[i][k - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[i][k - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                        }

                        k = i;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7)
                            {
                                if (figurines[k + 1][j] == '.')     //  empty block on the coordinate
                                    board[k + 1][j] = 'M';  //  move
                                else if (isupper(figurines[k + 1][j]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][j] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                        }

                        k = i;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k)
                            {
                                if (figurines[k - 1][j] == '.')     //  empty block on the coordinate
                                    board[k - 1][j] = 'M';  //  move
                                else if (isupper(figurines[k - 1][j]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][j] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7 && l < 7)
                            {
                                if (figurines[k + 1][l + 1] == '.')     //  empty block on the coordinate
                                    board[k + 1][l + 1] = 'M';  //  move
                                else if (isupper(figurines[k + 1][l + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][l + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                            l++;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7 && l)
                            {
                                if (figurines[k + 1][l - 1] == '.')     //  empty block on the coordinate
                                    board[k + 1][l - 1] = 'M';  //  move
                                else if (isupper(figurines[k + 1][l - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][l - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                            l--;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k && l < 7)
                            {
                                if (figurines[k - 1][l + 1] == '.')     //  empty block on the coordinate
                                    board[k - 1][l + 1] = 'M';  //  move
                                else if (isupper(figurines[k - 1][l + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][l + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                            l++;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k && l)
                            {
                                if (figurines[k - 1][l - 1] == '.')     //  empty block on the coordinate
                                    board[k - 1][l - 1] = 'M';  //  move
                                else if (isupper(figurines[k - 1][l - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][l - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                            l--;
                        }
                    }
                    else if (ch == 'Q')                 //  white queen selected by mouse
                    {
                        int k = j;
                        int l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7)
                            {
                                if (figurines[i][k + 1] == '.')     //  empty block on the coordinate
                                    board[i][k + 1] = 'M';  //  move
                                else if (islower(figurines[i][k + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[i][k + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                        }

                        k = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k)
                            {
                                if (figurines[i][k - 1] == '.')     //  empty block on the coordinate
                                    board[i][k - 1] = 'M';  //  move
                                else if (islower(figurines[i][k - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[i][k - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                        }

                        k = i;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7)
                            {
                                if (figurines[k + 1][j] == '.')     //  empty block on the coordinate
                                    board[k + 1][j] = 'M';  //  move
                                else if (islower(figurines[k + 1][j]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][j] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                        }

                        k = i;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k)
                            {
                                if (figurines[k - 1][j] == '.')     //  empty block on the coordinate
                                    board[k - 1][j] = 'M';  //  move
                                else if (islower(figurines[k - 1][j]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][j] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7 && l < 7)
                            {
                                if (figurines[k + 1][l + 1] == '.')     //  empty block on the coordinate
                                    board[k + 1][l + 1] = 'M';  //  move
                                else if (islower(figurines[k + 1][l + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][l + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                            l++;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k < 7 && l)
                            {
                                if (figurines[k + 1][l - 1] == '.')     //  empty block on the coordinate
                                    board[k + 1][l - 1] = 'M';  //  move
                                else if (islower(figurines[k + 1][l - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k + 1][l - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k++;
                            l--;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k && l < 7)
                            {
                                if (figurines[k - 1][l + 1] == '.')     //  empty block on the coordinate
                                    board[k - 1][l + 1] = 'M';  //  move
                                else if (islower(figurines[k - 1][l + 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][l + 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                            l++;
                        }

                        k = i;
                        l = j;

                        while (true)                    //  try to find all possible movement coordinates for figurine
                        {
                            if (k && l)
                            {
                                if (figurines[k - 1][l - 1] == '.')     //  empty block on the coordinate
                                    board[k - 1][l - 1] = 'M';  //  move
                                else if (islower(figurines[k - 1][l - 1]))  //  opponent's figurine on the coordinate
                                {
                                    board[k - 1][l - 1] = 'A';  //  attack
                                    break;
                                }
                                else
                                    break;
                            }
                            else
                                break;

                            k--;
                            l--;
                        }
                    }
                    else if (ch == 'k')                 //  black king selected by mouse
                    {
                        if (i == 4 && j == 0)
                        {
                            if (blackCastlingKingside)
                            {
                                if (toLinear(figurines[7][0]) == 'r' && figurines[6][0] == '.' && figurines[5][0] == '.')   //  check if kingside castling is possible
                                    board[7][0] = 'C';      //  mark castling on the board
                            }

                            if (blackCastlingQueenside)
                            {
                                if (toLinear(figurines[0][0]) == 'r' && figurines[1][0] == '.' && figurines[2][0] == '.' && figurines[3][0] == '.')     //  check if queenside castling is possible
                                    board[0][0] = 'C';      //  mark castling on the board
                            }
                        }

                        if (i && figurines[i - 1][j] == '.')    //  empty block on the coordinate
                        {
                            board[i - 1][j] = 'M';      //  move
                        }
                        else if (i && isupper(figurines[i - 1][j]))     //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j] = 'A';      //  attack
                        }

                        if (i < 7 && figurines[i + 1][j] == '.')    //  empty block on the coordinate
                        {
                            board[i + 1][j] = 'M';      //  move
                        }
                        else if (i < 7 && isupper(figurines[i + 1][j]))     //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j] = 'A';      //  attack
                        }

                        if (j && figurines[i][j - 1] == '.')    //  empty block on the coordinate
                        {
                            board[i][j - 1] = 'M';      //  move
                        }
                        else if (j && isupper(figurines[i][j - 1]))     //  opponent's figurine on the coordinate
                        {
                            board[i][j - 1] = 'A';      //  attack
                        }

                        if (j < 7 && figurines[i][j + 1] == '.')    //  empty block on the coordinate
                        {
                            board[i][j + 1] = 'M';      //  move
                        }
                        else if (j < 7 && isupper(figurines[i][j + 1]))     //  opponent's figurine on the coordinate
                        {
                            board[i][j + 1] = 'A';      //  attack
                        }

                        if (i && j && figurines[i - 1][j - 1] == '.')   //  empty block on the coordinate
                        {
                            board[i - 1][j - 1] = 'M';  //  move
                        }
                        else if (i && j && isupper(figurines[i - 1][j - 1]))    //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j - 1] = 'A';  //  attack
                        }

                        if (i && j < 7 && figurines[i - 1][j + 1] == '.')   //  empty block on the coordinate
                        {
                            board[i - 1][j + 1] = 'M';  //  move
                        }
                        else if (i && j < 7 && isupper(figurines[i - 1][j + 1]))    //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j + 1] = 'A';  //  attack
                        }

                        if (i < 7 && j && figurines[i + 1][j - 1] == '.')   //  empty block on the coordinate
                        {
                            board[i + 1][j - 1] = 'M';  //  move
                        }
                        else if (i < 7 && j && isupper(figurines[i + 1][j - 1]))    //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j - 1] = 'A';  //  attack
                        }

                        if (i < 7 && j < 7 && figurines[i + 1][j + 1] == '.')   //  empty block on the coordinate
                        {
                            board[i + 1][j + 1] = 'M';  //  move
                        }
                        else if (i < 7 && j < 7 && isupper(figurines[i + 1][j + 1]))    //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j + 1] = 'A';  //  attack
                        }
                    }
                    else if (ch == 'K')                 //  white king selected by mouse
                    {
                        if (i == 4 && j == 7)
                        {
                            if (whiteCastlingKingside)
                            {
                                if (toLinear(figurines[7][7]) == 'R' && figurines[6][7] == '.' && figurines[5][7] == '.')   //  check if kingside castling is possible
                                    board[7][7] = 'C';      //  mark castling on the board
                            }

                            if (whiteCastlingQueenside)
                            {
                                if (toLinear(figurines[0][7]) == 'R' && figurines[1][7] == '.' && figurines[2][7] == '.' && figurines[3][7] == '.')     //  check if queenside castling is possible
                                    board[0][7] = 'C';      //  mark castling on the board
                            }
                        }

                        if (i && figurines[i - 1][j] == '.')    //  empty block on the coordinate
                        {
                            board[i - 1][j] = 'M';      //  move
                        }
                        else if (i && islower(figurines[i - 1][j]))         //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j] = 'A';      //  attack
                        }

                        if (i < 7 && figurines[i + 1][j] == '.')    //  empty block on the coordinate
                        {
                            board[i + 1][j] = 'M';      //  move
                        }
                        else if (i < 7 && islower(figurines[i + 1][j]))     //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j] = 'A';      //  attack
                        }

                        if (j && figurines[i][j - 1] == '.')    //  empty block on the coordinate
                        {
                            board[i][j - 1] = 'M';      //  move
                        }
                        else if (j && islower(figurines[i][j - 1]))         //  opponent's figurine on the coordinate
                        {
                            board[i][j - 1] = 'A';      //  attack
                        }

                        if (j < 7 && figurines[i][j + 1] == '.')    //  empty block on the coordinate
                        {
                            board[i][j + 1] = 'M';      //  move
                        }
                        else if (j < 7 && islower(figurines[i][j + 1]))     //  opponent's figurine on the coordinate
                        {
                            board[i][j + 1] = 'A';      //  attack
                        }

                        if (i && j && figurines[i - 1][j - 1] == '.')   //  empty block on the coordinate
                        {
                            board[i - 1][j - 1] = 'M';  //  move
                        }
                        else if (i && j && islower(figurines[i - 1][j - 1]))    //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j - 1] = 'A';  //  attack
                        }

                        if (i && j < 7 && figurines[i - 1][j + 1] == '.')   //  empty block on the coordinate
                        {
                            board[i - 1][j + 1] = 'M';  //  move
                        }
                        else if (i && j < 7 && islower(figurines[i - 1][j + 1]))    //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j + 1] = 'A';  //  attack
                        }

                        if (i < 7 && j && figurines[i + 1][j - 1] == '.')   //  empty block on the coordinate
                        {
                            board[i + 1][j - 1] = 'M';  //  move
                        }
                        else if (i < 7 && j && islower(figurines[i + 1][j - 1]))    //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j - 1] = 'A';  //  attack
                        }

                        if (i < 7 && j < 7 && figurines[i + 1][j + 1] == '.')   //  empty block on the coordinate
                        {
                            board[i + 1][j + 1] = 'M';  //  move
                        }
                        else if (i < 7 && j < 7 && islower(figurines[i + 1][j + 1]))    //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j + 1] = 'A';  //  attack
                        }
                    }
                    else if (ch == 'n')                 //  black knight selected by mouse
                    {
                        if (i && (j + 1) < 7 && figurines[i - 1][j + 2] == '.')     //  empty block on the coordinate
                        {
                            board[i - 1][j + 2] = 'M';  //  move
                        }
                        else if (i && (j + 1) < 7 && isupper(figurines[i - 1][j + 2]))  //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j + 2] = 'A';  //  attack
                        }

                        if (i < 7 && (j + 1) < 7 && figurines[i + 1][j + 2] == '.') //  empty block on the coordinate
                        {
                            board[i + 1][j + 2] = 'M';  //  move
                        }
                        else if (i < 7 && (j + 1) < 7 && isupper(figurines[i + 1][j + 2]))  //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j + 2] = 'A';  //  attack
                        }

                        if (i && (j - 1) > 0 && figurines[i - 1][j - 2] == '.')     //  empty block on the coordinate
                        {
                            board[i - 1][j - 2] = 'M';  //  move
                        }
                        else if (i && (j - 1) > 0 && isupper(figurines[i - 1][j - 2]))  //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j - 2] = 'A';  //  attack
                        }

                        if (i < 7 && (j - 1) > 0 && figurines[i + 1][j - 2] == '.') //  empty block on the coordinate
                        {
                            board[i + 1][j - 2] = 'M';  //  move
                        }
                        else if (i < 7 && (j - 1) > 0 && isupper(figurines[i + 1][j - 2]))  //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j - 2] = 'A';  //  attack
                        }

                        if ((i + 1) < 7 && j && figurines[i + 2][j - 1] == '.')     //  empty block on the coordinate
                        {
                            board[i + 2][j - 1] = 'M';  //  move
                        }
                        else if ((i + 1) < 7 && j && isupper(figurines[i + 2][j - 1]))  //  opponent's figurine on the coordinate
                        {
                            board[i + 2][j - 1] = 'A';  //  attack
                        }

                        if ((i + 1) < 7 && j < 7 && figurines[i + 2][j + 1] == '.') //  empty block on the coordinate
                        {
                            board[i + 2][j + 1] = 'M';  //  move
                        }
                        else if ((i + 1) < 7 && j < 7 && isupper(figurines[i + 2][j + 1]))  //  opponent's figurine on the coordinate
                        {
                            board[i + 2][j + 1] = 'A';  //  attack
                        }

                        if ((i - 1) > 0 && j && figurines[i - 2][j - 1] == '.')     //  empty block on the coordinate
                        {
                            board[i - 2][j - 1] = 'M';  //  move
                        }
                        else if ((i - 1) > 0 && j && isupper(figurines[i - 2][j - 1]))  //  opponent's figurine on the coordinate
                        {
                            board[i - 2][j - 1] = 'A';  //  attack
                        }

                        if ((i - 1) > 0 && j < 7 && figurines[i - 2][j + 1] == '.') //  empty block on the coordinate
                        {
                            board[i - 2][j + 1] = 'M';  //  move
                        }
                        else if ((i - 1) > 0 && j < 7 && isupper(figurines[i - 2][j + 1]))  //  opponent's figurine on the coordinate
                        {
                            board[i - 2][j + 1] = 'A';  //  attack
                        }
                    }
                    else if (ch == 'N')                 //  white knight selected by mouse
                    {
                        if (i && (j + 1) < 7 && figurines[i - 1][j + 2] == '.')     //  empty block on the coordinate
                        {
                            board[i - 1][j + 2] = 'M';  //  move
                        }
                        else if (i && (j + 1) < 7 && islower(figurines[i - 1][j + 2]))  //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j + 2] = 'A';  //  attack
                        }

                        if (i < 7 && (j + 1) < 7 && figurines[i + 1][j + 2] == '.') //  empty block on the coordinate
                        {
                            board[i + 1][j + 2] = 'M';  //  move
                        }
                        else if (i < 7 && (j + 1) < 7 && islower(figurines[i + 1][j + 2]))  //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j + 2] = 'A';  //  attack
                        }

                        if (i && (j - 1) > 0 && figurines[i - 1][j - 2] == '.')     //  empty block on the coordinate
                        {
                            board[i - 1][j - 2] = 'M';  //  move
                        }
                        else if (i && (j - 1) > 0 && islower(figurines[i - 1][j - 2]))  //  opponent's figurine on the coordinate
                        {
                            board[i - 1][j - 2] = 'A';  //  attack
                        }

                        if (i < 7 && (j - 1) > 0 && figurines[i + 1][j - 2] == '.') //  empty block on the coordinate
                        {
                            board[i + 1][j - 2] = 'M';  //  move
                        }
                        else if (i < 7 && (j - 1) > 0 && islower(figurines[i + 1][j - 2]))  //  opponent's figurine on the coordinate
                        {
                            board[i + 1][j - 2] = 'A';  //  attack
                        }

                        if ((i + 1) < 7 && j && figurines[i + 2][j - 1] == '.')     //  empty block on the coordinate
                        {
                            board[i + 2][j - 1] = 'M';  //  move
                        }
                        else if ((i + 1) < 7 && j && islower(figurines[i + 2][j - 1]))  //  opponent's figurine on the coordinate
                        {
                            board[i + 2][j - 1] = 'A';  //  attack
                        }

                        if ((i + 1) < 7 && j < 7 && figurines[i + 2][j + 1] == '.') //  empty block on the coordinate
                        {
                            board[i + 2][j + 1] = 'M';  //  move
                        }
                        else if ((i + 1) < 7 && j < 7 && islower(figurines[i + 2][j + 1]))  //  opponent's figurine on the coordinate
                        {
                            board[i + 2][j + 1] = 'A';  //  attack
                        }

                        if ((i - 1) > 0 && j && figurines[i - 2][j - 1] == '.')     //  empty block on the coordinate
                        {
                            board[i - 2][j - 1] = 'M';  //  move
                        }
                        else if ((i - 1) > 0 && j && islower(figurines[i - 2][j - 1]))  //  opponent's figurine on the coordinate
                        {
                            board[i - 2][j - 1] = 'A';  //  attack
                        }

                        if ((i - 1) > 0 && j < 7 && figurines[i - 2][j + 1] == '.') //  empty block on the coordinate
                        {
                            board[i - 2][j + 1] = 'M';  //  move
                        }
                        else if ((i - 1) > 0 && j < 7 && islower(figurines[i - 2][j + 1]))  //  opponent's figurine on the coordinate
                        {
                            board[i - 2][j + 1] = 'A';  //  attack
                        }
                    }

                    int count = 0;

                    for (int j = 0; j < 8; j++)
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            if (board[i][j] == 'M')     //  move
                                highlight[count] = 1;   //  save found coordinates into highlight array
                            else if (board[i][j] == 'A')     //  attack
                                highlight[count] = 2;   //  save found coordinates into highlight array
                            else if (board[i][j] == 'C')     //  castling
                                highlight[count] = 3;   //  save found coordinates into highlight array

                            count++;
                        }
                    }

                    return;
                }
            }
        }
    }

    bool performMove(int & i, int & j)                  //  perform move with figurine to position on desired coordinates
    {
        string oldLinear = linear;
        string oldFigurines[8];

        for (int i = 0; i < 8; i++)
            oldFigurines[i] = figurines[i];             //  save actual game board state

        if (board[i][j] == 'M')                         //  move is going to be performed
        {
            figurines[i][j] = figurines[selection[0]][selection[1]];    //  move figurine to new position
            figurines[selection[0]][selection[1]] = '.';    //  previous position is going to be empty
        }
        else if (board[i][j] == 'C')                    //  castling is going to be performed
        {                                               //  attack check of all three king's movement positions needs to be performed
            if (calculateCheck())                       //  king is under attack on its initial position
            {
                linear = oldLinear;                     //  restore old array

                for (int i = 0; i < 8; i++)
                    figurines[i] = oldFigurines[i];    //  restore old game board state

                return false;
            }

            if (i == 0)                                 //  differ kingside and queenside castling
            {
                figurines[3][j] = figurines[4][j];
                figurines[4][j] = '.';
            }
            else if (i == 7)
            {
                figurines[5][j] = figurines[4][j];
                figurines[4][j] = '.';
            }

            if (calculateCheck())                       //  king is under attack on its movement position
            {
                linear = oldLinear;                     //  restore old array

                for (int i = 0; i < 8; i++)
                    figurines[i] = oldFigurines[i];    //  restore old game board state

                return false;
            }

            if (i == 0)                                 //  differ kingside and queenside castling
            {
                figurines[2][j] = figurines[3][j];

                figurines[3][j] = figurines[0][j];
                figurines[0][j] = '.';
            }
            else if (i == 7)
            {
                figurines[6][j] = figurines[5][j];

                figurines[5][j] = figurines[7][j];
                figurines[7][j] = '.';
            }
        }
        else if (board[i][j] == 'A')                    //  attack is going to be performed
        {
            char ch = figurines[i][j];

            for (int k = 0; k < 32; k++)
            {
                if (real[k] == ch)
                {
                    linear[k] = '.';                    //  remove opponent's figurine from the game board
                    break;
                }
            }

            figurines[i][j] = figurines[selection[0]][selection[1]];    //  move figurine to new position
            figurines[selection[0]][selection[1]] = '.';    //  previous position is going to be empty
        }

        if (calculateCheck())                           //  king is under attack on its final position
        {
            linear = oldLinear;                         //  restore old array

            for (int i = 0; i < 8; i++)
                figurines[i] = oldFigurines[i];         //  restore old game board state

            return false;
        }

        if (board[i][j] == 'C')                         //  castling has just been performed
        {
            if (blackTurn)
            {
                blackCastlingKingside = false;          //  castling is not allowed anymore for black player
                blackCastlingQueenside = false;
            }
            else
            {
                whiteCastlingKingside = false;          //  castling is not allowed anymore for white player
                whiteCastlingQueenside = false;
            }
        }
        else if (toLinear(figurines[i][j]) == 'k')
        {
            blackCastlingKingside = false;              //  castling is not allowed anymore for black player
            blackCastlingQueenside = false;
        }
        else if (toLinear(figurines[i][j]) == 'K')
        {
            whiteCastlingKingside = false;              //  castling is not allowed anymore for white player
            whiteCastlingQueenside = false;
        }
        else if (toLinear(figurines[i][j]) == 'r')
        {
            if (figurines[i][j] == 'i')
                blackCastlingQueenside = false;
            else if (figurines[i][j] == 'p')
                blackCastlingKingside = false;          //  castling is not allowed anymore for black player
        }
        else if (toLinear(figurines[i][j]) == 'R')
        {
            if (figurines[i][j] == 'I')
                whiteCastlingQueenside = false;
            else if (figurines[i][j] == 'P')
                whiteCastlingKingside = false;          //  castling is not allowed anymore for white player
        }

        lastSelection[0] = i;
        lastSelection[1] = j;

        promotion = -1;                                 //  reset pawn promotion flag

        if (j == 0 && toLinear(figurines[i][j]) == 'P')     //  handle pawn promotion
        {
            char ch = figurines[i][j];

            for (int k = 0; k < 32; k++)
            {
                if (real[k] == ch)
                {
                    linear[k] = 'Q';                    //  replace pawn figurine by queen figurine
                    promotion = i;                      //  set promotion flag to appropriate index value
                    break;
                }
            }
        }
        else if (j == 7 && toLinear(figurines[i][j]) == 'p')    //  handle pawn promotion
        {
            char ch = figurines[i][j];

            for (int k = 0; k < 32; k++)
            {
                if (real[k] == ch)
                {
                    linear[k] = 'q';                    //  replace pawn figurine by queen figurine
                    promotion = i;                      //  set promotion flag to appropriate index value
                    break;
                }
            }
        }

        return true;
    }

    bool refreshFigurines(int & i, int & j)             //  rehresh game board after clicking onto highlighted game board board
    {
        if (selection[0] == -1 || selection[1] == -1)   //  do nothing when nothing is selected
            return false;

        if (i < 0 || j < 0 || i >= 8 || j >= 8)         //  handle index overflow
            return false;

        return performMove(i, j);                       //  perform move with figurine
    }

    bool refreshFigurines(int index)                    //  rehresh game board after clicking onto highlighted game board board
    {
        if (selection[0] == -1 || selection[1] == -1)   //  do nothing when nothing is selected
            return false;

        int count = 0;

        for (int j = 0; j < 8; j++)                     //  iterate through game board
        {
            for (int i = 0; i < 8; i++)
            {
                if (count == index)                     //  correct coordinates have been found
                    return performMove(i, j);           //  perform move with figurine

                count++;
            }
        }

        return false;
    }

    void refreshPositions()                             //  refresh real coordinates of all figurines after performed turn
    {
        for (int i = 0; i < 8; i++)                     //  iterate through all figurines
        {
            for (int j = 0; j < 8; j++)
            {
                char ch = figurines[i][j];

                if (ch >= 'a' && ch <= 'h')             //  black pawn
                {
                    positions[64 + int(ch - 'a')].x = float(i) * 0.4f;  //  multiply figurine offset coordinate by difference value
                    positions[64 + int(ch - 'a')].z = float(j) * 0.4f;
                }
                else if (ch >= 'A' && ch <= 'H')        //  white pawn
                {
                    positions[72 + int(ch - 'A')].x = float(i) * 0.4f;  //  multiply figurine offset coordinate by difference value
                    positions[72 + int(ch - 'A')].z = float(j) * 0.4f;
                }
                else if (ch >= 'i' && ch <= 'p')        //  black non-pawn
                {
                    positions[80 + int(ch - 'i')].x = float(i) * 0.4f;  //  multiply figurine offset coordinate by difference value
                    positions[80 + int(ch - 'i')].z = float(j) * 0.4f;
                }
                else if (ch >= 'I' && ch <= 'P')        //  white non-pawn
                {
                    positions[88 + int(ch - 'I')].x = float(i) * 0.4f;  //  multiply figurine offset coordinate by difference value
                    positions[88 + int(ch - 'I')].z = float(j) * 0.4f;
                }
            }
        }

        for (int i = 64; i < 96; i++)                   //  move game board to the center of the coordinate system
        {
            positions[i].x -= 1.4f;
            positions[i].z -= 1.4f;
        }
    }

    void handle()                                       //  handles click on any game object and executes appropriate reaction
    {
        if (animationActive)
            return;

        if (clicked >= 0)                               //  check if something was clicked
        {
            if (clicked < 64)                           //  game board clicked
            {
                if (highlight[clicked])                 //  check if selected position is highlighted
                {
                    if (refreshFigurines(clicked) == false)     //  calculate new turn
                    {
                        clicked = -1;
                        return;
                    }

                    selection[0] = selection[1] = -1;   //  remove actual selection
                    cameraPtr->toggle();                    //  toggle camera to trace second player
                    animationActive = true;             //  activate animation flag
                    blackTurn = !blackTurn;             //  opponent is now on the turn

                    for (int i = 0; i <= 96; i++)       //  remove all highlights from game board
                    {
                        if (highlight[i])
                            highlight[i] = 0;
                    }
                }
            }
            else                                        //  figurine clicked
            {
                if (highlight[clicked])                 //  game board block was clicked
                {
                    for (int i = 0; i <= 96; i++)       //  mark clicked object as highlighted
                    {
                        if (highlight[i])
                            highlight[i] = 0;
                    }
                }
                else                                    //  figurine was clicked
                {
                    if (blackTurn)                      //  black player is on turn
                    {
                        if ((clicked >= 72 && clicked < 80) || (clicked >= 88 && clicked < 96))     //  handle clicking on the opponent's pawn
                        {
                            int i, j;                   //  hold appropriate coordinates

                            if (isUnderAttack(clicked, i, j))   //  check if figurine is under attack
                            {
                                if (refreshFigurines(i, j) == false)     //  calculate new turn
                                {
                                    clicked = -1;
                                    return;
                                }

                                selection[0] = selection[1] = -1;   //  remove actual selection
                                cameraPtr->toggle();        //  toggle camera to trace second player
                                animationActive = true;     //  activate animation flag
                                blackTurn = !blackTurn;     //  opponent is now on the turn

                                for (i = 0; i <= 96; i++)   //  remove all highlights from game board
                                {
                                    if (highlight[i])
                                        highlight[i] = 0;
                                }
                            }
                            else if (promotion >= 0 && clicked >= 72 && clicked < 80)    //  handle promotion of opponent's pawn
                            {
                                int index = clicked;
                                index -= 64;            //  ignore game board blocks
                                char ch = real[index];  //  use mapping of index to real type of figurine

                                if (ch == figurines[promotion][0])  //  concrete figurine found inside array
                                {
                                    ch = linear[index];     //  use mapping to linear type of figurine

                                    if (ch == 'Q')      //  handle toggling between allowed promotion figurines
                                        linear[index] = 'N';
                                    else if (ch == 'N')
                                        linear[index] = 'R';
                                    else if (ch == 'R')
                                        linear[index] = 'B';
                                    else if (ch == 'B')
                                        linear[index] = 'Q';
                                }

                                clicked = -1;
                                return;                 //  just toggle figurine without affecting opponent's turn
                            }
                        }
                        else if ((clicked >= 64 && clicked < 72) || (clicked >= 80 && clicked < 88))    //  handle clicking on the black figurine
                        {
                            for (int i = 0; i <= 96; i++)   //  remove all highlights from game board
                            {
                                if (highlight[i])
                                    highlight[i] = 0;
                            }

                            refreshBoard(clicked);

                            highlight[clicked] = 1;     //  mark clicked object as highlighted
                        }
                    }
                    else                                //  white player is on turn
                    {
                        if ((clicked >= 64 && clicked < 72) || (clicked >= 80 && clicked < 88))     //  handle clicking on the opponent's pawn
                        {
                            int i, j;                   //  hold appropriate coordinates

                            if (isUnderAttack(clicked, i, j))   //  check if figurine is under attack
                            {
                                if (refreshFigurines(i, j) == false)     //  calculate new turn
                                {
                                    clicked = -1;
                                    return;
                                }

                                selection[0] = selection[1] = -1;   //  remove actual selection
                                cameraPtr->toggle();        //  toggle camera to trace second player
                                animationActive = true;     //  activate animation flag
                                blackTurn = !blackTurn;     //  opponent is now on the turn

                                for (i = 0; i <= 96; i++)   //  remove all highlights from game board
                                {
                                    if (highlight[i])
                                        highlight[i] = 0;
                                }
                            }
                            else if (promotion >= 0 && clicked >= 64 && clicked < 72)    //  handle promotion of opponent's pawn
                            {
                                int index = clicked;
                                index -= 64;            //  ignore game board blocks
                                char ch = real[index];      //  use mapping of index to real type of figurine

                                if (ch == figurines[promotion][7])  //  concrete figurine found inside array
                                {
                                    ch = linear[index];     //  use mapping to linear type of figurine

                                    if (ch == 'q')      //  handle toggling between allowed promotion figurines
                                        linear[index] = 'n';
                                    else if (ch == 'n')
                                        linear[index] = 'r';
                                    else if (ch == 'r')
                                        linear[index] = 'b';
                                    else if (ch == 'b')
                                        linear[index] = 'q';
                                }

                                clicked = -1;
                                return;                 //  just toggle figurine without affecting opponent's turn
                            }
                        }
                        else if ((clicked >= 72 && clicked < 80) || (clicked >= 88 && clicked < 96))    //  handle clicking on the white figurine
                        {
                            for (int i = 0; i <= 96; i++)   //  remove all highlights from game board
                            {
                                if (highlight[i])
                                    highlight[i] = 0;
                            }

                            refreshBoard(clicked);

                            highlight[clicked] = 1;     //  mark clicked object as highlighted
                        }
                    }
                }
            }

            refreshPositions();

            clicked = -1;
        }
    }
};

int main(int argc, char ** argv)                        //  required main method implementation
{
    ios_base::sync_with_stdio(false);

    bool fullscreen = false;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v"))
        {
            cout << "richess\n";
            cout << "version 1.2\n";
            return 0;
        }
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
        {
            cout << "DESCRIPTION\n"
                 << "    richess - 3D Chess game written in C++ and OpenGL\n\n"
                 << "USAGE\n"
                 << "    richess --help        ->  print usage information\n"
                 << "    richess --version     ->  print version information\n"
                 << "    richess --fullscreen  ->  run application in fullscreen mode\n\n"
                 << "RETURN VALUES\n"
                 << "    0  ->  success\n"
                 << "    1  ->  program arguments error\n\n"
                 << "KEYBOARD SHORTCUTS\n"
                 << "    w  ->  rotate camera up\n"
                 << "    s  ->  rotate camera down\n"
                 << "    a  ->  rotate camera left\n"
                 << "    d  ->  rotate camera right\n"
                 << "    q  ->  zoom camera in\n"
                 << "    e  ->  zoom camera out\n"
                 << "    i  ->  move active light source forward\n"
                 << "    k  ->  move active light source backward\n"
                 << "    j  ->  move active light source left\n"
                 << "    l  ->  move active light source right\n"
                 << "    u  ->  move active light source up\n"
                 << "    h  ->  move active light source down\n"
                 << "    m  ->  switch active light source\n"
                 << "    p  ->  light sources visibility toggle\n"
                 << "    o  ->  activate light source orbit mode\n";

            return 0;
        }
        else if (!strcmp(argv[i], "--fullscreen") || !strcmp(argv[i], "-f"))
            fullscreen = true;
    }

    glfwInit();                                         //  initialize GLFW window and set GLFW version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    if (fullscreen)
    {
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());   //  get actual resolution of the screen

        if (mode)
        {
            window_width = mode->width;
            window_height = mode->height;
        }
    }

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);    // this line allows successfull compilation on Apple operating systems
#endif
    
    GLFWwindow * window;

    if (fullscreen)                                     //  toggle fullscreen mode by the appropriate flag value
        window = glfwCreateWindow(window_width, window_height, "Richess", glfwGetPrimaryMonitor(), nullptr);
    else
        window = glfwCreateWindow(window_width, window_height, "Richess", nullptr, nullptr);

    if (window == nullptr)                              //  window has not been created successfully
    {
        cerr << "GLFW window has not been created successfully\n";
        glfwTerminate();

        return -1;
    }

    glfwMakeContextCurrent(window);                     //  set GLFW context
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  //  connect all GLFW callbacks with defined callback methods
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Failed to initialize GLAD\n";          //  initialize glad
        return -1;
    }

    glEnable(GL_DEPTH_TEST);                            //  enable depth and stencil test
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_MULTISAMPLE);                           //  multisampling is going to be used

    unique_ptr<Chess> chessPtr(new Chess);              //  create chess game logic instance

    unique_ptr<Model> cubePtr(new Model(ROOT_DIR "res/models/block/positions.txt", ROOT_DIR "res/models/block/normals.txt", ROOT_DIR "res/models/block/indices.txt", ROOT_DIR "res/models/block/uv.txt"));
    unique_ptr<Model> rookPtr(new Model(ROOT_DIR "res/models/rook/positions.txt", ROOT_DIR "res/models/rook/normals.txt", ROOT_DIR "res/models/rook/indices.txt", ROOT_DIR "res/models/rook/uv.txt"));
    unique_ptr<Model> knightPtr(new Model(ROOT_DIR "res/models/knight/positions.txt", ROOT_DIR "res/models/knight/normals.txt", ROOT_DIR "res/models/knight/indices.txt", ROOT_DIR "res/models/knight/uv.txt"));
    unique_ptr<Model> bishopPtr(new Model(ROOT_DIR "res/models/bishop/positions.txt", ROOT_DIR "res/models/bishop/normals.txt", ROOT_DIR "res/models/bishop/indices.txt", ROOT_DIR "res/models/bishop/uv.txt"));
    unique_ptr<Model> kingPtr(new Model(ROOT_DIR "res/models/king/positions.txt", ROOT_DIR "res/models/king/normals.txt", ROOT_DIR "res/models/king/indices.txt", ROOT_DIR "res/models/king/uv.txt"));
    unique_ptr<Model> queenPtr(new Model(ROOT_DIR "res/models/queen/positions.txt", ROOT_DIR "res/models/queen/normals.txt", ROOT_DIR "res/models/queen/indices.txt", ROOT_DIR "res/models/queen/uv.txt"));
    unique_ptr<Model> pawnPtr(new Model(ROOT_DIR "res/models/pawn/positions.txt", ROOT_DIR "res/models/pawn/normals.txt", ROOT_DIR "res/models/pawn/indices.txt", ROOT_DIR "res/models/pawn/uv.txt"));

    unique_ptr<Shader> shaderPBR(new Shader(ROOT_DIR "res/shaders/pbr.vs", ROOT_DIR "res/shaders/pbr.fs"));

    shaderPBR->activate();
    shaderPBR->passInteger("albedoMap", 0);
    shaderPBR->passInteger("normalMap", 1);
    shaderPBR->passInteger("metallicMap", 2);
    shaderPBR->passInteger("roughnessMap", 3);
    shaderPBR->passInteger("aoMap", 4);

    unsigned int albedoRed = loadTexture(ROOT_DIR "res/models/textures/highlight/albedoRed.png");
    unsigned int albedoGreen = loadTexture(ROOT_DIR "res/models/textures/highlight/albedoGreen.png");
    unsigned int albedoBlue = loadTexture(ROOT_DIR "res/models/textures/highlight/albedoBlue.png");
    unsigned int albedoYellow = loadTexture(ROOT_DIR "res/models/textures/highlight/albedoYellow.png");
    unsigned int albedoWhite = loadTexture(ROOT_DIR "res/models/textures/highlight/albedoWhite.png");
    unsigned int albedoBlack = loadTexture(ROOT_DIR "res/models/textures/highlight/albedoBlack.png");

    unsigned int normalHighlight = loadTexture(ROOT_DIR "res/models/textures/highlight/normal.png");
    unsigned int metallicHighlight = loadTexture(ROOT_DIR "res/models/textures/highlight/metallic.png");
    unsigned int roughnessHighlight = loadTexture(ROOT_DIR "res/models/textures/highlight/roughness.png");
    unsigned int aoHighlight = loadTexture(ROOT_DIR "res/models/textures/highlight/ao.png");

    unsigned int albedoBlackFigurine = loadTexture(ROOT_DIR "res/models/textures/blackFigurine/albedo.png");
    unsigned int normalBlackFigurine = loadTexture(ROOT_DIR "res/models/textures/blackFigurine/normal.png");
    unsigned int metallicBlackFigurine = loadTexture(ROOT_DIR "res/models/textures/blackFigurine/metallic.png");
    unsigned int roughnessBlackFigurine = loadTexture(ROOT_DIR "res/models/textures/blackFigurine/roughness.png");
    unsigned int aoBlackFigurine = loadTexture(ROOT_DIR "res/models/textures/blackFigurine/ao.png");

    unsigned int albedoWhiteBlock = loadTexture(ROOT_DIR "res/models/textures/whiteBlock/albedo.png");
    unsigned int normalWhiteBlock = loadTexture(ROOT_DIR "res/models/textures/whiteBlock/normal.png");
    unsigned int metallicWhiteBlock = loadTexture(ROOT_DIR "res/models/textures/whiteBlock/metallic.png");
    unsigned int roughnessWhiteBlock = loadTexture(ROOT_DIR "res/models/textures/whiteBlock/roughness.png");
    unsigned int aoWhiteBlock = loadTexture(ROOT_DIR "res/models/textures/whiteBlock/ao.png");

    unsigned int albedoBlackBlock = loadTexture(ROOT_DIR "res/models/textures/blackBlock/albedo.png");
    unsigned int normalBlackBlock = loadTexture(ROOT_DIR "res/models/textures/blackBlock/normal.png");
    unsigned int metallicBlackBlock = loadTexture(ROOT_DIR "res/models/textures/blackBlock/metallic.png");
    unsigned int roughnessBlackBlock = loadTexture(ROOT_DIR "res/models/textures/blackBlock/roughness.png");
    unsigned int aoBlackBlock = loadTexture(ROOT_DIR "res/models/textures/blackBlock/ao.png");

    unsigned int albedoWhiteFigurine = loadTexture(ROOT_DIR "res/models/textures/whiteFigurine/albedo.png");
    unsigned int normalWhiteFigurine = loadTexture(ROOT_DIR "res/models/textures/whiteFigurine/normal.png");
    unsigned int metallicWhiteFigurine = loadTexture(ROOT_DIR "res/models/textures/whiteFigurine/metallic.png");
    unsigned int roughnessWhiteFigurine = loadTexture(ROOT_DIR "res/models/textures/whiteFigurine/roughness.png");
    unsigned int aoWhiteFigurine = loadTexture(ROOT_DIR "res/models/textures/whiteFigurine/ao.png");

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
    shaderPBR->activate();
    shaderPBR->passMatrix("projection", projection);

    for (int i = 0; i < 8; i++)                         //  initialize first row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[i] = glm::vec3(offset, 0.0f, 0.0f);
        highlight[i] = 0;                               //  nothing is highlighted by default

        if (i & 1)
            isWhite[i] = false;
        else
            isWhite[i] = true;
    }

    for (int i = 0; i < 8; i++)                         //  initialize second row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[8 + i] = glm::vec3(offset, 0.0f, 0.4f);
        highlight[8 + i] = 0;                           //  nothing is highlighted by default

        if (i & 1)
            isWhite[8 + i] = true;
        else
            isWhite[8 + i] = false;
    }

    for (int i = 0; i < 8; i++)                         //  initialize third row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[16 + i] = glm::vec3(offset, 0.0f, 0.8f);
        highlight[16 + i] = 0;                          //  nothing is highlighted by default

        if (i & 1)
            isWhite[16 + i] = false;
        else
            isWhite[16 + i] = true;
    }

    for (int i = 0; i < 8; i++)                         //  initialize fourth row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[24 + i] = glm::vec3(offset, 0.0f, 1.2f);
        highlight[24 + i] = 0;                          //  nothing is highlighted by default

        if (i & 1)
            isWhite[24 + i] = true;
        else
            isWhite[24 + i] = false;
    }

    for (int i = 0; i < 8; i++)                         //  initialize fifth row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[32 + i] = glm::vec3(offset, 0.0f, 1.6f);
        highlight[32 + i] = 0;                          //  nothing is highlighted by default

        if (i & 1)
            isWhite[32 + i] = false;
        else
            isWhite[32 + i] = true;
    }

    for (int i = 0; i < 8; i++)                         //  initialize sixth row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[40 + i] = glm::vec3(offset, 0.0f, 2.0f);
        highlight[40 + i] = 0;                          //  nothing is highlighted by default

        if (i & 1)
            isWhite[40 + i] = true;
        else
            isWhite[40 + i] = false;
    }

    for (int i = 0; i < 8; i++)                         //  initialize seventh row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[48 + i] = glm::vec3(offset, 0.0f, 2.4f);
        highlight[48 + i] = 0;                          //  nothing is highlighted by default

        if (i & 1)
            isWhite[48 + i] = false;
        else
            isWhite[48 + i] = true;
    }

    for (int i = 0; i < 8; i++)                         //  initialize eighth row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[56 + i] = glm::vec3(offset, 0.0f, 2.8f);
        highlight[56 + i] = 0;                          //  nothing is highlighted by default

        if (i & 1)
            isWhite[56 + i] = true;
        else
            isWhite[56 + i] = false;
    }

    for (int i = 0; i < 8; i++)                         //  initialize black pawns
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[64 + i] = glm::vec3(offset, 0.23f, 0.4f);
        highlight[64 + i] = 0;                          //  nothing is highlighted by default
        isWhite[64 + i] = false;
    }

    for (int i = 0; i < 8; i++)                         //  initialize white pawns
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[72 + i] = glm::vec3(offset, 0.23f, 2.4f);
        highlight[72 + i] = 0;                          //  nothing is highlighted by default
        isWhite[72 + i] = true;
    }

    for (int i = 0; i < 8; i++)                         //  initialize black non-pawns
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[80 + i] = glm::vec3(offset, 0.23f, 0.0f);
        highlight[80 + i] = 0;                          //  nothing is highlighted by default
        isWhite[80 + i] = false;
    }

    for (int i = 0; i < 8; i++)                         //  initialize white non-pawns
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[88 + i] = glm::vec3(offset, 0.23f, 2.8f);
        highlight[88 + i] = 0;                          //  nothing is highlighted by default
        isWhite[88 + i] = true;
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[96 + i] = glm::vec3(offset, -0.03f, -0.4f);
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f - 0.4f;          //  handle object offset

        positions[105 + i] = glm::vec3(offset, -0.03f, 3.2f);
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[114 + i] = glm::vec3(3.2f, -0.03f, offset);
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f - 0.4f;          //  handle object offset

        positions[123 + i] = glm::vec3(-0.4f, -0.03f, offset);
    }

    for (int i = 0; i < 132; i++)                       //  move all game objects into the center of coordinate system
    {
        positions[i].x -= 1.4f;
        positions[i].z -= 1.4f;
    }

    while (!glfwWindowShouldClose(window))              //  handle infinite render loop
    {
        processInput(window);                           //  process keyboard input

        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);    //  update background color of window
        glClearStencil(0);                              //  clear stencil buffer and set default index value
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);     //  clear all required buffers

        chessPtr->handle();                                 //  handle chess logic turn
        
        if (chessPtr->animationActive)                      //  check if camera animation is activated
        {
            chessPtr->actual = glfwGetTime();               //  get actual timestamp

            if (chessPtr->actual - chessPtr->previous > 0.01)   //  check if time after last animation step is a required chunk
            {
                if (cameraPtr->animateToggle())             //  handle camera movement
                    chessPtr->previous = chessPtr->actual;
                else
                    chessPtr->animationActive = false;      //  disable animation after successfull camera movement
            }
        }

        if (lampOrbit)
        {
            float radius;

            radius = 7.0f;
            lightPositionOne.x = sin(glfwGetTime() * 0.1f) * radius;     //  orbit light source around the game board
            lightPositionOne.z = cos(glfwGetTime() * 0.1f) * radius;

            radius = 7.0f;
            lightPositionTwo.x = sin(glfwGetTime() * 0.09f) * radius;     //  orbit light source around the game board
            lightPositionTwo.z = cos(glfwGetTime() * 0.09f) * radius;

            radius = 7.0f;
            lightPositionThree.x = sin(glfwGetTime() * 0.08f) * radius;     //  orbit light source around the game board
            lightPositionThree.z = cos(glfwGetTime() * 0.08f) * radius;

            radius = 7.0f;
            lightPositionFour.x = sin(glfwGetTime() * 0.07f) * radius;     //  orbit light source around the game board
            lightPositionFour.z = cos(glfwGetTime() * 0.07f) * radius;
        }

        // reset viewport
        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        shaderPBR->activate();
        glm::mat4 view = glm::mat4(1.0f);
        view = cameraPtr->loadViewMatrix();
        shaderPBR->passMatrix("view", view);
        shaderPBR->passVector("camPos", cameraPtr->Position);

        for (int i = 0; i < 64; i++)                    //  render all game board blocks
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, positions[i]);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

            if (highlight[i] == 1)                          //  move
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, albedoBlue);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, normalHighlight);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, metallicHighlight);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessHighlight);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoHighlight);
            }
            else if (highlight[i] == 2)                     //  attack
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, albedoRed);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, normalHighlight);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, metallicHighlight);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessHighlight);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoHighlight);
            }
            else if (highlight[i] == 3)                     //  castling
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, albedoYellow);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, normalHighlight);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, metallicHighlight);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessHighlight);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoHighlight);
            }
            else
            {
                if (isWhite[i] == true)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, albedoWhiteBlock);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, normalWhiteBlock);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, metallicWhiteBlock);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, roughnessWhiteBlock);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, aoWhiteBlock);
                }
                else
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, albedoBlackBlock);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, normalBlackBlock);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, metallicBlackBlock);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, roughnessBlackBlock);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, aoBlackBlock);
                }
            }

            shaderPBR->passMatrix("model", model);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer
            cubePtr->render();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoWhiteBlock);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalWhiteBlock);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallicWhiteBlock);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessWhiteBlock);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, aoWhiteBlock);

        for (int i = 64; i < 96; i++)                   //  render all figurines
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, positions[i]);  //  translate and scale object before rendering
            model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));

            if (highlight[i])
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, albedoGreen);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, normalHighlight);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, metallicHighlight);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessHighlight);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoHighlight);
            }
            else
            {
                if (isWhite[i] == true)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, albedoWhiteFigurine);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, normalWhiteFigurine);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, metallicWhiteFigurine);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, roughnessWhiteFigurine);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, aoWhiteFigurine);
                }
                else
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, albedoBlackFigurine);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, normalBlackFigurine);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, metallicBlackFigurine);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, roughnessBlackFigurine);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, aoBlackFigurine);
                }
            }

            shaderPBR->passMatrix("model", model);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer

            string linear = chessPtr->getLinear();
                                                        //  check type of figure using value inside linear array
            if (linear[i - 64] == 'p' || linear[i - 64] == 'P')
                pawnPtr->render();
            else if (linear[i - 64] == 'r' || linear[i - 64] == 'R')
                rookPtr->render();
            else if (linear[i - 64] == 'n')
            {
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                shaderPBR->passMatrix("model", model);
                knightPtr->render();
            }
            else if (linear[i - 64] == 'N')
                knightPtr->render();
            else if (linear[i - 64] == 'b' || linear[i - 64] == 'B')
                bishopPtr->render();
            else if (linear[i - 64] == 'q' || linear[i - 64] == 'Q')
                queenPtr->render();
            else if (linear[i - 64] == 'k' || linear[i - 64] == 'K')
                kingPtr->render();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoBlack);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalHighlight);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallicHighlight);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessHighlight);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, aoHighlight);

        for (int i = 96; i < 132; i++)                  //  render game board border cubes
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, positions[i]);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            shaderPBR->passMatrix("model", model);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer
            cubePtr->render();
        }

        shaderPBR->passVector("lightPositions[0]", lightPositionOne);
        shaderPBR->passVector("lightColors[0]", lightColorOne);
        shaderPBR->passVector("lightPositions[1]", lightPositionTwo);
        shaderPBR->passVector("lightColors[1]", lightColorTwo);
        shaderPBR->passVector("lightPositions[2]", lightPositionThree);
        shaderPBR->passVector("lightColors[2]", lightColorThree);
        shaderPBR->passVector("lightPositions[3]", lightPositionFour);
        shaderPBR->passVector("lightColors[3]", lightColorFour);

        if (lampVisible)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, albedoWhite);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalHighlight);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, metallicHighlight);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, roughnessHighlight);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, aoHighlight);

            glm::vec3 finalPosition = lightPositionOne;
            finalPosition.y += 0.2;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, finalPosition);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            shaderPBR->passMatrix("model", model);
            glStencilFunc(GL_ALWAYS, 97, -1);           //  write object into the stencil buffer
            cubePtr->render();
        }

        glfwSwapBuffers(window);                        //  swap buffers and poll IO events
        glfwPollEvents();
    }

    glfwTerminate();                                    //  terminate GLFW runtime
    return 0;
}

void key_callback(GLFWwindow * /*window*/, int key, int /*scancode*/, int action, int /*mods*/)     //  callback keyboard input
{
    if (key == GLFW_KEY_P && action == GLFW_PRESS)      //  toggle lamp object visibility
        lampVisible = !lampVisible;

    if (key == GLFW_KEY_O && action == GLFW_PRESS)      //  toggle lamp orbit
    {
        if (lampOrbit)                                  //  enable lamp orbit
        {
            lampOrbit = false;

            lightPositionOne.x = -5.0f;
            lightPositionOne.y = 5.0f;
            lightPositionOne.z = 0.0f;

            lightPositionTwo.x = -5.0f;
            lightPositionTwo.y = 5.0f;
            lightPositionTwo.z = -6.0f;

            lightPositionThree.x = -5.0f;
            lightPositionThree.y = 5.0f;
            lightPositionThree.z = 6.0f;

            lightPositionFour.x = -3.5f;
            lightPositionFour.y = 10.0f;
            lightPositionFour.z = -0.0f;

            lightColorOne.x = 150.0f;
            lightColorOne.y = 150.0f;
            lightColorOne.z = 150.0f;

            lightColorTwo.x = 150.0f;
            lightColorTwo.y = 150.0f;
            lightColorTwo.z = 150.0f;

            lightColorThree.x = 150.0f;
            lightColorThree.y = 150.0f;
            lightColorThree.z = 150.0f;

            lightColorFour.x = 150.0f;
            lightColorFour.y = 150.0f;
            lightColorFour.z = 150.0f;
        }
        else                                            //  disable lamp orbit
        {
            lampOrbit = true;

            lightPositionOne.x = 0.0f;
            lightPositionOne.y = 5.0f;
            lightPositionOne.z = 0.0f;

            lightPositionTwo.x = 0.0f;
            lightPositionTwo.y = 5.0f;
            lightPositionTwo.z = 0.0f;

            lightPositionThree.x = 0.0f;
            lightPositionThree.y = 5.0f;
            lightPositionThree.z = 0.0f;

            lightPositionFour.x = 0.0f;
            lightPositionFour.y = 5.0f;
            lightPositionFour.z = 0.0f;

            lightColorOne.x = 237.0f;
            lightColorOne.y = 21.0f;
            lightColorOne.z = 77.0f;

            lightColorTwo.x = 21.0f;
            lightColorTwo.y = 237.0f;
            lightColorTwo.z = 93.0f;

            lightColorThree.x = 29.0f;
            lightColorThree.y = 129.0f;
            lightColorThree.z = 243.0f;

            lightColorFour.x = 243.0f;
            lightColorFour.y = 200.0f;
            lightColorFour.z = 29.0f;
        }
    }
    else if (key == GLFW_KEY_M && action == GLFW_PRESS)     //  toggle active lamp
    {
        if (activeLamp == 3)
            activeLamp = 0;
        else
            activeLamp++;
    }
}

void processInput(GLFWwindow * window)                  //  non-callback keyboard input
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);         //  close window
                                                        //  move camera by pressing appropriate buttons
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPtr->tiltUp();

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPtr->tiltDown();

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraPtr->rotateLeft();

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraPtr->rotateRight();

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        cameraPtr->zoomOut();

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        cameraPtr->zoomIn();
                                                        //  move light source by pressing appropriate buttons
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        switch (activeLamp)
        {
            case 0:
                lightPositionOne.z += 0.03;
                break;
            case 1:
                lightPositionTwo.z += 0.03;
                break;
            case 2:
                lightPositionThree.z += 0.03;
                break;
            default:
                lightPositionFour.z += 0.03;
                break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        switch (activeLamp)
        {
            case 0:
                lightPositionOne.z -= 0.03;
                break;
            case 1:
                lightPositionTwo.z -= 0.03;
                break;
            case 2:
                lightPositionThree.z -= 0.03;
                break;
            default:
                lightPositionFour.z -= 0.03;
                break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        switch (activeLamp)
        {
            case 0:
                lightPositionOne.x -= 0.03;
                break;
            case 1:
                lightPositionTwo.x -= 0.03;
                break;
            case 2:
                lightPositionThree.x -= 0.03;
                break;
            default:
                lightPositionFour.x -= 0.03;
                break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        switch (activeLamp)
        {
            case 0:
                lightPositionOne.x += 0.03;
                break;
            case 1:
                lightPositionTwo.x += 0.03;
                break;
            case 2:
                lightPositionThree.x += 0.03;
                break;
            default:
                lightPositionFour.x += 0.03;
                break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        switch (activeLamp)
        {
            case 0:
                if (lightPositionOne.y >= 0.4f)
                    lightPositionOne.y -= 0.03;

                break;
            case 1:
                if (lightPositionTwo.y >= 0.4f)
                    lightPositionTwo.y -= 0.03;

                break;
            case 2:
                if (lightPositionThree.y >= 0.4f)
                    lightPositionThree.y -= 0.03;

                break;
            default:
                if (lightPositionFour.y >= 0.4f)
                    lightPositionFour.y -= 0.03;

                break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        switch (activeLamp)
        {
            case 0:
                lightPositionOne.y += 0.03;
                break;
            case 1:
                lightPositionTwo.y += 0.03;
                break;
            case 2:
                lightPositionThree.y += 0.03;
                break;
            default:
                lightPositionFour.y += 0.03;
                break;
        }
    }
}

void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height)  //  handle window resize using callback
{
    glViewport(0, 0, width, height);

    window_width = width;
    window_height = height;
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int)    //  handle mouse button callback
{
    if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS) 
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);         //  get click position coordinates

        int X = round(xpos);                            //  round coordinate values
        int Y = round(ypos);

        GLuint index;

        glReadPixels(X, window_height - Y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);    //  load index value from the stencil buffer

        if (index && index <= 96)
            clicked = index - 1;                        //  ignore border cubes and light source click
    }
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char * data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (data)
    {
        GLenum format;

        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        cout << "Texture failed to load at path: " << path << endl;
        stbi_image_free(data);
    }

    return textureID;
}
