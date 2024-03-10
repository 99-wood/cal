#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

typedef Adafruit_SSD1306 Screen;
#define BLACK SSD1306_BLACK
#define WHITE SSD1306_WHITE
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

const int W = 128, H = 64;
int len = 21;
char str[] = "12345678901234567890";
int pos;
const int N = 2, M = 9;
const int X = 0, Y = 40;
const int L = 12;
int locx = 0, locy = 0;
char mapp[N][M] = {
  {'1', '2', '3', '4', '5', '6', '7', '8', '9'},
  {'0', '+', '-', '*', '/', '(', ')', '=', 'C'},
};
#define LEFT 2
#define RIGHT 3
#define UP 4
#define DOWN 5
#define YES 6
int priority(char c){					//运算符优先级定义 
	switch (c){
		case '(':return 0;break;
		case '+':return 1;break;
		case '-':return 1;break;
		case '*':return 2;break;
		case '/':return 2;break;
		case '^':return 3;break;
	}
}
void pop(long * numStack, int &top, char * charStack, int &ctop){							//出栈运算 
	switch (charStack[ctop]){
		case '+':--top;numStack[top]=numStack[top]+numStack[top+1];break;
		case '-':
			if(top > 0){
				--top;
				numStack[top] = numStack[top] - numStack[top + 1];
				break;
			}
			else{
				numStack[top] = -numStack[top];
				break;
			}
		case '*':--top;numStack[top]=numStack[top]*numStack[top+1];break;
		case '/':--top;numStack[top]=numStack[top]/numStack[top+1];break;
		case '^':--top;numStack[top]=pow(numStack[top],numStack[top+1]);break;
	}
  // Serial.print(numStack[0]);
  // Serial.print(" ");
  // Serial.print(numStack[1]);
  // Serial.print(" ");
  // Serial.print(numStack[2]);
  // Serial.println("");
	return;
}
long trans(char *s, int len, int &p){			//字符串转数字 
	int j=0;
  long ans = 0;
	for(;p<len&&s[p]>='0'&&s[p]<='9';++p){
    ans = (ans * 10) + (s[p] - '0');
	}
  --p;
	return ans;
}//atoi
long calc(char *s, int len){
	long * numStack = new long[len]; int top=-1;
  memset(numStack, 0, sizeof(numStack));
	char * charStack = new char[len]; int ctop=-1;
  memset(charStack, 0, sizeof(charStack));
	for(int i=0;i<len;++i){
		if(s[i]<='9'&&s[i]>='0'){		//入数据栈 
			++top;
			numStack[top]=trans(s, len, i);
		}
		else {
			char temp=s[i];
			if(ctop==-1||temp=='('){	//入符号栈 
				++ctop;
				charStack[ctop]=temp;
			}
			else if(temp==')'){			//计算括号内 
				while(charStack[ctop]!='('){
					pop(numStack, top, charStack, ctop);
					--ctop;
				}
				ctop--;
			}
			else{						//按优先级运算 
				while(priority(temp)<=priority(charStack[ctop])){
					pop(numStack, top, charStack, ctop);
					--ctop;
				}
				++ctop;
				charStack[ctop]=temp;
			}
		}
	}
	for(;ctop>=0;--ctop){			//处理剩余数据 
		pop(numStack, top, charStack, ctop);
	}
	long ans = numStack[top];
	delete[] numStack;
	delete[] charStack;
	return ans;
}
void makeButton(){
  display.fillRect(X + locx * L, Y + locy * L, L, L, BLACK);
  display.drawRect(X + locx * L, Y + locy * L, L, L, WHITE);
  display.drawChar(X + locx * L + (L - 7) / 2 + 1, Y + locy * L + (L - 10) / 2 + 1, mapp[locy][locx], WHITE, BLACK, 1);
  display.display();
  return;
}
void selectButton(){
  display.fillRect(X + locx * L, Y + locy * L, L, L, WHITE);
  return;
}
void printStr(){
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.setTextColor(WHITE, BLACK);
  display.print(str);
  display.display();
  return;
}
void deselect(){
  makeButton();
}
void select(){
  selectButton();
}
void click(){
  if(mapp[locy][locx] == '='){
    long ans = calc(str, pos);
    for(int i = 0; i < len; ++i){
      str[i] = ' ';
    }
    ltoa(ans, str, 10);
    pos = len - 1;
    for(int i = 0; i < len; ++i) if(str[i] == '\0') str[i] = ' ';
    while(str[pos] == ' ') --pos;
    ++pos;
    str[pos] = '_';
  }
  else if(mapp[locy][locx] == 'C'){
    if(pos){
      str[pos] = ' ';
      str[--pos] = '_';
    }
  }
  else{
    if(pos < len - 1){
      str[pos] = mapp[locy][locx];
      str[++pos] = '_';
    }
  }
  printStr();
  display.display();
}
void setup() {
  Serial.begin(9600);
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(YES, INPUT);
  len = strlen(str);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  for(int i = 0; i < N; ++i){
    for(int j = 0; j < M; ++j){
      locx = j;
      locy = i;
      makeButton();
    }
  }
  selectButton();
  printStr();
  display.display();
  for(int i = 0; i < len; ++i) str[i] = ' ';
  pos = 0;
  str[pos] = '_';
  delay(500);
  printStr();
}
void loop() {
  // Serial.println("OK");
  if(digitalRead(LEFT) == LOW){
    Serial.println("LEFT");
    if(locx > 0){
      deselect();
      locx--;
      select();
    }
  }
  if(digitalRead(RIGHT) == LOW){
    Serial.println("RIGHT");
    if(locx < M - 1){
      deselect();
      locx++;
      select();
    }
  }
  if(digitalRead(UP) == LOW){
    Serial.println("UP");
    if(locy > 0){
      deselect();
      locy--;
      select();
    }
  }
  if(digitalRead(DOWN) == LOW){
    Serial.println("DOWN");
    if(locy < N - 1){
      deselect();
      locy++;
      select();
    }
  }
  if(digitalRead(YES) == LOW){
    Serial.println("YES");
    click();
  }
  // Serial.println("NO");
  display.display();
  select();
  delay(100);
}