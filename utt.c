/***
  Project main function template for MicroZed based MZ_APO board
  designed by Petr Porazil at PiKRON

  change_me.c      - main file

  Ultimate tic-tac-toe
  Semestral project - Computer architecture CVUT
  Fabian Pascual
  Oluwaseun Olasoji
  22/05/2022
	
 ***/

#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "font_types.h"

#define LCD_WIDTH 480
#define LCD_HEIGH 320

unsigned char *spiled_reg_base;
unsigned char *parlcd_reg_base;

//INTERFACE.H------------------------------------------------------------------------------------------------------------------------------



union led {
        struct {
                uint8_t b,g,r;
        };
        uint32_t data;
};

union pixel {
        struct {
                unsigned b : 5;
                unsigned g : 6;
                unsigned r : 5;
        };
        uint16_t d;
};

union pixel buffer[LCD_WIDTH][LCD_HEIGH];

unsigned char *print(){
	parlcd_reg_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
		parlcd_write_cmd(parlcd_reg_base, 0x2c); // reset location to the top left corner
		for (unsigned y = 0; y < LCD_HEIGH; y++)
			for (unsigned x = 0; x < LCD_WIDTH; x++)
				parlcd_write_data(parlcd_reg_base, buffer[x][y].d);
	return parlcd_reg_base;
}

void util_init(){
	spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
	if (spiled_reg_base == NULL) exit(1);

	}
int draw_timer(){
	
	volatile uint32_t time = 0x00000001;
	*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_LINE_o) = 0x00000000;
	for(int i=0; i < 32;i++){ 	
		*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_LINE_o) = time;
		sleep(1);
		time = time * 2 + 1;
		if(i > 23){
		*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_LINE_o) = 0x00000000;
		sleep(1);
		}
	}	
	return 1; 	
}

int get_knobs_bound(int kn){
	if(( 0 <= kn && kn < 9) || (81 <= kn && kn < 90) || (163 <= kn && kn < 172))
		return 0;
	if(( 9 <= kn && kn < 18)|| (90 <= kn && kn < 100)||(172 <= kn && kn < 181) )
		return 1;
	if((18 <= kn && kn < 27)||(100 <= kn && kn < 109)||(181 <= kn && kn < 199 ))
		return 2;
	if((27 <= kn && kn < 36)||(109 <= kn && kn < 118)||(199 <= kn && kn < 218))
		return 3;
	if((36 <= kn && kn < 45)||(118 <= kn && kn < 127)||(218 <= kn && kn < 227))
		return 4;
	if((45 <= kn && kn < 54)||(127 <= kn && kn < 136)||(227 <= kn && kn < 236))
		return 5;
	if((54 <= kn && kn < 63)||(136 <= kn && kn < 145)||(236 <= kn && kn < 242))
		return 6;
	if((63 <= kn && kn < 72)||(145 <= kn && kn < 154)||(242 <= kn && kn < 248))
		return 7;
	if((72 <= kn && kn < 81)||(154 <= kn && kn < 163)||(248 <= kn && kn < 255))
		return 8;
	return -1;
}

int get_row(int row){
	if (row == 110)
		return 0;
	if (row == 140)
		return 1;
	if (row == 170)
		return 2;
	if (row == 210)
		return 3;
	if (row == 240)
		return 4;
	if (row == 270)
		return 5;
	if (row == 310)
		return 6;
	if (row == 340)
		return 7;
	if (row == 370)
		return 8;
	return -1;
	}

int get_col(int col){
	if (col == 30)
		return 0;
	if (col == 60)
		return 1;
	if (col == 90)
		return 2;
	if (col == 130)
		return 3;
	if (col == 160)
		return 4;
	if (col == 190)
		return 5;
	if (col == 230)
		return 6;
	if (col == 260)
		return 7;
	if (col == 290)
		return 8;
	return -1;
	}

