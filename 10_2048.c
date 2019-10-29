#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#define _white 0xeeffffff
#define _blank 0x00ffffff
#define VALUE_DELAY 40000000
int matrix[4][4] = {
//	2, 2, 8, 16,
//	2, 2, 32, 4,
//	8, 4, 2, 8, 
//	4, 2, 4, 16,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
};
int lock[4][4] = {
	0,0,0,0,
	0,0,0,0,
	0,0,0,0, 
	0,0,1,0,
};//m

//枚举类型 表示手指滑动方向
enum finger_move{
	MOVE_LEFT = 1,
	MOVE_RIGHT,
	MOVE_UP,
	MOVE_DOWN,
};
enum finger_exist{ //是否存在点击 
	NO_EXIST = 0,
	EXIST
};
int x_start = 10,y_start = 325;
int xx_start = 10;
int yy_start = 330;
int way;
int start,end;
int in_move_flag ,judge_move_flag;
int lock_flag = 0;
int *plcd = NULL;
int ro[16]={0};
int co[16]={0};
int num[10]={0};//分数 
int cnt = 0;

char *bmp_num[] = {
	"_0.bmp",
	"_1.bmp","_2.bmp","_3.bmp","_4.bmp",
	"_5.bmp","_6.bmp","_7.bmp","_8.bmp",
	"_9.bmp","_10.bmp",
	"_s.cmp",
	"one.bmp",
	"two.bmp",
	"bgm.bmp",
	NULL,
};
char *bmp_name[] = {
	NULL,
	"1_1.bmp","1_2.bmp","1_3.bmp","1_4.bmp",
	"1_5.bmp","1_6.bmp","1_7.bmp","1_8.bmp",
	"1_9.bmp","1_10.bmp" ,"1_11.bmp",
//	"1_12.bmp","1_13.bmp","1_14.bmp",
//	"1_15.bmp","1_16.bmp" ,
	"1_gameover.bmp",
	"1_youwin",
	"2048_1.bmp",
	NULL,
};
char *bmp_name1_1[] = {//灰色 
	NULL,
	"5-1.bmp","5-2.bmp","5-3.bmp","5-4.bmp",
	"5-5.bmp","5-6.bmp","5-7.bmp","5-8.bmp",
	"5-9.bmp","5-10.bmp" ,"5-11.bmp",
	NULL,
};
char *bmp_name2[] = {
	NULL,
	"2_1.bmp","2_2.bmp","2_3.bmp","2_4.bmp",
	"2_5.bmp","2_6.bmp","2_7.bmp","2_8.bmp",
	"2_9.bmp","2_10.bmp" ,"2_11.bmp",
//	"2_12.bmp","2_13.bmp","2_14.bmp",
//	"2_15.bmp","2_16.bmp" ,
	"2_gameover.bmp",
	"2_youwin",
	"2048_2.bmp",
	NULL,
};
char *bmp_name2_2[] = {//灰色 
	NULL,
	"4-1.bmp","4-2.bmp","4-3.bmp","4-4.bmp",
	"4-5.bmp","4-6.bmp","4-7.bmp","4-8.bmp",
	"4-9.bmp","4-10.bmp" ,"4-11.bmp",
	NULL,
};
void game_way(int way); //选择模式 
char *get_file_name_by_digit(int way,int num);
char *get_file_name_by_digit_lock(int way,int num);
void lcd_draw_point(int x, int y, int color);
void lcd_draw_rect(int x0, int y0, int w, int h, int color);
void lcd_draw_bmp(const char *file_name,int cc,int rr, int x0, int y0);
void lcd_draw_matrix(int way,int x0, int y0, int num, int pice_size, int blank,int color);
int get_finger_move_direction();
int get_finger_move_direction1(int x_min,int y_min,int x_max,int y_max);
void swap(int *a,int *b);
void move_left();
void move_right();
void move_up();
void move_down(); 
void delay();
int search_blank();
void srand_num(int cnt_blank);
void srand_lock(int cnt_blank);
void intl();
int judge();
void count(int cnt);//计分 

