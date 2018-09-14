#pragma once
#ifndef _ZT_MEMORYPOOL_H_
#define _ZT_MEMORYPOOL_H_
#include <vector>
#include <cassert>
namespace ztCompiler {
// 	class memory_pool {
// 		static const int block_size = 4 * 1024;	//Ĭ�ϵ��ڴ�����С
// 	private:
// 		char* alloc_ptr;	//�ڴ��ƫ��ָ�룬ָ��δʹ�õ��ڴ���׵�ַ
// 		size_t remained;	//ʣ����ڴ�
// 		std::vector<char*> blocks;	//
// 		size_t allocated_memory;	//�ѷ�������ڴ�
// 	public:
// 		memory_pool() {
// 			allocated_memory = 0;
// 			alloc_ptr = nullptr;
// 			remained = 0;
// 		}
// 
// 		~memory_pool() {
// 			for (int i = 0; i < blocks.size();++i)
// 				delete[] blocks[i];
// 		}
// 	public:
// 		char* allocate(size_t bytes) {
// 			assert(bytes >0);
// 			//�����Ҫ���ڴ�����ʣ����ڴ棬��ֱ�ӷ��䡣���ƶ�ָ�룬����ʣ���ڴ����������ظշ����ڴ���׵�ַ
// 			if (bytes <= remained) {	
// 				char* begin_pos = alloc_ptr;	//�ȱ���ָ��ĳ�ʼλ��
// 				alloc_ptr += bytes;
// 				remained -= bytes;
// 				return begin_pos;
// 			}
// 			//�������ڴ����ʣ���ڴ�ʱ
// 			return allocate_fallback(bytes);
// 		}
// 	private:
// 		char* allocate_fallback(size_t bytes) {
// 			if (bytes > block_size/4) {	//�������ڴ����1024kbʱ������䵥������һ���СΪbytes���ڴ�
// 				char* ret = allocate_new_block(bytes);
// 				return ret;
// 			}
// 			//�������ڴ����ʣ���ڴ棬��С��1024kbʱ
// 			alloc_ptr = allocate_new_block(block_size);
// 			remained = block_size;
// 
// 			char* begin_pos = alloc_ptr;
// 			alloc_ptr += bytes;
// 			remained -= bytes;
// 			return begin_pos;
// 		}
// 		char* allocate_new_block(size_t bytes) {
// 			char* ret = new char[bytes];	//�����ڴ�
// 			allocated_memory += bytes;
// 			blocks.push_back(ret);	//��ӽ��ڴ�ָ������
// 			return ret;
// 		}
// 	};

	class MemPool {
	public:
		MemPool() : allocated_(0) {}
		virtual ~MemPool() {}
		MemPool(const MemPool& other) = delete;
		MemPool& operator=(const MemPool& other) = delete;
		virtual void* allocate() = 0;
		virtual void free(void* addr) = 0;
		virtual void clear() = 0;

	protected:
		size_t allocated_;
	};


	template <class T>
	class MemPoolImp : public MemPool {
	public:
		MemPoolImp() : root_(nullptr) {}
		virtual ~MemPoolImp() {}
		MemPoolImp(const MemPool& other) = delete;
		MemPoolImp& operator=(MemPool& other) = delete;
		virtual void* allocate();
		virtual void free(void* addr);
		virtual void clear();

	private:
		enum {
			COUNT = (4 * 1024) / sizeof(T)
		};

		union Chunk {
			Chunk* next_;
			char mem_[sizeof(T)];
		};

		struct Block {
			Block() {
				for (size_t i = 0; i < COUNT - 1; ++i)
					chunks_[i].next_ = &chunks_[i + 1];
				chunks_[COUNT - 1].next_ = nullptr;
			}
			Chunk chunks_[COUNT];
		};

		std::vector<Block*> blocks_;
		Chunk* root_;
	};


	template <class T>
	void* MemPoolImp<T>::allocate() {
		if (nullptr == root_) { // �ռ䲻������Ҫ����ռ�
			auto block = new Block();
			root_ = block->chunks_;
			// ���blocksʵ��Ϊstd::list, ��ôpush_backʵ�ʵ�overhead����
			// ��Ҳ��������ʹ���ǲ���Ҫ������ʹ���(��ôstd::vector�Ŀ�����һ��overhead)��
			// ��Ȼ������ʹ��std::vector��
			// ��Ȼstd::vector��ָ����capacity����������ڴ��˷ѡ�
			blocks_.push_back(block);
		}

		auto ret = root_;
		root_ = root_->next_;

		++allocated_;
		return ret;
	}


	template <class T>
	void MemPoolImp<T>::free(void* addr) {
		if (nullptr == addr)
			return;

		auto chunk = static_cast<Chunk*>(addr);
		chunk->next_ = root_;
		root_ = chunk;

		--allocated_;
	}


	template <class T>
	void MemPoolImp<T>::clear() {
		for (auto block : blocks_)
			delete block;

		blocks_.resize(0);
		root_ = nullptr;
		allocated_ = 0;
	}

}

#endif