int pchar(font_descriptor_t *font, char c, unsigned x, unsigned y){
  int ch_w = font->maxwidth;
  if(c < font->firstchar) return 0;
  c-= font->firstchar;
  if(c>=font->size) return 0;
  if(font->width)
  	ch_w = font-> width[(int)c];
	
  for (unsigned w = 0; w < ch_w; w++)
    for (unsigned h = 0; h < font->height; h++)
      if (font->bits[c * font->height + h] & (1 << (16 - w)))
        buffer[x + w][y + h].d = 0x0000;
  return ch_w;      
 }

int draw_proportional(font_descriptor_t *font, char c, unsigned x, unsigned y, int proportion){
  int ch_w = font->maxwidth;
  if(c < font->firstchar) return 0;
  c-= font->firstchar;
  if(c>=font->size) return 0;
  if(font->width)
	ch_w = font-> width[(int)c];
  
  for (unsigned w = 0; w < ch_w+1; w++)
    for (unsigned h = 0; h < font->height; h++)
		for(unsigned i = 0; i <= proportion; i++)
			for(unsigned j = 0; j <= proportion; j++){
				if (font->bits[c * font->height + h] & (1 << (16 - w)))
				//	for(unsigned k = proportion; k >= 0 ; k--)
					buffer[x + (w*proportion+i)][y + (h*proportion+j)].d = 0x0000;
		}
	return ch_w * proportion; 
 
}

void draw_step(int turn,font_descriptor_t *font){
	//volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
	volatile uint32_t current_turn = 0x00000000;
	*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_LINE_o) = current_turn + turn;	
	for (int x = 0; x <= 90; x++){
		for(int y = 10; y <= 60; y++){			
			buffer[x][y].d = 0xFFFF;
		}
	}	
	char unit[10] = "0123456789";
	draw_proportional(font, unit[(turn/10)] ,30,15,2);
	draw_proportional(font, unit[(turn%10)] ,55,15,2);
	print();

}

void draw_game(int h, int w){
	for (unsigned i = 0; i < 90; i++) {
		for(unsigned j=0; j<3;j++){	
			buffer[w-14+j][h + 45-i].d = 0x0000;// left	
			buffer[w+14+j][h + 45-i].d = 0x0000;// right	
			buffer[w -45+i][h+14+j].d = 0x0000;// top		
			buffer[w -45+i][h-14+j].d = 0x0000;// bottom	  
		}
	}
}

void draw_board(){
	
	for (unsigned x = 0; x < LCD_WIDTH; x++){
		for (unsigned y = 0; y < LCD_HEIGH; y++){
			buffer[x][y].d = 0xffff;
		}
	}
	int i=0;
	for(int x = 140; x<=340; x += 100){
		i++;
		int j = 0;
		for(int y = 60;  y<=260;y += 100){
			j++;
			draw_game(y,x);}
		}
		
}

void draw_player(int w, int h, bool player){ 
	for (int x = w - 7; x <= w + 9; x++){
		for(int y = h - 7; y <= h + 9; y++){
			if (player==1){ // 1 to red and 0 to blue
			buffer[x][y]= (union pixel){.r = 0x1f};
		}else{              
			buffer[x][y]= (union pixel){.b = 0x1f};
			}
		}
	}
}

void draw_result(int w, int h, bool player){
	for (int x = w - 45; x <= w + 45; x++){
		for(int y = h - 45; y <= h + 45; y++){
			if (player==1){
			buffer[x][y]= (union pixel){.r = 0x1f};
		}else{
			buffer[x][y]= (union pixel){.b = 0x1f};
			}
		}
	}
	print();
}

