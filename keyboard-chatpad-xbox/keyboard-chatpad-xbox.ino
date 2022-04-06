/***********************************************************************
* Copyright 2022 Cleverson S A
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*  
*  Chatpad Knoff Teensy port
*  
*  Based on original post: https://hackaday.io/page/2329-xbox-chatpad
*  
*  REMEMBER: DO IT BY YOUR OWN RISC, BECAUSE THIS CODE DOES NOT RUN
*            with the default blob controller on the board. You have
*            to hard remove the chip to let Teensy all the control of
*            the keyboard. 
*            
 ***********************************************************************/
#include "Keyboard.h"

/***********************************************************************
 * Globals and definitions
 ***********************************************************************/
#define SCAN_DELAY_TIMEOUT 195      /* If you set a low value, ghosthing characters will occour */

#define R_TP1_PIN  PIN_B2
#define R_TP20_PIN PIN_D0
#define R_TP21_PIN PIN_D1
#define R_TP22_PIN PIN_F5
#define R_TP23_PIN PIN_B7
#define R_TP24_PIN PIN_B3
#define R_TP25_PIN PIN_B1

#define C_TP6_PIN PIN_D2
#define C_TP7_PIN PIN_D3
#define C_TP8_PIN PIN_C6
#define C_TP9_PIN PIN_C7
#define C_TP10_PIN PIN_F1
#define C_TP11_PIN PIN_F6
#define C_TP12_PIN PIN_F7

#define BACKSPACE -1
#define LEFT      -2
#define RIGHT     -3
#define UP        -10
#define DOWN      -11
#define CAPS      -12
#define PG_UP     -13
#define PG_DWN    -14
#define HOME      -15
#define END       -16
#define ESC       -17
#define R_SELECT  -18
#define L_SELECT  -19
#define TAB       -20
#define F1        -21
#define F2        -22
#define F3        -23
#define F4        -24
#define F5        -25
#define F6        -26
#define F7        -27
#define F8        -28
#define F9        -29
#define F10       -30
#define SCUT1     -31
#define SCUT2     -32
#define SCUT3     -33
#define SCUT4     -34
#define SCUT5     -35
#define SCUT6     -36
#define SCUT7     -37
#define SCUT8     -38
#define SCUT9     -39
#define SCUT0     -40
#define GREEN     -4
#define RED       -5
#define CHAT      -6
#define ENTER     -7
#define SHIFT     -8
#define NORMAL    -9


#define MODE_SHIFT  1000
#define MODE_GREEN  1010
#define MODE_RED    1100
#define MODE_NORMAL 1110
#define MODE_CTRL   1111
#define MODE_ALT    1002

int current_mode = MODE_NORMAL,
    in_ctrl_mode = 0,
    in_alt_mode = 0;
    
int rows[7] = {R_TP1_PIN, R_TP20_PIN, R_TP21_PIN, R_TP22_PIN, R_TP23_PIN, R_TP24_PIN, R_TP25_PIN};
int cols[7] = {C_TP6_PIN, C_TP7_PIN, C_TP8_PIN, C_TP9_PIN, C_TP10_PIN, C_TP11_PIN, C_TP12_PIN};

/***********************************************************************
 * Strings shortcut (userful on RPi Only)
 ***********************************************************************/
char shortcut[10][60]= {
  "sudo shutdown -h now\n",
  "vim /home/pi/Documents/quick_note_$(date +\"%Y%m%d\").txt\n",
  "vim /home/pi/Documents/general_note.txt\n",
  "bc\n",
  "sudo apt-get update ",
  "sudo apt install ",
  "sudo vim /etc/wpa_supplicant/wpa_supplicant.conf\n",
  "sudo mount /dev/sda1 /media/pendrive\n",
  "sudo umount /media/pendrive\n",
  "sudo reboot\n"
};

/***********************************************************************
 * Keyboard mapping
 ***********************************************************************/
char characters [7][7] =
{
   /* 6         7      8      9   10    11     12 */
  {BACKSPACE, 'k'   , 'i' , 'o' , ' ', ' '  , 'l'}, // TP1
  {'7'      , '1'   , '2' , '3' , '4', '5'  , '6'}, // TP20
  {'u'      , 'q'   , 'w' , 'e' , 'r', 't'  , 'y'}, // TP21
  {'j'      , 'a'   , 's' , 'd' , 'f', 'g'  , 'h'}, // TP22
  {'n'      , SHIFT , 'z' , 'x' , 'c', 'v'  , 'b'}, // TP23
  {TAB      , GREEN , CHAT, LEFT, ' ', '.'  , 'm'}, // TP24
  {RED      , '8'   , '9' , '0' , 'p', ENTER, ','}  // TP25
};

