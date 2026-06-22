#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h> // Для uint32_t
#include <stdbool.h> // Для bool

// Узел цепочки - элемент связного списка в одной ячейке
typedef struct HNode {
	char* key; // Строка-ключ
	int value; // Значение
	struct HNode* next; // Указатель на следующий узел
} HNode;

// Хэш-таблица
typedef struct {
	HNode** nodes; // Массив указателей на головы цепочек
	int capacity; // Количество связных списков
	int size; // Сколько всего элементов в таблице
} HashTable;

// Множество - обертка над HashTable
// Ключ = значение элемента, value всегда 1
typedef struct {
	HashTable* ht; // внутри хэш-таблица
} Set;

// Мультимножество
// Ключ = значение элемента, а value сколько раз он встречается
typedef struct {
	HashTable* ht;
} MultiSet;

// Хэш-функция
uint32_t fnv1a_hash(const char* key);

// Хэш-таблица
HashTable* ht_create(int capacity);
void ht_destroy(HashTable* ht);
void ht_put(HashTable* ht, const char* key, int value);
bool ht_contains(const HashTable* ht, const char* key);
bool ht_get(const HashTable* ht, const char* key, int* out_value);
bool ht_remove(HashTable* ht, const char* key);
int ht_size(const HashTable* ht);

// Множество
Set* set_create(int capacity);
void set_destroy(Set* set);
void set_add(Set* set, const char* key);
bool set_contains(const Set* set, const char* key);
bool set_remove(Set* set, const char* key);
int set_size(const Set* set);
Set* set_union(const Set* a, const Set* b);
Set* set_intersection(const Set* a, const Set* b);
Set* set_difference(const Set* a, const Set* b);
void set_print(const Set* set);

// Мультимножество
MultiSet* multiset_create(int capacity);
void multiset_destroy(MultiSet* ms);
void multiset_add(MultiSet* ms, const char* key, int count);
bool multiset_remove(MultiSet* ms, const char* key, int count);
int multiset_count(const MultiSet* ms, const char* key);
bool multiset_contains(const MultiSet* ms, const char* key);
void multiset_print(const MultiSet* ms);

#endif HASHTABLE_H