void draw_cursor(int w, int h, bool type){ //(position, bool player)
	if(type){// 1 big game
		for (unsigned i = 0; i < 103; i++) {
			for(unsigned j=0; j<4;j++){	
				buffer[w -52 +j][h + 51-i].d = 0x8f88;//left
				buffer[w +48 +j][h + 51-i].d = 0x8f88;//rigth
				buffer[w -51 +i][h + 49+j].d = 0x8f88;;//bottom
				buffer[w -51 +i][h - 51+j].d = 0x8f88;;//top
			}
		}	
	}else{	//0 samll game
		for (unsigned i = 0; i < 22; i++) {
		  for(unsigned j=0; j<2;j++){	
			buffer[w -10 +j][h + 11-i].d = 0x8f88;//left
			buffer[w +11 +j][h + 11-i].d = 0x8f88;//rigth
			buffer[w -10 +i][h + 11+j].d = 0x8f88;;//bottom
			buffer[w -10 +i][h - 10+j].d = 0x8f88;;//top
			}
		}									
	}
	print();
}

void draw_title(){
	char tictac[11] = "TIC TAC TOE";
	char ultimate[8] = "ULTIMATE";
	
	for(int i=0, j=12;i<11; i++, j+= 26){
	  draw_proportional(&font_winFreeSystem14x16, tictac[i] ,420,j,2);
	}
	for(int m=0, n=62;m<8; m++, n+= 30){
	  draw_proportional(&font_winFreeSystem14x16, ultimate[m] ,40,n,2);
	}

}

void draw_turn(bool player){
	if(player){
		//volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);{
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_RGB1_o) = 0xFFFF0000;
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_RGB2_o) = 0xFFFF0000;
		
		for (unsigned x = 390; x < 470 ; x++){ // refresh the oposite side
			for (unsigned y = 10; y < 310; y++){
					buffer[x][y].d = 0xffff;
				}
			}		
		for (unsigned x = 10; x < 90 ; x++){   //paint it red
			for (unsigned y = 60; y < 310; y++){
				buffer[x][y].d = 0xf000;
			}
		}
	}
	else{
		//volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);{
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_RGB1_o) = 0x0000FFFF;
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_RGB2_o) = 0x0000FFFF;
		
		for (unsigned x = 10; x < 90 ; x++){ // refresh the oposite side
			for (unsigned y = 60; y < 310; y++){
				buffer[x][y].d = 0xffff;
			}
		}					
		for (unsigned x = 390; x < 470 ; x++){//paint it blue
			for (unsigned y = 10; y < 310; y++){
				buffer[x][y].d = 0x00ff;
			}
		}
		}	
	draw_title();
}

void draw_tie(){
	for (unsigned x = 0; x < LCD_WIDTH; x++){
		for (unsigned y = 0; y < LCD_HEIGH; y++){
			buffer[x][y].d = 0xffff;
		}  
	}
	char text[4] = "Tie!";
	int x=0;
	for(int i=0, j=30 ;i<4; i++, j+= x){
		x = draw_proportional(&font_winFreeSystem14x16, text[i],140+j,120,5);
	}
}

void draw_winner(bool player){
	for (unsigned x = 0; x < LCD_WIDTH; x++){
		for (unsigned y = 0; y < LCD_HEIGH; y++){
			buffer[x][y].d = 0xffff;
		}  
	}
	if(player){
		//volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);{
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_RGB1_o) = 0xFFFF0000;
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_RGB2_o) = 0xFFFF0000;
						
		for (unsigned x = 10; x < 470 ; x++){   //paint it red
			for (unsigned y = 10; y < 310; y++){
				buffer[x][y].d = 0xf000;
			}
		}
	}else{
		//volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);{
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_RGB1_o) = 0x0000FFFF;
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_RGB2_o) = 0x0000FFFF;
							
		for (unsigned x = 390; x < 470 ; x++){//paint it blue
			for (unsigned y = 10; y < 310; y++){
				buffer[x][y].d = 0x00ff;
			}
		}
	}
	
	char text[7] = "Winner!";
	int x=0;
	for(int i=0, j=30 ;i<7; i++, j+= x){
		x = draw_proportional(&font_winFreeSystem14x16, text[i],80+j,120,5);
	}print();
	for(int i=0; i<=10; i++){
		volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);{
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_LINE_o) = 0xFFFFFFFF;
			sleep(1);
			*(volatile uint32_t*)(spiled_reg_base + SPILED_REG_LED_LINE_o) = 0x00000000;
			sleep(1);		
		}
	}
	//draw_board();
}

