
#include "SmartResponseXEmt.h"
#include <avr/sleep.h>

// Select the timers you're using, here ITimer1
#define USE_TIMER_1     true
#define USE_TIMER_2     true
#define USE_TIMER_3     false
#define USE_TIMER_4     false
#define USE_TIMER_5     false

#include "TimerInterrupt.h"

int row;
int col;
int scrollRow;
char buf[17][70];
char draw[5];
char incomingByte = 0; // for incoming serial data
void setup()
{
  Serial1.begin(9600); // opens serial port, sets data rate to 9600 bps

  pinMode(INT2, INPUT_PULLUP);
  TRXPR = 1 << SLPTR; // send transceiver to sleep

  SRXEInit(0xe7, 0xd6, 0xa2); // initialize display
  SRXEFill(0);
  SRXEScrollArea(0, 136, 24); // Visible screen area

  SRXEWriteString(0, 0, "Pocket terminal by Michele Trombetta", FONT_NORMAL, 3, 0);
  SRXEWriteString(0, 9, "      www.github.com/chmod775       ", FONT_NORMAL, 3, 0);

  row = 2;
  col = 0;
  scrollRow = 0;
  
  // Init timer ITimer1
  ITimer1.init();
  ITimer1.attachInterrupt(1000, KeyBoard_Handler);
  // Init timer ITimer2
  //ITimer2.init();
  //ITimer2.attachInterrupt(2, Cursor_Handler);
}

void loop() {}

/* ##### SERIAL MANAGEMENT ##### */
void serialEvent1() {
   if (Serial1.available() > 0) {
    incomingByte = Serial1.read();
    PutChar(incomingByte);
  } 
}

char Serial1_BlockingRead(){
  while(!Serial1.available());
  return Serial1.read();
}
/* ############################# */

/* ##### KEYBOARD + CURSOR MANAGEMENT ##### */
byte cursorVisible = 0;
byte typing = 0;
int CursorHandler_Cnt = 0;

void Cursor_Handler() {
  if (!typing) {
    cursorVisible = cursorVisible > 0 ? 0 : 1;
    DrawCursor(cursorVisible);
  }
  typing = false;
}

void KeyBoard_Handler() {
  int key = SRXEGetKey();
  if (key) {
    if ((key >= 0xe0) && (key <= 0xef)) {
      Serial1.write('\e');
      Serial1.write('[');
      switch (key) {
        case 0xe0:
          Serial1.write('A');
          break;
        case 0xe1:
          Serial1.write('B');
          break;
        case 0xe2:
          Serial1.write('C');
          break;
        case 0xe3:
          Serial1.write('D');
          break;
      }
    }
    else
      Serial1.write(key);
  }

  CursorHandler_Cnt++;
  if (CursorHandler_Cnt >= 500) {
    CursorHandler_Cnt = 0;
    Cursor_Handler();
  }
}

void DrawCursor(bool visible) {
  cursorVisible = visible;
  if (visible) {
    SRXEVerticalLine(6 * col, 8 * ((row + scrollRow) % 17), 8, 3);
  } else {
    int tRow = (row + scrollRow) % 17;
    SRXEVerticalLine(6 * col, 8 * tRow, 8, 0);
    SRXEWriteChar(6 * col, 8 * tRow, buf[tRow][col]);
  }
}
/* ############################################### */

/* ##### TERMINAL EMULATOR ##### */
void PutChar(char ch) {
  if (cursorVisible) DrawCursor(false);
  
  if (ch == '\n') { NewLine(); return; }
  if (ch == '\r') { CarriageReturn();  return; }
  if (ch == '\b') { Backspace();  return; }
  if (ch == '\t') { Tab();  return; }
  if (ch == '\e') { Escape();  return; }

  typing = true;
  
  int tRow = (row + scrollRow) % 17;

  buf[tRow][col] = ch;

  SRXEWriteChar(6 * col, 8 * tRow, ch);

  if (col < 70)
    col++;
}

void ScrollUp() {
  scrollRow++;
  if (scrollRow >= 17) scrollRow = 0;
  
  memset(buf[row], ' ', 70);
  buf[row][69] = 0x00;
  SRXEWriteString(0, 8 * ((row + scrollRow) % 17), buf[row], FONT_SMALL, 3, 0);
  SRXEScroll(8);
}

void NewLine() {
  if (row >= 16) {
    row = 16;
    ScrollUp();
  } else
    row++;
}

void Tab() {
  if (col < 70)
    col += 4;
}

void Backspace(){
  if (col > 0);
    col--;
}

void CarriageReturn() {
  col = 0;
}