void print();

 
int main(int argc, char *argv[]){
	int fd; //framebuffer
	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0){
		perror("open file error: ");
		return 0;
	}
	plcd = mmap(NULL, 800*480*4,
						PROT_READ | PROT_WRITE,
						MAP_SHARED,
						fd,
						0);	
	if (plcd == MAP_FAILED){
		perror("mmap error :");
		return 0;
	}
	lcd_draw_bmp(bmp_num[14], 800, 480, 0, 0 );//(100,50) 
	while(1){ //两种模式 
//		lcd_draw_rect(0, 0, 800, 480, _blank);
		lcd_draw_bmp(bmp_num[12], 100, 100, 100, 50 );//(100,50) 
		lcd_draw_bmp(bmp_num[13], 100, 100, 100, 250 );//(100,250) 
		int move_1 = get_finger_move_direction1(100, 50, 200, 150);
		int move_2 = get_finger_move_direction1(100, 250, 200, 350); //(100,250) 
		if(move_1){
			way = 1;
			game_way(way);
		}else if(move_2){
			way = 2;
			game_way(way);
			
		}else{		
			printf("again\n");
		}
	}
	munmap(plcd, 800*480*4);//关闭lcd设备
	close(fd);
	return 0;
}


void game_way(int way){
	cnt = 0;
	memset(matrix,0,sizeof(matrix));
	memset(lock,0,sizeof(lock));      //m
	memset(ro,0,sizeof(ro));
	memset(co,0,sizeof(co));	
	memset(num,0,sizeof(num));
	srand_num( search_blank());
	srand_num( search_blank());
	
	if(way==1) lcd_draw_bmp(bmp_name[14],800,480,0,0);
	else lcd_draw_bmp(bmp_name2[14],800,480,0,0);
	lcd_draw_matrix(way, yy_start, xx_start, 4, 100, 20, _white);//(10,50)
	srand_num( search_blank());
	srand_num( search_blank());
	int move;
	int rand_lock = 0;//随机产生lock格 个数 
	while (1){
		if(rand_lock && (rand_lock%10==0 ) ){//每十步出现一个lock 
			if(search_blank())srand_lock( search_blank());
		}
		rand_lock++;
		move = get_finger_move_direction();
		lock_flag = in_move_flag = judge_move_flag = 0;      //m
		switch (move){
			case MOVE_LEFT:   move_left(); in_move_flag = 1; count(cnt);  break;
			case MOVE_RIGHT:  move_right(); in_move_flag = 1;count(cnt);  break;
			case MOVE_DOWN:   move_down(); in_move_flag = 1;count(cnt);  break;
			case MOVE_UP:     move_up(); in_move_flag = 1; count(cnt);  break;
			default:break;
			
		}	
		if(in_move_flag){ //随机数 
			lcd_draw_matrix(way, yy_start, xx_start, 4, 100, 20, _white);
			if(judge_move_flag)
				if(search_blank()) {
					srand_num( search_blank());
				}
		}
		int flag = judge();
		if(!flag && !search_blank()){
			if(flag==1){
				printf("GameOver!\n");
				if(way==1) lcd_draw_bmp(bmp_name[12],150,300,500,100);//起始坐标(225,150)
				else if(way==2) lcd_draw_bmp(bmp_name2[12],150,300,500,100);//起始坐标(225,150)
			}else{
				printf("YouWin!\n");
				if(way==1) lcd_draw_bmp(bmp_name[13],150,300,500,100);//起始坐标(225,150)
				else if(way==2) lcd_draw_bmp(bmp_name2[13],150,300,500,100);//起始坐标(225,150)
			}
			while(1){
				int ji = get_finger_move_direction1(0,0,300,150);
				if(ji)break;
			}
			break;
		}
	}
}
void print(){
	int i,j;
	for(i=0; i<4; i++){
		for(j=0; j<4; j++){
			printf("%d ",lock[i][j]);
		}printf("\n");
	}
}
char *get_file_name_by_digit_lock(int way,int num){
	
	if(way==1){
		switch(num){
			case 2:return bmp_name1_1[1];break;
			case 4:return bmp_name1_1[2];break;
			case 8:return bmp_name1_1[3];break;
			case 16:return bmp_name1_1[4];break;
			case 32:return bmp_name1_1[5];break;
			case 64:return bmp_name1_1[6];break;
			case 128:return bmp_name1_1[7];break;
			case 256:return bmp_name1_1[8];break;
			case 512:return bmp_name1_1[9];break;
			case 1028:return bmp_name1_1[10];break;
			case 2048:return bmp_name1_1[11];break;
			default:return NULL;
		}
	}else if(way==2){
		switch(num){
			case 2:return bmp_name2_2[1];break;
			case 4:return bmp_name2_2[2];break;
			case 8:return bmp_name2_2[3];break;
			case 16:return bmp_name2_2[4];break;
			case 32:return bmp_name2_2[5];break;
			case 64:return bmp_name2_2[6];break;
			case 128:return bmp_name2_2[7];break;
			case 256:return bmp_name2_2[8];break;
			case 512:return bmp_name2_2[9];break;
			case 1028:return bmp_name2_2[10];break;
			case 2048:return bmp_name2_2[11];break;
			default:return NULL;
		}
	}
}
char *get_file_name_by_digit(int way,int num){
	
	if(way==1){
		switch(num){
			case 2:return bmp_name[1];break;
			case 4:return bmp_name[2];break;
			case 8:return bmp_name[3];break;
			case 16:return bmp_name[4];break;
			case 32:return bmp_name[5];break;
			case 64:return bmp_name[6];break;
			case 128:return bmp_name[7];break;
			case 256:return bmp_name[8];break;
			case 512:return bmp_name[9];break;
			case 1028:return bmp_name[10];break;
			case 2048:return bmp_name[11];break;
			default:return NULL;
		}
	}else if(way==2){
		switch(num){
			case 2:return bmp_name2[1];break;
			case 4:return bmp_name2[2];break;
			case 8:return bmp_name2[3];break;
			case 16:return bmp_name2[4];break;
			case 32:return bmp_name2[5];break;
			case 64:return bmp_name2[6];break;
			case 128:return bmp_name2[7];break;
			case 256:return bmp_name2[8];break;
			case 512:return bmp_name2[9];break;
			case 1028:return bmp_name2[10];break;
			case 2048:return bmp_name2[11];break;
			default:return NULL;
		}
	}
}

