#include<stdio.h>
#include<stdlib.h>
#include<time.h>
int ro[16]={0};
int co[16]={0};
int matrix[4][4] = {
	8, 8, 0, 0,
	2, 16, 32, 4,
	8, 4, 32, 2, 
	4, 2, 2, 0,
};
int lock[4][4] = {
	1,0,0,0,
	0,0,1,0,
	0,0,0,0, 
	0,0,1,0,
};
int lock_flag=0,judge_move_flag = 0;
void swap(int *a, int *b){
	int temp = *a;
	*a = *b;
	*b = temp;
}

enum finger_move{
	MOVE_LEFT,
	MOVE_RIGHT,
	MOVE_DOWN,
	MOVE_UP,
};
int cnt = 0;
int is_move = 1;

void move_down_lock(){
	int end,i,j,z;
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
void move_up(){
	int end,i,j,z;
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
							for(z=2 ;z>=end-2; z--){
								if(!matrix[z][i]) break;
								matrix[z-1][i] = matrix[z+1][i];
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
	int end,i,j,z;
	for(i=0; i<4; i++){
		end = 3;
		for(j=3; j>=0 ; j--){
			if(matrix[j][i] ){
					for(z=j-1; z>=0; z--){
						if( matrix[z][i]  ){//没锁  
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
			}
			
		}
	}
}


int num[10];
void count(int cnt){
	int z,i = 0, j = cnt;
	while(j){
		num[i++] = j % 10;
		j /= 10;
	}
	for(z=i-1;z>=0;z--){//倒序输出 
		printf("%d ",num[z]);
	}
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

void srand_lock(int cnt_blank){
	
	srand(time(NULL));//播种子
	int blank_th = rand() % cnt_blank + 1;
	lock[ro[blank_th]][co[blank_th]] = 1;
//	lcd_draw_matrix(way, yy_start, xx_start, 4, 100, 20, _white);
}
void print_matrix(){
	int i,j;
	for(i=0; i<4; i++){
		for(j=0; j<4; j++){
			printf("%d ",matrix[i][j]);
		}
		printf("\n");
	}
}
void print_lock(){
	int i,j;
	for(i=0; i<4; i++){
		for(j=0; j<4; j++){
			printf("%d ",lock[i][j]);
		}
		printf("\n");
	}
}
int main(){
	print_matrix();
	print_lock();
	printf("\n");
	move_up();
//	move_down_lock();
//	srand_lock( search_blank()); //lock随机产生空 
//	srand_num( search_());
	printf("%d\n",judge_move_flag);
	print_matrix();
	print_lock();
	
	printf("cnt = %d\n",cnt);//计分 
	
	cnt = 115693;
	printf("modify cnt = %d\n",cnt);//计分 
	count(cnt);
	return 0;
}