void refresh(){ 	
	for(int x = 140; x<=340; x += 100){
		for(int y = 60;  y<=260;y += 100){			
			for (unsigned i = 0; i < 103; i++){
				for(unsigned j=0; j<6;j++){	
					buffer[x -52 +j][y + 51-i].d = 0xffff;//left
					buffer[x +47 +j][y + 51-i].d = 0xffff;//rigth
					buffer[x -51 +i][y + 48+j].d = 0xffff;;//bottom
					buffer[x -51 +i][y - 52+j].d = 0xffff;;//top
						for(int w = x-30; w<=x+30; w += 30){//0 small game
							for(int h = y-30;  h<=y+30;h += 30){	
							  for (unsigned i = 0; i < 22; i++){
								  for(unsigned j=0; j<2;j++){	
									buffer[w -10 +j][h + 11-i].d = 0xffff;//left
									buffer[w +11 +j][h + 11-i].d = 0xffff;//rigth
									buffer[w -10 +i][h + 11+j].d = 0xffff;//bottom
									buffer[w -10 +i][h - 10+j].d = 0xffff;//top
	}	}	}	}	}	}	}	}					
	
}


//GAME.H------------------------------------------------------------------------------------------------------------------------------

#define ROWS 9
#define COLS 9

extern font_descriptor_t font_rom8x16;
extern font_descriptor_t font_winFreeSystem14x16;

int Board_Position[9][2];
typedef char Board[ROWS][COLS];
typedef char MetaBoard[ROWS / 3][COLS / 3];
typedef struct{
	int x;
	int y;
	}Position;
	
void fillSubBoard(Board board, int x, int y, char c)
{
    for (; (x % 3) != 0; x--); // quickly set x to left bound of sub-board
    for (; (y % 3) != 0; y--); // quickly set y to upper bound of sub-board
    for (int rowMax = x + 2, row = x; row <= rowMax; row++)
    {
        for (int columnMax = y + 2, column = y; column <= columnMax; column++)
        {
            board[row][column] = c;
        }
    }
}

int getRowBound(int row)
{
    switch (row)
    {
        case 0 ... 2:
            return 0;
        case 3 ... 5:
            return 1;
        case 6 ... 8:
            return 2;
        default:
            return -1;
    }
}

int getColumnBound(int column)
{
    switch (column)
    {
        case 0 ... 2:
            return 0;
        case 3 ... 5:
            return 1;
        case 6 ... 8:
            return 2;
        default:
            return -1;
    }
}