char shift_characters [7][7] =
{
   /* 6         7       8      9      10    11     12 */
  {BACKSPACE, 'K'   ,    'I',   'O', ' ', ' '  , 'L'}, // TP1
  {'7'      , '1'   ,    '2',   '3', '4', '5'  , '6'}, // TP20
  {'U'      , 'Q'   ,    'W',   'E', 'R', 'T'  , 'Y'}, // TP21
  {'J'      , 'A'   ,    'S',   'D', 'F', 'G'  , 'H'}, // TP22
  {'N'      , SHIFT ,    'Z',   'X', 'C', 'V'  , 'B'}, // TP23
  {RIGHT    , GREEN ,   CHAT,  LEFT, ' ', '.'  , 'M'}, // TP24
  {RED      , '8'   ,   '9' ,   '0', 'P', ENTER, ','}  // TP25
};

char red_characters [7][7] =
{
   /* 6         7       8      9      10    11     12 */
  {BACKSPACE,   'k',    'i', PG_UP,    ' ',  ' ',PG_DWN}, // TP1
  {F7       ,    F1,     F2,    F3,     F4,    F5,   F6}, // TP20
  {'u'      ,   ESC,     UP,   'e',    '$',   't',  'y'}, // TP21
  {'"'      ,  LEFT,   DOWN, RIGHT,    'e',   'y', '\\'}, // TP22
  {'n'      , CAPS ,    'z',   'x',    'c',   '_',  '+'}, // TP23
  {END      , GREEN,   CHAT,  HOME,    ' ',   ' ',  'u'}, // TP24
  {RED      , F8   ,     F9,  F10  ,    '=', ENTER,  ';'} // TP25
};    

char green_characters [7][7] =
{
   /* 6         7       8      9      10    11     12 */
  {BACKSPACE,   '[',   '*',   '(',    ' ',   ' ',  ']'}, // TP1
  {SCUT7    ,SCUT1 ,SCUT2 ,SCUT3 ,SCUT4 ,SCUT5 ,SCUT6 }, // TP20
  {'&'      ,   '!',   '@',   'E',    '#',   '%',  '^'}, // TP21
  {'\''     ,   '~',   ' ',   '{',    '}',   ' ',  '/'}, // TP22
  {'<'      , SHIFT,   '`',   '<',    '>',   '-',  '|'}, // TP23
  {RIGHT    , GREEN, CHAT , LEFT ,    ' ',   '?',  '>'}, // TP24
  {RED      ,SCUT8 ,SCUT9 ,SCUT0 ,    ')', ENTER,  ':'} // TP25
};


/***********************************************************************
 * Prepare
 ***********************************************************************/
void setup()
{
  for (int i = 0; i < 7; i++)
    pinMode(rows[i], OUTPUT);
  
  for (int i = 0; i < 7; i++)
    pinMode(cols[i], INPUT_PULLUP);
  
  Keyboard.begin();
}


/***********************************************************************
 * Main
 ***********************************************************************/
void loop()
{
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(rows[i], LOW);
    for (int j = 0; j < 7; j++)
    {
      int pinstate = digitalRead(cols[j]);
      int key = get_matrix_value(i,j,current_mode);
      
      if (pinstate == LOW)
      {
        press_key(key);
        
        if (key == SHIFT)
          change_current_mode(MODE_SHIFT);
        else if (key == GREEN)
          change_current_mode(MODE_GREEN);
        else if (key == RED)
          change_current_mode(MODE_RED);
        else if (key == CHAT)
        {
          change_current_mode(MODE_CTRL);
          in_ctrl_mode = 1;
        }
        else if (key == LEFT && 
            !(current_mode == MODE_SHIFT) &&
            !(current_mode == MODE_GREEN) &&
            !(current_mode == MODE_RED))
        {
          change_current_mode(MODE_ALT);
          in_alt_mode = 1;
        }
         
        if (key > 0)
        {
          if (in_ctrl_mode == 1) {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press(key);
            delay(SCAN_DELAY_TIMEOUT);
            in_ctrl_mode = 0;
            Keyboard.releaseAll();
          } else if (in_alt_mode == 1) {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press(key);
            delay(SCAN_DELAY_TIMEOUT);
            in_alt_mode = 0;
            Keyboard.releaseAll();
          } else
            Keyboard.write(key);
          change_current_mode(MODE_NORMAL);
        }
        
        delay(SCAN_DELAY_TIMEOUT);
      }
    }
    digitalWrite(rows[i], HIGH);
    
  }
}


/***********************************************************************
 * Functions and procedures
 ***********************************************************************/

void change_current_mode(int mode)
{
   if (current_mode == mode) 
     current_mode = MODE_NORMAL;
   else
      current_mode = mode;  
   
    /*switch(current_mode)
    {
      case(MODE_SHIFT): Keyboard.print("S"); break;
      case(MODE_GREEN): Keyboard.print("G"); break;
      case(MODE_RED)  : Keyboard.print("R"); break;
      default:
        Keyboard.print("N");
    }*/

}