void count(int cnt){ //计分
//	if(way==1)
//		lcd_draw_bmp( bmp_num[num[11]], 20, 50, 10, 0);
//	else if(way==2) 
//		lcd_draw_bmp( bmp_num[num[11]], 20, 50, 10, 0);
	int z,i = 0, j = cnt;
	while(j){
		num[i++] = j % 10;
		j /= 10;
	}
	for(z = 0; z<i; z++){//倒序输出 
		lcd_draw_bmp( bmp_num[num[i-1-z]], 20, 20, 10, 50 + z*20);
	}
}

void lcd_draw_point(int x, int y, int color){
	*(plcd + y*800 + x) = color;
}

void lcd_draw_rect(int x0, int y0, int w, int h, int color){
	int x, y;
	for (y = y0; y < y0 + h; y++){
		for (x = x0; x < x0 + w; x++)
			lcd_draw_point(x, y, color);
	}
}
//800 480
void lcd_draw_bmp(const char *file_name,int cc,int rr, int x0, int y0){
	int fd;	
	fd = open(file_name, O_RDONLY);
	if (fd < 0){
		perror("open file error: ");
		return ;
	}
	lseek(fd, 54, SEEK_SET);
	char buf[cc*rr*3];
	read(fd, buf, rr*cc*3);//因为这个bmp图片是24位色
	close(fd);
	int color, x, y, i = 0;
	unsigned char r, g, b;
	for (x = cc-1; x >= 0; x--){
		for (y = 0; y < rr; y++){ //反向点阵画 
			b = buf[i++];
			g = buf[i++];
			r = buf[i++];
			color = ( r<<16 ) | (g << 8) | (b);
//			lcd_draw_point(x+x0, rr-1-y+y0, color);//反向画 
			lcd_draw_point(x+x0, 479-y0-y, color); 
		}
	}
}
void lcd_draw_bmp1(const char *file_name,int cc,int rr, int x0, int y0){
	int fd;	
	fd = open(file_name, O_RDONLY);
	if (fd < 0){
		perror("open file error: ");
		return ;
	}
	lseek(fd, 54, SEEK_SET);
	char buf[cc*rr*3];
	read(fd, buf, cc*rr*3);//因为这个bmp图片是24位色
	close(fd);
	int color, x, y, i = 0;
	unsigned char r, g, b;
	for (x = cc-1; x >= 0; x--){
		for (y = 0; y < rr; y++){ //反向点阵画 
			b = buf[i++];
			g = buf[i++];
			r = buf[i++];
			color = ( r<<16 ) | (g << 8) | (b);
			lcd_draw_point(x+x0, rr-1-y+y0, color);//反向画 
//			lcd_draw_point(x+x0, 479-y0-y, color); 
		}
	}
}
void lcd_draw_matrix(int way,int x0, int y0, int num, int pice_size, int blank,int color){
	printf("enter matrix\n");
	int i, j;
	int x, y;
	for (j = 0; j < num; j++){
		y = y0 + (pice_size + blank)*j;
		for (i = 0; i < num; i++){
			x = x0 + (pice_size + blank)*i;	
			if (!matrix[i][j])
				lcd_draw_rect(x, y, pice_size, pice_size, color);
			else{
				if(way==1){
					lcd_draw_bmp1(get_file_name_by_digit(way,matrix[i][j]),100,100, x, y);
				}else{//way == 2
					if(!lock[i][j]) //没锁 
						lcd_draw_bmp1(get_file_name_by_digit(way,matrix[i][j]),100,100, x, y);
					else 
						lcd_draw_bmp1(get_file_name_by_digit_lock(way,matrix[i][j]),100,100, x, y);
				}
				
			} 
							
		}
	}
	print();
}
int get_finger_move_direction(){
	int fd, ret;
	int x1 = -1, y1 = -1;//手指滑动时第一个触点的坐标(x1,y1)
	int x2, y2;//往后的所有触点的坐标(x2,y2)
	struct input_event ev;
	fd = open("/dev/event0", O_RDWR);
	if (fd < 0){
		perror("open event0 error: ");
		return ;
	}
	while (1){
		ret = read(fd, &ev, sizeof(struct input_event));
		if (ret != sizeof(ev)){
			continue;
		}
		if ((ev.type == EV_ABS) && (ev.code == ABS_X)){
			if(x1 == -1) x1 = ev.value;
			else x2 = ev.value;
		}
		if ((ev.type == EV_ABS) && (ev.code == ABS_Y)){
			if (y1 == -1) y1 = ev.value;
			else y2 = ev.value;
		}
		if ((ev.type == EV_ABS) && (ev.code == ABS_PRESSURE) && (ev.value == 0 ) ){
			int delta_x = abs(x2 - x1);
			int delta_y = abs(y2 - y1);

			if (delta_x > 2*delta_y){
				if (x2 < x1){
					printf("UP ↑\n");
					close(fd);
					return MOVE_UP; //↓ 
				}else if (x2 > x1){
					printf("DOWN ↓\n");
					close(fd);
					return MOVE_DOWN; //↑ 
				}
			}else if (delta_x < 2*delta_y){
				if (y2 > y1){
					printf("RIGHT →\n");
					close(fd);
					return MOVE_RIGHT; //→ 
				}else if (y2 < y1){
					printf("LEFT ←\n");
					close(fd);
					return MOVE_LEFT; //← 
				}  
			}
			else{ //该手势不能识别 恭喜你再来一次			
				x1 = y1 = -1;
			}
		}
	}
}
int get_finger_move_direction1(int x_min,int y_min,int x_max,int y_max){
	int fd, ret;
	int x1=-1, y1= -1;
	int x2, y2;
	struct input_event ev;
	fd = open("/dev/event0", O_RDWR);
	if (fd < 0){
		perror("open event0 error: ");
		return ;
	}
	while (1){
		ret = read(fd, &ev, sizeof(struct input_event));
		if (ret != sizeof(ev))	continue;
		if ((ev.type == EV_ABS) && (ev.code == ABS_X)){
			if (x1 == -1) 	x1 = ev.value;
			else x2 = ev.value;
		}
		if ((ev.type == EV_ABS) && (ev.code == ABS_Y)){
			if (y1 == -1)y1 = ev.value;
			else y2 = ev.value;
		}
		if ((ev.type == EV_ABS) && (ev.code == ABS_PRESSURE) && (ev.value == 0) ){
			if(x1>=x_min && x1<=x_max && y1>=y_min && y1<=y_max ){
				return EXIST;
			}
			if(x2>=x_min && x2<=x_max && y2>=y_min && y2<=y_max ){
				return EXIST;
			}
			return NO_EXIST;
		}
	
	}
}