static int checkMeta(MetaBoard meta)
{
    const int xStart[ROWS - 1] = {0,  0,  0,  0,  1,  2,  0,  0};
    const int yStart[COLS - 1] = {0,  1,  2,  0,  0,  0,  0,  2};
    const int xDelta[ROWS - 1] = {1,  1,  1,  0,  0,  0,  1,  1};
    const int yDelta[COLS - 1] = {0,  0,  0,  1,  1,  1,  1,  -1};
    static int startx, starty, deltax, deltay;
    for (int trip = 0; trip < ROWS - 1; trip++)
    {
        startx = xStart[trip];
        starty = yStart[trip];
        deltax = xDelta[trip];
        deltay = yDelta[trip];
        // main logic to check if a subboard has a winner
        if (meta[startx][starty] != '-' &&
            meta[startx][starty] == meta[startx + deltax][starty + deltay] &&
            meta[startx][starty] == meta[startx + deltax + deltax][starty + deltay + deltay]) return 1;
    }
    return 0;
}
static int checkBoard(Board board, MetaBoard meta, int player, int row, int column)
{
    const int xStart[ROWS - 1] = {0,  0,  0,  0,  1,  2,  0,  2};
    const int yStart[COLS - 1] = {0,  1,  2,  0,  0,  0,  0,  0};
    const int xDelta[ROWS - 1] = {1,  1,  1,  0,  0,  0,  1,  -1};
    const int yDelta[COLS - 1] = {0,  0,  0,  1,  1,  1,  1,  1};
    int startx, starty, deltax, deltay, status = 0;
    row = row/3*3;
    column = column/3*3;
    for (int trip = 0; trip < ROWS - 1; trip++)
    {
        startx = row + xStart[trip];
        starty = column + yStart[trip];
        deltax = xDelta[trip];
        deltay = yDelta[trip];
        if (board[startx][starty] != '-' &&
            board[startx][starty] == board[startx + deltax][starty + deltay] &&
            board[startx][starty] == board[startx + deltax + deltax][starty + deltay + deltay])
        {	
            fillSubBoard(board, row, column, (player == 1) ? 'R' : 'B');
            draw_result(Board_Position[(row/3)*3+(column/3)][0],Board_Position[(row/3)*3+(column/3)][1],player);// draw the result
            meta[getRowBound(row)][getColumnBound(column)] = (player == 1) ? 'R' : 'B';
            status = 1;
        } 
    }
    return (status + checkMeta(meta)); // always check if the game has a winner
}

void printBoard(Board board)
{
    printf("\n=============||===========||=============\n");
    for (int row = 0; row < ROWS; row++)
    {
        printf("||");
        for (int column = 0; column < COLS; column++)
        {
            if (board[row][column] == '-') printf("%d,%d|", row, column);
            else printf(" %c |", board[row][column]);
            if (0 == (column+1) % 3) printf("|");
        }
        if ((row+1) % 3 == 0) printf("\n=============||===========||=============\n");
        else printf("\n-----|---|---||---|---|---||---|---|-----\n");
    }
}
Position get_meta_position(int knob_bound, int x, int y){
	Position position;
		if (knob_bound == 0){
			position.x = x-30;
			position.y = y-30;
		}
		if (knob_bound == 1){
			position.x = x;
			position.y = y-30;
		}
		if (knob_bound == 2){
			position.x = x+30;
			position.y = y-30;
			}
		if (knob_bound ==3){
			position.x = x-30;
			position.y = y;
			}
		if (knob_bound ==4){
			position.x = x;
			position.y = y;
			}
		if (knob_bound ==5){
			position.x = x+30;
			position.y = y;
			}
		if (knob_bound ==6){
			position.x = x-30;
			position.y = y+30;
			}
		if (knob_bound ==7){
			position.x = x;
			position.y = y+30;
			}
		if (knob_bound ==8){
			position.x = x+30;
			position.y = y+30;
			}		
	return position;
}

