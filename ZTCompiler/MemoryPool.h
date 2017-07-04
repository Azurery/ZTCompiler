#pragma once
#ifndef _ZT_MEMORYPOOL_H_
#define _ZT_MEMORYPOOL_H_
#include <vector>
#include <cassert>
namespace ztCompiler {
	class memory_pool {
		static const int block_size = 4 * 1024;	//Ĭ�ϵ��ڴ�����С
	private:
		char* alloc_ptr;	//�ڴ��ƫ��ָ�룬ָ��δʹ�õ��ڴ���׵�ַ
		size_t remained;	//ʣ����ڴ�
		std::vector<char*> blocks;	//
		size_t allocated_memory;	//�ѷ�������ڴ�
	public:
		memory_pool() {
			allocated_memory = 0;
			alloc_ptr = nullptr;
			remained = 0;
		}

		~memory_pool() {
			for (int i = 0; i < blocks.size();++i)
				delete[] blocks[i];
		}
	public:
		char* allocate(size_t bytes) {
			assert(bytes >0);
			//�����Ҫ���ڴ�����ʣ����ڴ棬��ֱ�ӷ��䡣���ƶ�ָ�룬����ʣ���ڴ����������ظշ����ڴ���׵�ַ
			if (bytes <= remained) {	
				char* begin_pos = alloc_ptr;	//�ȱ���ָ��ĳ�ʼλ��
				alloc_ptr += bytes;
				remained -= bytes;
				return begin_pos;
			}
			//�������ڴ����ʣ���ڴ�ʱ
			return allocate_fallback(bytes);
		}
	private:
		char* allocate_fallback(size_t bytes) {
			if (bytes > block_size/4) {	//�������ڴ����1024kbʱ������䵥������һ���СΪbytes���ڴ�
				char* ret = allocate_new_block(bytes);
				return ret;
			}
			//�������ڴ����ʣ���ڴ棬��С��1024kbʱ
			alloc_ptr = allocate_new_block(block_size);
			remained = block_size;

			char* begin_pos = alloc_ptr;
			alloc_ptr += bytes;
			remained -= bytes;
			return begin_pos;
		}
		char* allocate_new_block(size_t bytes) {
			char* ret = new char[bytes];	//�����ڴ�
			allocated_memory += bytes;
			blocks.push_back(ret);	//��ӽ��ڴ�ָ������
			return ret;
		}
	};
}

#endif