void swap(int *a, int *b){
	int temp = *a;
	*a = *b;
	*b = temp;
}
void move_right(){
	int i,j,z;
	for(i=0; i<4; i++){
		end = 3;
		for(j=0; j<4; j++){
			if(matrix[i][j]){
				if(!lock[i][j]){//没锁 
					for(z=j+1; z<4; z++){
						if( matrix[i][z] && !lock[i][z] ){
							if(matrix[i][j] == matrix[i][z]){
								judge_move_flag = 1;
								matrix[i][j] *= 2;
								cnt += matrix[i][j];//计分 
								matrix[i][z] = 0;
							}
							break;
						}
					}
					for(z=0;  z<end ; z++){
						if(!matrix[i][z]){
							swap(&matrix[i][j],&matrix[i][z]);					
							judge_move_flag = 1;
						}
					}
				}else{//锁定 
					if(j<3 && !lock[i][j+1]){
						if(matrix[i][j+1]==matrix[i][j]){
							matrix[i][j+1] = 0;
						 	lock[i][j] = 0;
							for(z=3 ;z>=end-2; z--){
								if(!matrix[i][z]) break;
								matrix[i][z] = matrix[i][z+1];
							}
						}
					}
					end = j;
				}
			}
			
		}
	}
}

void move_left(){
	int i,j,z;
	for(i=0; i<4; i++){
		end =3;
		for(j=3; j>=0 ; j--){
			if(matrix[i][j] ){
				if(!lock[i][j]){//没锁 
					for(z=j-1; z>=0; z--){
						if( matrix[i][z] && !lock[i][z]){//没锁  
							if(matrix[i][j] == matrix[i][z]){
								judge_move_flag = 1;
								matrix[i][j] *= 2;
								cnt += matrix[i][j];//计分 
								matrix[i][z] = 0;
							}			
							break;
						}
					}
					for(z=end; z>j ; z--){
							if(!matrix[i][z]){  //为0 
								judge_move_flag = 1;
								swap(&matrix[i][j],&matrix[i][z]); 						
						}
					}
				}else{//锁定 
					if(j>0 && !lock[i][j-1]){
						if(matrix[i][j-1]==matrix[i][j]){
							matrix[i][j-1] = 0;
						 	lock[i][j] = 0;
							int z;
							for(z=end-2;z>=0;z--){
								if(!matrix[i][z]) break;
								matrix[i][z+1] = matrix[i][z];
							}
						}
					}
					end = j;
				}				
			}
			
		}
	}
}
void move_up(){
	int i,j,z;
	for(i=0; i<4; i++){
		end = 3;
		for(j=0; j<4; j++){
			if(matrix[j][i]){
				if(!lock[j][i]){//没锁 
					for(z=j+1; z<4; z++){
						if( matrix[z][i] && !lock[z][i]){ 
							if( matrix[j][i] == matrix[z][i]){
								judge_move_flag = 1;
								matrix[j][i] *= 2;
								cnt += matrix[j][i];
								matrix[z][i] = 0;
							}
							break;
						} 
					}
					for(z=0; z<j ; z++){
						if(!matrix[z][i]){
							swap(&matrix[j][i],&matrix[z][i]);						
							judge_move_flag = 1;
						}
					}
				}else{
					if(j<3 && !lock[j+1][i]){
						if(matrix[j+1][i]==matrix[j][i]){
							matrix[j+1][i] = 0;
						 	lock[j][i] = 0;
							for(z=3 ;z>=end-2; z--){
								if(!matrix[z][i]) break;
								matrix[z][i] = matrix[z+1][i];
							}
						}
					}
					end = j;
				}
			}
			
		}
	}
}
void move_down(){
	int i,j,z;
	for(i=0; i<4; i++){
		end = 3;
		for(j=3; j>=0 ; j--){
			if(matrix[j][i] ){
				if(!lock[j][i] ){//没锁 
					for(z=j-1; z>=0; z--){
						if( matrix[z][i] && !lock[z][i] ){//没锁  
							if(matrix[j][i] == matrix[z][i] ){
								judge_move_flag = 1;
								matrix[j][i] *= 2;
								cnt += matrix[j][i];//计分 
								matrix[z][i] = 0;
							}			
							break;
						}
					}
					for(z=end; z>j ; z--){
							if( !matrix[z][i] ){  //为0 
								judge_move_flag = 1;
								swap( &matrix[j][i], &matrix[z][i] ); 						
						}
					}
				}else{//锁定 
					if(j>0 && !lock[j-1][i] ){
						if( matrix[j-1][i] == matrix[j][i] ){
							matrix[j-1][i] = 0;
						 	lock[j][i] = 0;
							int z;
							for(z=end-2;z>=0;z--){
								if(!matrix[z][i]) break;
								matrix[z+1][i] = matrix[z][i];
							}
						}
					}
					end = j;
				}				
			}
			
		}
	}
}

