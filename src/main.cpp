#include "shader.h"
#include "model.h"
#include "camera.h"
                                                        //  define all required GLFW callback functions
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow * window);

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

bool lampVisible = false;                               //  holds lamp object visibility
glm::vec3 lampPos(0.0f, 1.4f, 0.0f);                    //  initial position of light source
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);                 //  emitted color of light source

const int objectCount = 96;                             //  holds number of game objects(game board and figurines)
glm::vec3 positions[objectCount + 36];                  //  positions of all game objects(includes game board border)
glm::vec3 colors[objectCount + 36];                     //  colors of all game objects(includes game board border)
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

        linear = "ppppppppPPPPPPPPrnbqkbnrRNBQKBNR";
        real = "abcdefghABCDEFGHijklmnopIJKLMNOP";
    }

    string getLinear()                                  //  linear string getter
    {
        return linear;
    }

    char toLinear(char figurine)
    {
        for (int i = 0; i < 32; i++)
        {
            if (real[i] == figurine)
                return linear[i];
        }

        return '.';
    }

    bool calculateCheck()
    {
        //  call this method right before clicking on highlighted board field
        //  no figurine is going to have changed its range due to the check result
        //  clicking on highlighted movement field will do nothing when move is impossible due to the check
        //  so just test if selected movement/attack will not cause check state for you before movement

        if (blackTurn)
        {
            int kingX = 0;
            int kingY = 0;

            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)                 //  iterate through figurines array
                {
                    temp[i][j] = toLinear(figurines[i][j]);

                    if (temp[i][j] == 'k')
                    {
                        kingX = j;
                        kingY = i;
                    }
                }
            }

            int x = kingX;
            int y = kingY;

            //  check horizontal or vertical attack

            while (x > 0)
            {
                x--;

                if (temp[y][x] == 'Q' || temp[y][x] == 'R')
                    return true;
            }

            x = kingX;

            while (x < 7)
            {
                x++;

                if (temp[y][x] == 'Q' || temp[y][x] == 'R')
                    return true;
            }

            x = kingX;

            while (y > 0)
            {
                y--;

                if (temp[y][x] == 'Q' || temp[y][x] == 'R')
                    return true;
            }

            y = kingY;

            while (y < 7)
            {
                y++;

                if (temp[y][x] == 'Q' || temp[y][x] == 'R')
                    return true;
            }

            y = kingY;

            //  check diagonal attack

            while (x > 0 && y > 0)
            {
                x--;
                y--;

                if (temp[y][x] == 'Q' || temp[y][x] == 'B')
                    return true;
            }

            x = kingX;
            y = kingY;

            while (x > 0 && y < 7)
            {
                x--;
                y++;

                if (temp[y][x] == 'Q' || temp[y][x] == 'B')
                    return true;
            }

            x = kingX;
            y = kingY;

            while (x < 7 && y > 0)
            {
                x++;
                y--;

                if (temp[y][x] == 'Q' || temp[y][x] == 'B')
                    return true;
            }

            x = kingX;
            y = kingY;

            while (x < 7 && y < 7)
            {
                x++;
                y++;

                if (temp[y][x] == 'Q' || temp[y][x] == 'B')
                    return true;
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

            if (y && figurines[y - 1][x] == 'K')
                return true;

            if (y < 7 && figurines[y + 1][x] == 'K')
                return true;

            if (x && figurines[y][x - 1] == 'K')
                return true;

            if (x < 7 && figurines[y][x + 1] == 'K')
                return true;

            if (y && x && figurines[y - 1][x - 1] == 'K')
                return true;

            if (y && x < 7 && figurines[y - 1][x + 1] == 'K')
                return true;

            if (y < 7 && x && figurines[y + 1][x - 1] == 'K')
                return true;

            if (y < 7 && x < 7 && figurines[y + 1][x + 1] == 'K')
                return true;

            //  check attack by pawn

            if (y && x < 7 && figurines[y - 1][x + 1] == 'P')
                return true;

            if (y < 7 && x < 7 && figurines[y + 1][x + 1] == 'P')
                return true;
        }
        else
        {
            int kingX = 0;
            int kingY = 0;

            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)                 //  iterate through figurines array
                {
                    temp[i][j] = toLinear(figurines[i][j]);

                    if (temp[i][j] == 'K')
                    {
                        kingX = j;
                        kingY = i;
                    }
                }
            }

            int x = kingX;
            int y = kingY;

            //  check horizontal or vertical attack

            while (x > 0)
            {
                x--;

                if (temp[y][x] == 'q' || temp[y][x] == 'r')
                    return true;
            }

            x = kingX;

            while (x < 7)
            {
                x++;

                if (temp[y][x] == 'q' || temp[y][x] == 'r')
                    return true;
            }

            x = kingX;

            while (y > 0)
            {
                y--;

                if (temp[y][x] == 'q' || temp[y][x] == 'r')
                    return true;
            }

            y = kingY;

            while (y < 7)
            {
                y++;

                if (temp[y][x] == 'q' || temp[y][x] == 'r')
                    return true;
            }

            y = kingY;

            //  check diagonal attack

            while (x > 0 && y > 0)
            {
                x--;
                y--;

                if (temp[y][x] == 'q' || temp[y][x] == 'b')
                    return true;
            }

            x = kingX;
            y = kingY;

            while (x > 0 && y < 7)
            {
                x--;
                y++;

                if (temp[y][x] == 'q' || temp[y][x] == 'b')
                    return true;
            }

            x = kingX;
            y = kingY;

            while (x < 7 && y > 0)
            {
                x++;
                y--;

                if (temp[y][x] == 'q' || temp[y][x] == 'b')
                    return true;
            }

            x = kingX;
            y = kingY;

            while (x < 7 && y < 7)
            {
                x++;
                y++;

                if (temp[y][x] == 'q' || temp[y][x] == 'b')
                    return true;
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

            if (y && figurines[y - 1][x] == 'k')
                return true;

            if (y < 7 && figurines[y + 1][x] == 'k')
                return true;

            if (x && figurines[y][x - 1] == 'k')
                return true;

            if (x < 7 && figurines[y][x + 1] == 'k')
                return true;

            if (y && x && figurines[y - 1][x - 1] == 'k')
                return true;

            if (y && x < 7 && figurines[y - 1][x + 1] == 'k')
                return true;

            if (y < 7 && x && figurines[y + 1][x - 1] == 'k')
                return true;

            if (y < 7 && x < 7 && figurines[y + 1][x + 1] == 'k')
                return true;

            //  check attack by pawn

            if (y && x && figurines[y - 1][x - 1] == 'p')
                return true;

            if (y < 7 && x && figurines[y + 1][x - 1] == 'p')
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
                        if (j < 7 && figurines[i][j + 1] == '.')
                        {
                            board[i][j + 1] = 'M';      //  move
                        }

                        if (i && j < 7 && figurines[i - 1][j + 1] != '.' && isupper(figurines[i - 1][j + 1]))
                        {
                            board[i - 1][j + 1] = 'A';  //  attack
                        }

                        if (i < 7 && j < 7 && figurines[i + 1][j + 1] != '.' && isupper(figurines[i + 1][j + 1]))
                        {
                            board[i + 1][j + 1] = 'A';  //  attack
                        }
                    }
                    else if (ch == 'P')                 //  white pawn
                    {
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

                            if (board[i][j] == 'A')     //  attack
                                highlight[count] = 2;   //  save found coordinates into highlight array

                            count++;
                        }
                    }

                    return;
                }
            }
        }
    }

    void refreshFigurines(int index)                    //  rehresh game board after clicking onto highlighted game board board
    {
        if (selection[0] == -1 || selection[1] == -1)   //  do nothing when nothing is selected
            return;

        int count = 0;

        for (int j = 0; j < 8; j++)                     //  iterate through game board
        {
            for (int i = 0; i < 8; i++)
            {
                if (count == index)
                {
                    if (board[i][j] == 'A')             //  attack is going to be performed
                    {
                        char ch = figurines[i][j];

                        for (int k = 0; k < 32; k++)
                        {
                            if (real[k] == ch)
                            {
                                linear[k] = '.';        //  remove opponent's figurine from the game board
                                break;
                            }
                        }
                    }

                    figurines[i][j] = figurines[selection[0]][selection[1]];    //  move figurine to new position
                    figurines[selection[0]][selection[1]] = '.';    //  previous position is going to be empty
                    selection[0] = selection[1] = -1;

                    camera.toggle();                    //  toggle camera to trace second player
                    animationActive = true;             //  activate animation flag
                    blackTurn = !blackTurn;             //  opponent is now on the turn

                    return;
                }

                count++;
            }
        }
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

    void handle()                                       //  handles click on any game object and executes appropropriate reaction
    {
        if (animationActive)
            return;

        if (clicked >= 0)                               //  check if something was clicked
        {
            if (clicked < 64)                           //  game board clicked
            {
                if (highlight[clicked])                 //  check if selected position is highlighted
                {
                    refreshFigurines(clicked);

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
                        if ((clicked >= 64 && clicked < 72) || (clicked >= 80 && clicked < 88))
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
                        if ((clicked >= 72 && clicked < 80) || (clicked >= 88 && clicked < 96))
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
        if (!strcmp(argv[i], "-v"))
        {
            cout << "richers\n";
            cout << "version 1.0\n";
            return 0;
        }
        else if (!strcmp(argv[i], "-h"))
        {
            cout << "DESCRIPTION\n"
                 << "    richess - rich and amazing chess game\n\n"
                 << "USAGE\n"
                 << "    richess -h  ->  print usage information\n"
                 << "    richess -v  ->  print version information\n"
                 << "    richess -l  ->  set low details of shadows\n"
                 << "    richess -m  ->  set medium details of shadows\n"
                 << "    richess -h  ->  set high details of shadows\n"
                 << "    richess -w  ->  run application in windowed mode\n\n"
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
                 << "    u  ->  move light source down\n"
                 << "    o  ->  move light source up\n"
                 << "    p  ->  toggle light source visibility\n";

            return 0;
        }
        else if (!strcmp(argv[i], "-w"))
            fullscreen = false;
        else if (!strcmp(argv[i], "-l"))
            details = 0;
        else if (!strcmp(argv[i], "-m"))
            details = 1;
        else if (!strcmp(argv[i], "-h"))
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
        window = glfwCreateWindow(window_width, window_height, "Chess", glfwGetPrimaryMonitor(), nullptr);
    else
        window = glfwCreateWindow(window_width, window_height, "Chess", nullptr, nullptr);

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

    Model cube("resources/vertices/block.txt", "resources/indices/block.txt");  //  load cube model from file
    Model rook("resources/vertices/rook.txt", "resources/indices/rook.txt");    //  load rook model from file
    Model knight("resources/vertices/knight.txt", "resources/indices/knight.txt");  //  load knight model from file
    Model bishop("resources/vertices/bishop.txt", "resources/indices/bishop.txt");  //  load bishop model from file
    Model king("resources/vertices/king.txt", "resources/indices/king.txt");    //  load king model from file
    Model queen("resources/vertices/queen.txt", "resources/indices/queen.txt"); //  load queen model from file
    Model pawn("resources/vertices/pawn.txt", "resources/indices/pawn.txt");    //  load pawn model from file
    Model lamp("resources/vertices/cube.txt", "resources/indices/cube.txt");    //  load lamp model from file

    glm::vec3 cubeBrownColor(rgbToFloats(139, 69, 19));     //  set RGB values of all required colors
    glm::vec3 cubeWhiteColor(rgbToFloats(255, 211, 155));
    glm::vec3 figurineBrownColor(rgbToFloats(139, 69, 19));
    glm::vec3 figurineWhiteColor(rgbToFloats(255, 211, 155));
    glm::vec3 greenHighlightColor(rgbToFloats(0, 201, 87));
    glm::vec3 blueHighlightColor(rgbToFloats(30, 144, 255));
    glm::vec3 redHighlightColor(rgbToFloats(255, 20, 80));

    for (int i = 0; i < 8; i++)                         //  initialize first row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[i] = glm::vec3(offset, 0.0f, 0.0f);
        highlight[i] = 0;                               //  nothing is highlighted by default

        if (i % 2 == 1)
            colors[i] = cubeBrownColor;
        else
            colors[i] = cubeWhiteColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize second row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[8 + i] = glm::vec3(offset, 0.0f, 0.4f);
        highlight[8 + i] = 0;                           //  nothing is highlighted by default

        if (i % 2 == 1)
            colors[8 + i] = cubeWhiteColor;
        else
            colors[8 + i] = cubeBrownColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize third row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[16 + i] = glm::vec3(offset, 0.0f, 0.8f);
        highlight[16 + i] = 0;                          //  nothing is highlighted by default

        if (i % 2 == 1)
            colors[16 + i] = cubeBrownColor;
        else
            colors[16 + i] = cubeWhiteColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize fourth row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[24 + i] = glm::vec3(offset, 0.0f, 1.2f);
        highlight[24 + i] = 0;                          //  nothing is highlighted by default

        if (i % 2 == 1)
            colors[24 + i] = cubeWhiteColor;
        else
            colors[24 + i] = cubeBrownColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize fifth row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[32 + i] = glm::vec3(offset, 0.0f, 1.6f);
        highlight[32 + i] = 0;                          //  nothing is highlighted by default

        if (i % 2 == 1)
            colors[32 + i] = cubeBrownColor;
        else
            colors[32 + i] = cubeWhiteColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize sixth row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[40 + i] = glm::vec3(offset, 0.0f, 2.0f);
        highlight[40 + i] = 0;                          //  nothing is highlighted by default

        if (i % 2 == 1)
            colors[40 + i] = cubeWhiteColor;
        else
            colors[40 + i] = cubeBrownColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize seventh row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[48 + i] = glm::vec3(offset, 0.0f, 2.4f);
        highlight[48 + i] = 0;                          //  nothing is highlighted by default

        if (i % 2 == 1)
            colors[48 + i] = cubeBrownColor;
        else
            colors[48 + i] = cubeWhiteColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize eighth row of game board blocks
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[56 + i] = glm::vec3(offset, 0.0f, 2.8f);
        highlight[56 + i] = 0;                          //  nothing is highlighted by default

        if (i % 2 == 1)
            colors[56 + i] = cubeWhiteColor;
        else
            colors[56 + i] = cubeBrownColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize black pawns
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[64 + i] = glm::vec3(offset, 0.36f, 0.4f);
        highlight[64 + i] = 0;                          //  nothing is highlighted by default
        colors[64 + i] = figurineBrownColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize white pawns
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[72 + i] = glm::vec3(offset, 0.36f, 2.4f);
        highlight[72 + i] = 0;                          //  nothing is highlighted by default
        colors[72 + i] = figurineWhiteColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize black non-pawns
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[80 + i] = glm::vec3(offset, 0.36f, 0.0f);
        highlight[80 + i] = 0;                          //  nothing is highlighted by default
        colors[80 + i] = figurineBrownColor;
    }

    for (int i = 0; i < 8; i++)                         //  initialize white non-pawns
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[88 + i] = glm::vec3(offset, 0.36f, 2.8f);
        highlight[88 + i] = 0;                          //  nothing is highlighted by default
        colors[88 + i] = figurineWhiteColor;
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[96 + i] = glm::vec3(offset, -0.05f, -0.4f);
        colors[96 + i] = rgbToFloats(160 - 10 * i, 160 - 10 * i, 160 - 10 * i);
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f - 0.4f;          //  handle object offset

        positions[105 + i] = glm::vec3(offset, -0.05f, 3.2f);
        colors[105 + i] = rgbToFloats(80 + 10 * i, 80 + 10 * i, 80 + 10 * i);
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f;                 //  handle object offset

        positions[114 + i] = glm::vec3(3.2f, -0.05f, offset);
        colors[114 + i] = rgbToFloats(80 + 10 * i, 80 + 10 * i, 80 + 10 * i);
    }

    for (int i = 0; i < 9; i++)                         //  initialize game board border
    {
        float offset = float(i) * 0.4f - 0.4f;          //  handle object offset

        positions[123 + i] = glm::vec3(-0.4f, -0.05f, offset);
        colors[123 + i] = rgbToFloats(160 - 10 * i, 160 - 10 * i, 160 - 10 * i);
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

        //  render scene only to the depth texture for future calculation of shadows

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;      //  setup view frustum
        
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);  //  enable orthographic view when rendering shadows
        lightView = glm::lookAt(lampPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        simpleDepthShader.activate();                   //  activate appropriate shader
        simpleDepthShader.passMatrix("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); //  use depth map
        glClear(GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < 64; i++)                    //  render all game board block
        {
            glm::mat4 modelOne;
            modelOne = glm::translate(modelOne, positions[i]);  //  translate and scale object before rendering
            modelOne = glm::scale(modelOne, glm::vec3(0.2f, 0.2f, 0.2f));

            if (highlight[i] == 1)
                simpleDepthShader.passVector("objectColor", blueHighlightColor);
            else if (highlight[i] == 2)
                simpleDepthShader.passVector("objectColor", redHighlightColor);
            else
                simpleDepthShader.passVector("objectColor", colors[i]);

            simpleDepthShader.passMatrix("model", modelOne);
            // glStencilFunc(GL_ALWAYS, i + 1, -1);
            cube.render();
        }

        for (int i = 64; i < 96; i++)                   //  render all figurines
        {
            glm::mat4 modelOne;
            modelOne = glm::translate(modelOne, positions[i]);  //  translate and scale object before rendering
            modelOne = glm::scale(modelOne, glm::vec3(0.08f, 0.08f, 0.08f));

            if (highlight[i])
                simpleDepthShader.passVector("objectColor", greenHighlightColor);
            else
                simpleDepthShader.passVector("objectColor", colors[i]);

            simpleDepthShader.passMatrix("model", modelOne);
            // glStencilFunc(GL_ALWAYS, i + 1, -1);

            string linear = chess.getLinear();
                                                        //  check type of figure using value inside linear array
            if (linear[i - 64] == 'p' || linear[i - 64] == 'P')
                pawn.render();
            else if (linear[i - 64] == 'r' || linear[i - 64] == 'R')
                rook.render();
            else if (linear[i - 64] == 'n')
            {
                modelOne = glm::rotate(modelOne, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                simpleDepthShader.passMatrix("model", modelOne);
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

        for (int i = 96; i < 132; i++)                  //  render game board border cubes
        {
            glm::mat4 modelOne;
            modelOne = glm::translate(modelOne, positions[i]);  //  translate and scale object before rendering
            modelOne = glm::scale(modelOne, glm::vec3(0.2f, 0.2f, 0.2f));
            simpleDepthShader.passVector("objectColor", colors[i]);
            simpleDepthShader.passMatrix("model", modelOne);
            // glStencilFunc(GL_ALWAYS, i + 1, -1);
            cube.render();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        //  render complete scene now, but use previously rendered depth map

        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.activate();                              //  activate appropriate shader
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
        glm::mat4 view = camera.loadViewMatrix();
        shader.passMatrix("projection", projection);
        shader.passMatrix("view", view);
        // set light uniforms
        shader.passVector("viewPos", camera.Position);
        shader.passVector("lightPos", lampPos);
        shader.passMatrix("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        
        for (int i = 0; i < 64; i++)                    //  render all game board block
        {
            // render the loaded model
            glm::mat4 modelOne;
            modelOne = glm::translate(modelOne, positions[i]);  //  translate and scale object before rendering
            modelOne = glm::scale(modelOne, glm::vec3(0.2f, 0.2f, 0.2f));

            if (highlight[i] == 1)
                shader.passVector("objectColor", blueHighlightColor);
            else if (highlight[i] == 2)
                shader.passVector("objectColor", redHighlightColor);
            else
                shader.passVector("objectColor", colors[i]);

            shader.passMatrix("model", modelOne);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer
            cube.render();
        }

        for (int i = 64; i < 96; i++)                   //  render all figurines
        {
            glm::mat4 modelOne;
            modelOne = glm::translate(modelOne, positions[i]);  //  translate and scale object before rendering
            modelOne = glm::scale(modelOne, glm::vec3(0.08f, 0.08f, 0.08f));

            if (highlight[i])
                shader.passVector("objectColor", greenHighlightColor);
            else
                shader.passVector("objectColor", colors[i]);

            shader.passMatrix("model", modelOne);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer

            string linear = chess.getLinear();
                                                        //  check type of figure using value inside linear array
            if (linear[i - 64] == 'p' || linear[i - 64] == 'P')
                pawn.render();
            else if (linear[i - 64] == 'r' || linear[i - 64] == 'R')
                rook.render();
            else if (linear[i - 64] == 'n')
            {
                modelOne = glm::rotate(modelOne, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                shader.passMatrix("model", modelOne);
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

        for (int i = 96; i < 132; i++)                  //  render game board border cubes
        {
            // render the loaded model
            glm::mat4 modelOne;
            modelOne = glm::translate(modelOne, positions[i]);  //  translate and scale object before rendering
            modelOne = glm::scale(modelOne, glm::vec3(0.2f, 0.2f, 0.2f));
            shader.passVector("objectColor", colors[i]);
            shader.passMatrix("model", modelOne);
            glStencilFunc(GL_ALWAYS, i + 1, -1);        //  write object into the stencil buffer
            cube.render();
        }

        //  render lamp object without shadow mapping with its own shader

        if (lampVisible)                                //  check whether lamp object should be visible
        {
            lampShader.activate();                      //  activate appropriate shader
            lampShader.passVector("lightColor", lightColor);

            // view/projection transformations
            glm::mat4 projectionThree = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
            glm::mat4 viewThree = camera.loadViewMatrix();
            lampShader.passMatrix("projection", projectionThree);
            lampShader.passMatrix("view", viewThree);

            // render the loaded model
            glm::mat4 modelThree;
            modelThree = glm::translate(modelThree, glm::vec3(lampPos));    //  translate and scale object before rendering
            modelThree = glm::scale(modelThree, glm::vec3(0.1f, 0.1f, 0.1f));
            lampShader.passMatrix("model", modelThree);
            glStencilFunc(GL_ALWAYS, 97, -1);           //  write object into the stencil buffer
            lamp.render();
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
        lampPos.z -= 0.03;

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        lampPos.z += 0.03;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        lampPos.x += 0.03;

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        lampPos.x -= 0.03;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        if (lampPos.y >= 0.4f)
            lampPos.y -= 0.03;
    }

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        lampPos.y += 0.03;
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