int get_matrix_value(int row, int col, int current_mode) 
{
    switch(current_mode)
    {
      case(MODE_SHIFT): return shift_characters[row][col];
      case(MODE_GREEN): return green_characters[row][col];
      case(MODE_RED)  : return red_characters[row][col];
      default:
        return characters[row][col];
    }
}

void send_shortcut(int scid){

  int id = 0;
  switch(scid) {
    
    case(SCUT0): id = 0; break;
    case(SCUT1): id = 1; break;
    case(SCUT2): id = 2; break;
    case(SCUT3): id = 3; break;
    case(SCUT4): id = 4; break;
    case(SCUT5): id = 5; break;
    case(SCUT6): id = 6; break;
    case(SCUT7): id = 7; break;
    case(SCUT8): id = 8; break;
    case(SCUT9): id = 9; break;
    default:
      id=0;
  }

  Keyboard.print(shortcut[id]);
  
}

void press_key(int key)
{
  switch(key) 
  {
      case(CAPS):
        Keyboard.press(KEY_CAPS_LOCK);
        Keyboard.release(KEY_CAPS_LOCK);
        change_current_mode(MODE_NORMAL);
        break;

      case(ENTER):
        Keyboard.press(KEY_RETURN);
        Keyboard.release(KEY_RETURN);
        break;
        
      case(BACKSPACE):
        Keyboard.press(KEY_BACKSPACE);
        Keyboard.release(KEY_BACKSPACE);
        break;
        
      case(DOWN):
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.release(KEY_DOWN_ARROW);
        break;

      case(UP):
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.release(KEY_UP_ARROW);
        break;

      case(TAB):
        Keyboard.press(KEY_TAB);
        Keyboard.release(KEY_TAB);
        break;

      case(F1):
        Keyboard.press(KEY_F1);
        Keyboard.release(KEY_F1);
        break;

      case(F2):
        Keyboard.press(KEY_F2);
        Keyboard.release(KEY_F2);
        break;

      case(F3):
        Keyboard.press(KEY_F3);
        Keyboard.release(KEY_F3);
        break;

      case(F4):
        Keyboard.press(KEY_F4);
        Keyboard.release(KEY_F4);
        break;

      case(F5):
        Keyboard.press(KEY_F5);
        Keyboard.release(KEY_F5);
        break;

      case(F6):
        Keyboard.press(KEY_F6);
        Keyboard.release(KEY_F6);
        break;

      case(F7):
        Keyboard.press(KEY_F7);
        Keyboard.release(KEY_F7);
        break;

      case(F8):
        Keyboard.press(KEY_F8);
        Keyboard.release(KEY_F8);
        break;

      case(F9):
        Keyboard.press(KEY_F9);
        Keyboard.release(KEY_F9);
        break;

      case(F10):
        Keyboard.press(KEY_F10);
        Keyboard.release(KEY_F10);
        break;

      case(RIGHT):
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.release(KEY_RIGHT_ARROW);
        break;

      case(LEFT):
          Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.release(KEY_LEFT_ARROW);
          break;

      case(PG_UP):
          Keyboard.press(KEY_PAGE_UP);
          Keyboard.release(KEY_PAGE_UP);
          break;

      case(PG_DWN):
          Keyboard.press(KEY_PAGE_DOWN);
          Keyboard.release(KEY_PAGE_DOWN);
          break;

      case(HOME):
          Keyboard.press(KEY_HOME);
          Keyboard.release(KEY_HOME);
          break;

      case(END):
          Keyboard.press(KEY_END);
          Keyboard.release(KEY_END);
          break;

      case(ESC):
          Keyboard.press(KEY_ESC);
          Keyboard.release(KEY_ESC);
          break;

      case (SCUT0): send_shortcut(SCUT0); change_current_mode(MODE_NORMAL); break;
      case (SCUT1): send_shortcut(SCUT1); change_current_mode(MODE_NORMAL); break;
      case (SCUT2): send_shortcut(SCUT2); change_current_mode(MODE_NORMAL); break;
      case (SCUT3): send_shortcut(SCUT3); change_current_mode(MODE_NORMAL); break;
      case (SCUT4): send_shortcut(SCUT4); change_current_mode(MODE_NORMAL); break;
      case (SCUT5): send_shortcut(SCUT5); change_current_mode(MODE_NORMAL); break;
      case (SCUT6): send_shortcut(SCUT6); change_current_mode(MODE_NORMAL); break;
      case (SCUT7): send_shortcut(SCUT7); change_current_mode(MODE_NORMAL); break;
      case (SCUT8): send_shortcut(SCUT8); change_current_mode(MODE_NORMAL); break;
      case (SCUT9): send_shortcut(SCUT9); change_current_mode(MODE_NORMAL); break;
          
      default:
          break;
  }  
}
