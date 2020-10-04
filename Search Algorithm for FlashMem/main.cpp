#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <random>
#include <math.h>
#include <stdint.h>

#define EMPTY true
#define NOT_EMPTY false
#define SEARCH_FAIL -1

#define TEST_PASSES 100 //테스트 횟수(1 이상)
#define BLOCK_PER_SECTOR 32
#define INIT_VALUE INT8_MAX

/***
* 1) 0~31의 범위를 가진 배열 == 블록 하나의 섹터(페이지) 수
* 2) 해당 배열은 플래시 메모리의 페이지 단위 매핑을 사용할 경우, 항상 순차적으로 빈 페이지부터 기록되는 특성을 따른다.
* 2) 해당 배열에서 어떤 인덱스 위치의 자리가 비어있지않을 경우(false), 그 이전 인덱스 위치들은 모두 비어있지 않다.
* 3) 해당 배열에서 어떤 인덱스 위치의 자리가 비어있을 경우(true), 그 이전 인덱스 위치들은 비어있거나 비어있지 않을 수 있다.
* 4) 이진탐색 알고리즘의 분할 정복 탐색 기법에 의한 빈 섹터(페이지)를 찾는 방법과 순차적으로 빈 섹터(페이지)를 찾는 방법을 비교
* 5) 배열의 랜덤한 인덱스 위치까지 기록되어 있다고 가정하여 테스트 횟수만큼 반복 수행
***/

bool* block = NULL;
int flash_read_count = 0;
int rand_index = 0;

std::random_device rd; //non-deterministic generator
std::mt19937 gen(rd()); //to seed mersenne twister
std::uniform_int_distribution<int> distribution(0, BLOCK_PER_SECTOR - 1); //0~31

void set_block(bool*& block, int& rand_index)
{	
	if(block == NULL)
		block = new bool[BLOCK_PER_SECTOR];

	for (int i = 0; i < BLOCK_PER_SECTOR; i++)
		block[i] = EMPTY;

	//랜덤한 인덱스 위치까지 기록되어 있다고 가정
	rand_index = distribution(gen);

	block[rand_index] = NOT_EMPTY; //해당 위치는 기록된 위치
	for (int i = rand_index; i >= 0; i--) //이전 위치들 모두 기록된 위치로 변경
		block[i] = NOT_EMPTY;

}

void print_block(bool*& block)
{
	for (int i = 0; i < BLOCK_PER_SECTOR; i++)
	{
		printf("Offset %d : ", i);
		printf(block[i] ? "EMPTY\n" : "NOT_EMPTY\n");
	}
}

int binary_search_for_empty_page(bool*& block, int& flash_read_count) //이진탐색 변형
{
	//빈 페이지 위치를 찾아서 반환
	
	int low = 0;
	int high = BLOCK_PER_SECTOR -1;
	int mid = round((low+high) / 2);
	int current_empty_index = INIT_VALUE;

	while (low<=high) {
		flash_read_count++;

		mid = (low + high) / 2;

		if (block[mid] == EMPTY) //비어있으면
		{
			//왼쪽으로 탐색
			current_empty_index = mid;
			high = mid-1;
		}
		else //비어있지 않으면
		{
			//왼쪽은 모두 비어있지 않음
			//오른쪽으로 탐색
			low = mid+1;
		}
	}

	if (current_empty_index != INIT_VALUE)
		return current_empty_index;

	return SEARCH_FAIL;
}

int seq_search_for_empty_page(bool*& block, int& flash_read_count) //순차탐색
{
	for (int i = 0; i < BLOCK_PER_SECTOR; i++)
	{
		flash_read_count++;
		
		if (block[i] == EMPTY)
			return i;
	}

	return SEARCH_FAIL;
}


void main()
{
	int result_empty_page = SEARCH_FAIL;
	int average_flash_read_count = 0;
	
	for (int i = 0; i < TEST_PASSES; i++)
	{
		set_block(block, rand_index);
		print_block(block);

		//result_empty_page = seq_search_for_empty_page(block, flash_read_count);
		result_empty_page = binary_search_for_empty_page(block, flash_read_count);

		if (result_empty_page != SEARCH_FAIL)
		{
			printf("--------------------------------------------------\n");
			printf("Empty Offset In Block : %d\n", result_empty_page);
			printf("flash_read_count : %d\n", flash_read_count);
			printf("--------------------------------------------------\n");
		}
		average_flash_read_count += flash_read_count;
		flash_read_count = 0;
	}
	average_flash_read_count /= TEST_PASSES;

	printf("--------------------------------------------------\n");
	printf("Test Passes : %d\n", TEST_PASSES);
	printf("Average flash_read_count : %d\n", average_flash_read_count);
	printf("--------------------------------------------------\n");

	system("pause");
}