void Clear() {
  col = 0;
  row = 0;
}

unsigned char c;
void Escape(){
  c = Serial1_BlockingRead();
  byte x,y,val;
  if(c == 'D'){
    NewLine();
  }
  if(c == 'M'){
    row--;
    if(row < 0){row = 0;}
  }
  if(c == 'E'){
    NewLine();
    CarriageReturn();
  }
  if(c == '['){
    c = Serial1_BlockingRead();
    
    if (c == '[')
      c = Serial1_BlockingRead();
    
    val = 255;
    if(isdigit(c)){
      val = c - '0';
      c = Serial1_BlockingRead();
      if(isdigit(c)){
        val*=10;
        val+=c-'0';
        c = Serial1_BlockingRead();
      }
    }
    switch(c){
      case ';':
        int val2;
        val2 = Serial1_BlockingRead() - '0';
        c = Serial1_BlockingRead();
        if(isdigit(c)){
          val2 *= 10;
          val2 += c-'0';
          c = Serial1_BlockingRead();
        }
        if(c == 'f' || c == 'H'){
          row = val-1; col = val2-1;
          if(col > 70){col = 70;}
          if(col < 0){col = 0;}
          if(row >= 16){row = 16;}
          if(row < 0){row = 0;}
        }
        break;
      case 'A':case 'a': row-=(val==255)?1:val; if(row < 0){row = 0;} break;
      case 'B':case 'b': row+=(val==255)?1:val; if(row >= 16){row = 16;} break;
      case 'C':case 'c': col+=(val==255)?1:val; if(col > 70){col = col - 70;} break;
      case 'D':case 'd': col-=(val==255)?1:val; if(col < 0){col = 0;} break;
      case 'H':col = 0; row =0 ; break;
      
      case 'K':
        if(val == 0 || val == 255){ // Cursor to end
          memset(buf[row], ' ', 70);
          buf[row][69] = 0x00;
          SRXEWriteString(6 * col, 8 * ((row + scrollRow) % 17), buf[row], FONT_SMALL, 3, 0);
        }
        if(val == 1){ // Cursor to begin
          memset(buf[row], ' ', col + 1);
          buf[row][col] = 0x00;
          SRXEWriteString(0, 8 * ((row + scrollRow) % 17), buf[row], FONT_SMALL, 3, 0);
        }
        if(val == 2){ // Entire line
          memset(buf[row], ' ', 70);
          buf[row][69] = 0x00;
          SRXEWriteString(0, 8 * ((row + scrollRow) % 17), buf[row], FONT_SMALL, 3, 0);
        }
        break;
      case 'J':
        if(val == 0 || val == 255){ // Cursor to end
          memset(buf[row], ' ', 70);
          buf[row][69] = 0x00;
          SRXEWriteString(6 * col, 8 * ((row + scrollRow) % 17), buf[row], FONT_SMALL, 3, 0);
          if (row > 16) {
            for (int r = row + 1; r < 17; r++) {
              memset(buf[r], ' ', 70);
              buf[r][69] = 0x00;
              SRXEWriteString(0, 8 * ((r + scrollRow) % 17), buf[r], FONT_SMALL, 3, 0);
            }
          }
        }
        if(val == 1){ // Cursor to begin
          memset(buf[row], ' ', col + 1);
          buf[row][col] = 0x00;
          SRXEWriteString(0, 8 * ((row + scrollRow) % 17), buf[row], FONT_SMALL, 3, 0);
          if (row > 0) {
            for (int r = row - 1; r > 0; r--) {
              memset(buf[r], ' ', 70);
              buf[r][69] = 0x00;
              SRXEWriteString(0, 8 * ((r + scrollRow) % 17), buf[r], FONT_SMALL, 3, 0);
            }       
          }
        }
        if(val == 2){ // Entire screen
          SRXEFill(0);
        }
        break;
      case 'n':/*
      if(val == 5){
        s.write(27);
        s.write("[0n");
      }
      if(val == 6){
        s.write(27);
        s.write("[");
        s.print((int)cx);
        s.write(";");
        s.print((int)cy);
        s.write("n");
      }*/
      break;
      case 'm':/*
        if(val == 0 || val == 255){
          cur_atr = 0;
        }
        if(val == 4){
          cur_atr |= UNDERLINE;
        }
        #ifdef ENABLE_INVERSE
        if(val == 7){
          cur_atr |= INVERSE;
        }
        #endif
        if(val == 5){
          cur_atr |= BLINK;
        }*/
        break;
      case 'Z':/*
        s.write(27);
        s.write("[?1;0c");*/
        break;
        
    }
  }
}
