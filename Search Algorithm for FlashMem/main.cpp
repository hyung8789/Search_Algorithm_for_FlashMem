#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <random>
#include <math.h>
#include <stdint.h>

#define EMPTY 0
#define NOT_EMPTY 1
#define SEARCH_FAIL -1
#define BLOCK_PER_SECTOR 32
#define INIT_VALUE INT8_MAX

#define TEST_PASSES 200 //�׽�Ʈ Ƚ��(1 �̻�)

/***
* 1) 0~31�� ������ ���� �迭 == ��� �ϳ��� ����(������) ��
* 2) �ش� �迭�� �÷��� �޸��� ������ ���� ������ ����� ���, �׻� ���������� �� ���������� ��ϵǴ� Ư���� ������.
* 2) �ش� �迭���� � �ε��� ��ġ�� �ڸ��� ����������� ���(false), �� ���� �ε��� ��ġ���� ��� ������� �ʴ�.
* 3) �ش� �迭���� � �ε��� ��ġ�� �ڸ��� ������� ���(true), �� ���� �ε��� ��ġ���� ����ְų� ������� ���� �� �ִ�.
* 4) ����Ž�� �˰����� ���� ���� Ž�� ����� ���� �� ����(������)�� ã�� ����� ���������� �� ����(������)�� ã�� ����� ��
* 5) �迭�� ������ �ε��� ��ġ���� ��ϵǾ� �ִٰ� �����Ͽ� �׽�Ʈ Ƚ����ŭ �ݺ� ����
***/

int* block = NULL;
int flash_read_count = 0;
int rand_index = 0;

std::random_device rd; //non-deterministic generator
std::mt19937 gen(rd()); //to seed mersenne twister
std::uniform_int_distribution<int> distribution(0, BLOCK_PER_SECTOR - 1); //0~31

void set_block(int*& block, int& rand_index)
{	
	if(block == NULL)
		block = new int[BLOCK_PER_SECTOR];

	for (int i = 0; i < BLOCK_PER_SECTOR; i++)
		block[i] = EMPTY;

	//������ �ε��� ��ġ���� ��ϵǾ� �ִٰ� ����
	rand_index = distribution(gen);

	block[rand_index] = NOT_EMPTY; //�ش� ��ġ�� ��ϵ� ��ġ
	for (int i = rand_index; i >= 0; i--) //���� ��ġ�� ��� ��ϵ� ��ġ�� ����
		block[i] = NOT_EMPTY;
}

void print_block(int*& block)
{
	for (int i = 0; i < BLOCK_PER_SECTOR; i++)
	{
		printf("Offset %d : ", i);
		printf(block[i] ? "NOT_EMPTY\n" : "EMPTY\n"); //1, 0
	}
}

int binary_search_for_empty_page(int*& block, int& flash_read_count) //����Ž��
{
	int low = 0;
	int high = BLOCK_PER_SECTOR -1;
	int mid = 0;

	int current_empty_index = INIT_VALUE;

	while (low<=high) {
		flash_read_count++;

		mid = (low + high) / 2;

		if (block[mid] == EMPTY) //���������
		{
			//�������� Ž��
			current_empty_index = mid;
			high = mid-1;
		}
		else //������� ������
		{
			//������ ��� ������� ����
			//���������� Ž��
			low = mid+1;
		}
	}

	if (current_empty_index != INIT_VALUE)
		return current_empty_index;

	return SEARCH_FAIL;
}

int wearlevel_binary_search_for_empty_page(int*& block, int& flash_read_count) //����Ž�� ����
{
	//Wear-leveling�� ���� �ʱ� mid���� ������ ������ ����

	int low = 0;
	int high = BLOCK_PER_SECTOR - 1;
	int mid = distribution(gen);

	int current_empty_index = INIT_VALUE;

	while (low <= high) {
		flash_read_count++;

		if (block[mid] == EMPTY) //���������
		{
			//�������� Ž��
			current_empty_index = mid;
			high = mid - 1;
		}
		else //������� ������
		{
			//������ ��� ������� ����
			//���������� Ž��
			low = mid + 1;
		}

		mid = (low + high) / 2;
	}

	if (current_empty_index != INIT_VALUE)
		return current_empty_index;

	return SEARCH_FAIL;
}


int seq_search_for_empty_page(int*& block, int& flash_read_count) //����Ž��
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

		//result_empty_page = seq_search_for_empty_page(block, flash_read_count); //���� Ž��
		//result_empty_page = binary_search_for_empty_page(block, flash_read_count); //���� Ž��
		result_empty_page = wearlevel_binary_search_for_empty_page(block, flash_read_count); //Wear-leveling�� ���� �ʱ� mid�� �����ϰ� �ְ� ���� Ž��

		if (result_empty_page != SEARCH_FAIL)
		{
			printf("--------------------------------------------------\n");
			printf("Empty Offset In Block : %d\n", result_empty_page);
			printf("flash_read_count : %d\n", flash_read_count);
			printf("--------------------------------------------------\n");
		}
		/*
		else
		{
			printf("--------------------------------------------------\n");
			printf("No Empty Page in block\n");
			printf("Current rand_index : %d\n", rand_index);
			printf("Current Passes : %d\n", i);
			printf("--------------------------------------------------\n");
			system("pause");
		}
		*/

		average_flash_read_count += flash_read_count;
		flash_read_count = 0;
		result_empty_page = SEARCH_FAIL;
	}
	//average_flash_read_count /= TEST_PASSES;

	printf("--------------------------------------------------\n");
	printf("Test Passes : %d\n", TEST_PASSES);
	printf("Total flash_read_count : %d\n", average_flash_read_count);
	printf("Average flash_read_count : %d\n", average_flash_read_count / TEST_PASSES);
	printf("--------------------------------------------------\n");

	system("pause");
}