int cursor(bool player, bool type, int x, int y){ 
	uint32_t rgb_knobs_value;
	//unsigned char *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);	
	int rk=0, bk=0, rb, bb,cu,cb;
	draw_turn(player);
	do{		
		rgb_knobs_value = *(volatile uint32_t*)(spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
		rk = (rgb_knobs_value>>16) & 0xFF;
		rb = (rgb_knobs_value>>26) & 1;
		bk = (rgb_knobs_value>>0) & 0xFF;
		bb = (rgb_knobs_value>>24) & 1;		
		if(player) cu = rk,cb= rb; 
			else cu = bk, cb = bb;
		if (cb != 0)break;
		int option = get_knobs_bound(cu);
		Position meta;		
		refresh();
		
		if(option == 0){ // position 1
			if (type){
				x = 141;
				y = 61;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		if(option == 1){ // position 2
			if (type){
				x=240;
				y=61;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		if(option == 2){ // position 3
			if (type){
				x=339;
				y=61;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		if(option == 3){ // position 4
			if (type){
				x=141;
				y=160;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		if(option  == 4){ // position 5
			if (type){
				x=240;
				y=160;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		if(option  == 5){ // position 6	
			if (type){
				x=339;
				y=160;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		if(option == 6){ // position 7			
			if (type){
				x=141;
				y=259;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		if(option  == 7){ // position 8	
			if (type){
				x=240;
				y=259;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		if(option == 8){ // position 9
			if (type){
				x=339;
				y=259;
				draw_cursor(x,y,type);//position 1 big game
			}else{
				meta = get_meta_position(option,x, y);
				draw_cursor( meta.x, meta.y, type);//position 1 small game
			}			
		}
		sleep(1);
	}while(1);
	//sleep(1);
	return cu;	
}

int verify(Board board, MetaBoard meta, int x, int y, int player){
	int r = get_row(x); int c = get_col(y);
	if (board[c][r] == '-'){
		printf("yes\n");
		int r1 = getRowBound(r); int c1 =getColumnBound(c);
		if (meta[c1][r1] == '-'){
			printf("yes meta\n");
			board[c][r] = (player == 1) ? 'R' : 'B';
			draw_player(x, y, player);
			return 1;}
		else{return 0;}
		
		}
	else{return 0;}
}

//MAIN.c------------------------------------------------------------------------------------------------------------------------------


int main(int argc, char *argv[]){
	bool winner = 0;
	int startx = 240, starty =160, verified = 0,row, column;
	Board board;
	MetaBoard meta;
	util_init();
	// initialize boards and fill with '-'
	for (int i = 0; i<ROWS; i++){
		for (int j= 0; j<COLS; j++){
			board[i][j] = '-';
			if (i < 3 && j < 3){
				meta[i][j] = '-';	
	}	}	}
	draw_board();// initialize the board
	int i = 0;
	// draws the board
	for(int x = 140; x<=340; x += 100){
		i++;
		int j = 0;
		for(int y = 60;  y<=260;y += 100){
			j++;
			int k = (j-1)*3+(i-1);
			Board_Position[k][0] = x;
			Board_Position[k][1] = y;
	}	}	

	int player = 0; // player blue start	
	int selected = cursor(player, 1, startx, starty);
	
	for (i = 0;i < ROWS*COLS || !winner;i++){ //------------------------------ Runs the game		
		draw_step(i, &font_rom8x16);		 
		if(i%2==0) player = 0; // Detects who's turn
		else player = 1;       // 1 to red and 0 to blue
		Position point;
		int next;
		next = get_knobs_bound(selected);
		do{
			selected= cursor(player,0,Board_Position[next][0],Board_Position[next][1]);
			point = get_meta_position(get_knobs_bound(selected), Board_Position[next][0],Board_Position[next][1]);
			verified = verify(board,meta, point.x, point.y, player);// verify if the choosen position is legal
			if (verified == 1)break;
		}while(1); 
						
		row = point.x;
		column = point.y;
		row = get_row(row); column = get_col(column);
		int check = checkBoard(board, meta, player, column, row);
		printBoard(board);
		if(check == 1){
			//next move can be anywhere
			int big_check = get_knobs_bound(selected);
			if (meta[big_check/3][big_check%3] != '-'){
				selected = cursor(!player, 1, Board_Position[get_knobs_bound(selected)][0],Board_Position[get_knobs_bound(selected)][1]);
			}
			else{continue;}
		}else if(check == 2){
			winner = player;
			break;
		}else{
			int big_check = get_knobs_bound(selected);
			if (meta[big_check/3][big_check%3] != '-'){
				selected = cursor(!player, 1, Board_Position[get_knobs_bound(selected)][0],Board_Position[get_knobs_bound(selected)][1]);
			}
		}
	}
	if (winner){
		draw_winner(player);
	}else{
		draw_tie();
		draw_timer();		
	} 
  return 0;
}
	