void delay(){
	int i;
	for(i=1;i<VALUE_DELAY;i++) ;
}
int search_blank(){
	int i,j;
	int cnt_blank = 0;
	for(i=0; i<4; i++){
		for(j=0; j<4; j++){
			if(!matrix[i][j]) {
				ro[++cnt_blank] = i;
				co[cnt_blank] = j;		
			}
		}
	}
	return cnt_blank;
}
void srand_num(int cnt_blank){
	delay();
	srand(time(NULL));//播种子
	int blank_th = rand() % cnt_blank + 1;
	matrix[ro[blank_th]][co[blank_th]] = ( rand()%2 ) * 2 + 2;
	lcd_draw_matrix(way,yy_start, xx_start, 4, 100, 20, _white);
}
void srand_lock(int cnt_blank){
	delay();
	srand(time(NULL));//播种子
	int blank_th = rand() % cnt_blank + 1;
	lock[ro[blank_th]][co[blank_th]] = 1;
	lcd_draw_matrix(way,yy_start, xx_start, 4, 100, 20, _white);
}

void intl(){
	cnt = 0;
	plcd = NULL; 
	judge_move_flag = 0;
	memset(matrix,0,sizeof(matrix));
	memset(lock,0,sizeof(lock));
	memset(ro,0,sizeof(ro));
	memset(co,0,sizeof(co));	
	memset(num,0,sizeof(num));
	srand_num( search_blank());
	srand_num( search_blank());
}

int judge(){
	int i,j;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			if(matrix[i][j]==2048) return 2;
			
			if(j<3 && matrix[i][j]==matrix[i][j+1])	return 1;
		
			if(j<3 && matrix[j][i]==matrix[j+1][i]) return 1;
			
		}
	}
	return 0;
}
