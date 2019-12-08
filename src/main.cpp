#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"
#include "model.h"
#include "camera.h"
                                                        //  define all required GLFW callback functions
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow * window);
unsigned int loadTexture(const char *path);

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

Camera camera;                                          //  create camera instance and use default camera position

bool lampOrbit = false;                                 //  holds if lamp orbit is active
bool lampVisible = false;                               //  holds lamp object visibility

glm::vec3 lightPosition(-3.5f, 3.5f, 0.0f);
glm::vec3 lightColor(150.0f, 150.0f, 150.0f);           //  emitted color of light source

const int objectCount = 96;                             //  holds number of game objects(game board and figurines)
glm::vec3 positions[objectCount + 36];                  //  positions of all game objects(includes game board border)
bool isWhite[objectCount + 36];                         //  colors of all game objects(includes game board border)
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
                    camera.toggle();                    //  toggle camera to trace second player
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
                                camera.toggle();        //  toggle camera to trace second player
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
                                camera.toggle();        //  toggle camera to trace second player
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

    bool fullscreen = true;
    int details = 1;

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
                 << "    richess --help     ->  print usage information\n"
                 << "    richess --version  ->  print version information\n"
                 << "    richess --low      ->  set low details of shadows\n"
                 << "    richess --medium   ->  set medium details of shadows\n"
                 << "    richess --high     ->  set high details of shadows\n"
                 << "    richess --window   ->  run application in windowed mode\n\n"
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
                 << "    i  ->  move light source forward\n"
                 << "    k  ->  move light source backward\n"
                 << "    j  ->  move light source left\n"
                 << "    l  ->  move light source right\n"
                 << "    u  ->  move light source up\n"
                 << "    h  ->  move light source down\n"
                 << "    p  ->  toggle light source visibility\n"
                 << "    o  ->  toggle light source orbit\n";

            return 0;
        }
        else if (!strcmp(argv[i], "--window") || !strcmp(argv[i], "-w"))
            fullscreen = false;
        else if (!strcmp(argv[i], "--low"))
            details = 0;
        else if (!strcmp(argv[i], "--medium"))
            details = 1;
        else if (!strcmp(argv[i], "--high"))
            details = 2;
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

    glewExperimental = GL_TRUE;                         //  initialize glew library
    glewInit();

    glEnable(GL_DEPTH_TEST);                            //  enable depth and stencil test
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_MULTISAMPLE);                           //  multisampling is going to be used

    Shader lampShader("lamp.vs", "lamp.fs");            //  setup and compile shaders
    Shader simpleDepthShader("depth.vs", "depth.fs");
    Shader shader("shadow.vs", "shadow.fs");
                                                        //  setup shadow rendering
    unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;           //  shadow resolution

    if (details == 1)                                   //  toggle shadows resolution by the appropriate flag value
        SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    else if (details == 2)
        SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    else
        SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
                                                        //  create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
                                                        //  attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Chess chess;                                        //  create chess game logic instance

    shader.activate();                                  //  activate and configure shader
    shader.passInteger("diffuseTexture", 0);
    shader.passInteger("shadowMap", 1);

    Model cube("resources/block/positions.txt", "resources/block/normals.txt", "resources/block/indices.txt", "resources/block/uv.txt");
    Model rook("resources/rook/positions.txt", "resources/rook/normals.txt", "resources/rook/indices.txt", "resources/rook/uv.txt");
    Model knight("resources/knight/positions.txt", "resources/knight/normals.txt", "resources/knight/indices.txt", "resources/knight/uv.txt");
    Model bishop("resources/bishop/positions.txt", "resources/bishop/normals.txt", "resources/bishop/indices.txt", "resources/bishop/uv.txt");
    Model king("resources/king/positions.txt", "resources/king/normals.txt", "resources/king/indices.txt", "resources/king/uv.txt");
    Model queen("resources/queen/positions.txt", "resources/queen/normals.txt", "resources/queen/indices.txt", "resources/queen/uv.txt");
    Model pawn("resources/pawn/positions.txt", "resources/pawn/normals.txt", "resources/pawn/indices.txt", "resources/pawn/uv.txt");

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
        isWhite[96 + i] = false;
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f - 0.4f;          //  handle object offset

        positions[105 + i] = glm::vec3(offset, -0.03f, 3.2f);
        isWhite[105 + i] = false;
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[114 + i] = glm::vec3(3.2f, -0.03f, offset);
        isWhite[114 + i] = false;
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f - 0.4f;          //  handle object offset

        positions[123 + i] = glm::vec3(-0.4f, -0.03f, offset);
        isWhite[123 + i] = false;
    }

    for (int i = 0; i < 132; i++)                       //  move all game objects into the center of coordinate system
    {
        positions[i].x -= 1.4f;
        positions[i].z -= 1.4f;
    }

    //  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        Shader shaderPBR("pbr.vs", "pbr.fs");

        shaderPBR.activate();
        shaderPBR.passInteger("albedoMap", 0);
        shaderPBR.passInteger("normalMap", 1);
        shaderPBR.passInteger("metallicMap", 2);
        shaderPBR.passInteger("roughnessMap", 3);
        shaderPBR.passInteger("aoMap", 4);

        unsigned int albedoRed = loadTexture("resources/textures/highlight/red/albedo.png");
        unsigned int albedoGreen = loadTexture("resources/textures/highlight/green/albedo.png");
        unsigned int albedoBlue = loadTexture("resources/textures/highlight/blue/albedo.png");
        unsigned int albedoYellow = loadTexture("resources/textures/highlight/yellow/albedo.png");
        unsigned int albedoWhite = loadTexture("resources/textures/highlight/white/albedo.png");

        unsigned int albedoGranite = loadTexture("resources/textures/granite/albedo.png");
        unsigned int normalGranite = loadTexture("resources/textures/granite/normal.png");
        unsigned int metallicGranite = loadTexture("resources/textures/granite/metallic.png");
        unsigned int roughnessGranite = loadTexture("resources/textures/granite/roughness.png");
        unsigned int aoGranite = loadTexture("resources/textures/granite/ao.png");

        unsigned int albedoWood = loadTexture("resources/textures/wood/albedo.png");
        unsigned int normalWood = loadTexture("resources/textures/wood/normal.png");
        unsigned int metallicWood = loadTexture("resources/textures/wood/metallic.png");
        unsigned int roughnessWood = loadTexture("resources/textures/wood/roughness.png");
        unsigned int aoWood = loadTexture("resources/textures/wood/ao.png");

        unsigned int albedoPlanks = loadTexture("resources/textures/planks/albedo.png");
        unsigned int normalPlanks = loadTexture("resources/textures/planks/normal.png");
        unsigned int metallicPlanks = loadTexture("resources/textures/planks/metallic.png");
        unsigned int roughnessPlanks = loadTexture("resources/textures/planks/roughness.png");
        unsigned int aoPlanks = loadTexture("resources/textures/planks/ao.png");

        unsigned int albedoMarble = loadTexture("resources/textures/marble/albedo.png");
        unsigned int normalMarble = loadTexture("resources/textures/marble/normal.png");
        unsigned int metallicMarble = loadTexture("resources/textures/marble/metallic.png");
        unsigned int roughnessMarble = loadTexture("resources/textures/marble/roughness.png");
        unsigned int aoMarble = loadTexture("resources/textures/marble/ao.png");

        unsigned int albedoFoam = loadTexture("resources/textures/foam/albedo.png");
        unsigned int normalFoam = loadTexture("resources/textures/foam/normal.png");
        unsigned int metallicFoam = loadTexture("resources/textures/foam/metallic.png");
        unsigned int roughnessFoam = loadTexture("resources/textures/foam/roughness.png");
        unsigned int aoFoam = loadTexture("resources/textures/foam/ao.png");

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
        shaderPBR.activate();
        shaderPBR.passMatrix("projection", projection);

    //  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    while (!glfwWindowShouldClose(window))              //  handle infinite render loop
    {
        processInput(window);                           //  process keyboard input

        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);    //  update background color of window
        glClearStencil(0);                              //  clear stencil buffer and set default index value
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);     //  clear all required buffers

        chess.handle();                                 //  handle chess logic turn
        
        if (chess.animationActive)                      //  check if camera animation is activated
        {
            chess.actual = glfwGetTime();               //  get actual timestamp

            if (chess.actual - chess.previous > 0.01)   //  check if time after last animation step is a required chunk
            {
                if (camera.animateToggle())             //  handle camera movement
                    chess.previous = chess.actual;
                else
                    chess.animationActive = false;      //  disable animation after successfull camera movement
            }
        }

        if (lampOrbit)
        {
            float radius = 3.0f;
            lightPosition.x = sin(glfwGetTime() * 0.1f) * radius;     //  orbit light source around the game board
            lightPosition.z = cos(glfwGetTime() * 0.1f) * radius;
        }

        // reset viewport
        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        shaderPBR.activate();
        glm::mat4 view = glm::mat4(1.0f);
        view = camera.loadViewMatrix();
        shaderPBR.passMatrix("view", view);
        shaderPBR.passVector("camPos", camera.Position);

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
                glBindTexture(GL_TEXTURE_2D, normalFoam);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, metallicFoam);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessFoam);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoFoam);
            }
            else if (highlight[i] == 2)                     //  attack
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, albedoRed);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, normalFoam);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, metallicFoam);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessFoam);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoFoam);
            }
            else if (highlight[i] == 3)                     //  castling
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, albedoYellow);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, normalFoam);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, metallicFoam);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessFoam);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoFoam);
            }
            else
            {
                if (isWhite[i] == true)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, albedoWood);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, normalWood);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, metallicWood);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, roughnessWood);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, aoWood);
                }
                else
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, albedoPlanks);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, normalPlanks);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, metallicPlanks);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, roughnessPlanks);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, aoPlanks);
                }
            }

            shaderPBR.passMatrix("model", model);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer
            cube.render();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoWood);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalWood);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallicWood);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessWood);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, aoWood);

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
                glBindTexture(GL_TEXTURE_2D, normalGranite);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, metallicGranite);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, roughnessGranite);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, aoGranite);
            }
            else
            {
                if (isWhite[i] == true)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, albedoMarble);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, normalMarble);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, metallicMarble);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, roughnessMarble);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, aoMarble);
                }
                else
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, albedoGranite);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, normalGranite);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, metallicGranite);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, roughnessGranite);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, aoGranite);
                }
            }

            shader.passMatrix("model", model);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer

            string linear = chess.getLinear();
                                                        //  check type of figure using value inside linear array
            if (linear[i - 64] == 'p' || linear[i - 64] == 'P')
                pawn.render();
            else if (linear[i - 64] == 'r' || linear[i - 64] == 'R')
                rook.render();
            else if (linear[i - 64] == 'n')
            {
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                shaderPBR.passMatrix("model", model);
                knight.render();
            }
            else if (linear[i - 64] == 'N')
                knight.render();
            else if (linear[i - 64] == 'b' || linear[i - 64] == 'B')
                bishop.render();
            else if (linear[i - 64] == 'q' || linear[i - 64] == 'Q')
                queen.render();
            else if (linear[i - 64] == 'k' || linear[i - 64] == 'K')
                king.render();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoFoam);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalFoam);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallicFoam);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessFoam);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, aoFoam);

        for (int i = 96; i < 132; i++)                  //  render game board border cubes
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, positions[i]);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            shaderPBR.passMatrix("model", model);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer
            cube.render();
        }

        shaderPBR.passVector("lightPositions[0]", lightPosition);
        shaderPBR.passVector("lightColors[0]", lightColor);

        if (lampVisible)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, albedoWhite);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalFoam);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, metallicFoam);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, roughnessFoam);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, aoFoam);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPosition);
            model = glm::scale(model, glm::vec3(0.1f));
            shaderPBR.passMatrix("model", model);
            glStencilFunc(GL_ALWAYS, 97, -1);           //  write object into the stencil buffer
            cube.render();
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

            lightPosition.x = 0.0f;
            lightPosition.y = 1.4f;
            lightPosition.z = 0.0f;
        }
        else                                            //  disable lamp orbit
        {
            lampOrbit = true;

            lightPosition.x = 0.0f;
            lightPosition.y = 2.4f;
            lightPosition.z = 0.0f;
        }
    }
}

void processInput(GLFWwindow * window)                  //  non-callback keyboard input
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);         //  close window
                                                        //  move camera by pressing appropriate buttons
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.tiltUp();

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.tiltDown();

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.rotateLeft();

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.rotateRight();

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        camera.zoomOut();

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        camera.zoomIn();
                                                        //  move light source by pressing appropriate buttons
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        lightPosition.z += 0.03;

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        lightPosition.z -= 0.03;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        lightPosition.x -= 0.03;

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        lightPosition.x += 0.03;

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        if (lightPosition.y >= 0.4f)
            lightPosition.y -= 0.03;
    }

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        lightPosition.y += 0.03;
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
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
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
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
