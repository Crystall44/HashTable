#define _CRT_NONSTDC_NO_DEPRECATE
#include "hashtable.h"
#include <stdlib.h> // Для malloc, free, calloc
#include <string.h> // Для strcmp, strdup
#include <stdio.h> // Для printf

// Хэш-функция FNV-1a (32 бита)
uint32_t fnv1a_hash(const char* key) {
	// Начальное значение хэша - не случайное
	uint32_t hash = 0x811c9dc5u;

	// Простое число, на которое будем умножать
	const uint32_t prime = 0x01000193u;

	// Идем посимвольно пока не встретим '\0'
	while (*key) {
		// Шаг 1: XOR текущего хэша с байтом строки
		hash ^= (uint8_t)(*key);

		// Шаг 2: умножаем на простое число
		hash *= prime;

		// Переходим к следующему символу
		key++;
	}

	return hash;
}

// Вспомогательные функции для узлов

// Создать узел цепочки
static HNode* node_create(const char* key, int value) {
	// Выделяем память
	HNode* node = malloc(sizeof(HNode));
	if (!node) return NULL;

	// Копируем строку ключа
	node->key = strdup(key);
	node->value = value;
	node->next = NULL;

	return node;
}

// Удалить узел
static void node_destroy(HNode* node) {
	if (!node) return;
	free(node->key); // Освобождаем строку
	free(node); // И сам узел
}

// Хэш-таблица

// Создать таблицу
HashTable* ht_create(int capacity) {
	// Выделяем память
	HashTable* ht = malloc(sizeof(HashTable));
	if (!ht) return NULL;

	ht->capacity = capacity;
	ht->size = 0;

	// Выделяем массив указателей на головы цепочек
	ht->nodes = calloc(capacity, sizeof(HNode*));

	return ht;
}

// Удалить таблицу
void ht_destroy(HashTable* ht) {
	if (!ht) return;

	for (int i = 0; i < ht->capacity; i++) {
		HNode* current = ht->nodes[i];

		while (current) {
			HNode* next = current->next;
			node_destroy(current);
			current = next;
		}
	}

	free(ht->nodes);
	free(ht);
}

// Вставить пару (ключ, значение) в таблицу
// Если ключ уже есть то обновляет значение
void ht_put(HashTable* ht, const char* key, int value) {
	// Вычисляем индекс узла: берем хэш и остаток от деления
	uint32_t idx = fnv1a_hash(key) % ht->capacity;

	HNode* current = ht->nodes[idx];

	// Идем по цепочке - ищем, есть ли такой ключ
	while (current) {
		if (strcmp(current->key, key) == 0) {
			// Нашли такой ключ - обновляем значение
			current->value = value;
			return;
		}
		current = current->next;
	}

	// Ключа в цепочке нет. Создаем новый узел
	HNode* node = node_create(key, value);

	// Вставляем в начало цепочки
	node->next = ht->nodes[idx];
	ht->nodes[idx] = node;

	ht->size++;
}

// Проверяем, есть ли ключ в таблице
bool ht_contains(const HashTable* ht, const char* key) {
	return ht_get(ht, key, NULL);
}

// Ишем значение по ключу
bool ht_get(const HashTable* ht, const char* key, int* out_value) {
	// Вычисляем индекс
	uint32_t idx = fnv1a_hash(key) % ht->capacity;

	// Идем по цепочке
	HNode* current = ht->nodes[idx];
	while (current) {
		if (strcmp(current->key, key) == 0) {
			//Ключ найден
			if (out_value) *out_value = current->value; // Записываем значение если передан указатель
			return true;
		}
		current = current->next;
	}

	// Не нашли
	return false;
}

// Удаляем ключ из таблицы
bool ht_remove(HashTable* ht, const char* key) {
	uint32_t idx = fnv1a_hash(key) % ht->capacity;

	HNode* current = ht->nodes[idx];
	HNode* pred = NULL; // Предыдущий узел, в начале его нет

	// Идем по цепочке
	while (current) {
		if (strcmp(current->key, key) == 0) {
			// Нашли узел
			if (pred) {
				// Случай 1: Удаляем не голову. Предыдущий узел теперь указывает на следующий
				pred->next = current->next;
			}
			else {
				// Случай 2: удаляем голову цепочки. Указатель в массиве nodes указывает на следующий
				ht->nodes[idx] = current->next;
			}

			// Освобождаем память
			node_destroy(current);

			ht->size--;
			return true;
		}

		// Переходим к сдедующему узлу
		pred = current;
		current = current->next;
	}

	// Прошли все, ключ не нашли
	return false;
}

// Количество элементов в таблице
int ht_size(const HashTable* ht) {
	return ht->size;
}

// Множество. Это хэш-таблица, где значение всегда 1 - означает что "элемент присутствует"
Set* set_create(int capacity) {
	Set* set = malloc(sizeof(Set));
	set->ht = ht_create(capacity);
	return set;
}

void set_destroy(Set* set) {
	if (!set) return;
	ht_destroy(set->ht);
	free(set);
}

// Добавить элемент во множество. Если уже есть то ничего не делаем (без дубликатов)
void set_add(Set* set, const char* key) {
	// Просто кладем 1 по ключу
	ht_put(set->ht, key, 1);
}

bool set_contains(const Set* set, const char* key) {
	return ht_contains(set->ht, key);
}

bool set_remove(Set* set, const char* key) {
	return ht_remove(set->ht, key);
}

int set_size(const Set* set) {
	return ht_size(set->ht);
}

// Обьединение множеств - все элементы множества из A плюс все из B
Set* set_union(const Set* a, const Set* b) {
	// Создаем новое множество, размер побольше
	Set* result = set_create(a->ht->capacity + b->ht->capacity);

	// Проходим по всем цепочкам таблицы A
	for (int i = 0; i < a->ht->capacity; i++) {
		HNode* current = a->ht->nodes[i];
		while (current) {
			set_add(result, current->key);
			current = current->next;
		}
	}

	// Проходим по всем цепочкам таблицы B
	for (int i = 0; i < b->ht->capacity; i++) {
		HNode* current = b->ht->nodes[i];
		while (current) {
			set_add(result, current->key);
			current = current->next;
		}
	}

	return result;
}

// Пересичение множеств: только те элементы, которые есть и в A, и в B
Set* set_intersection(const Set* a, const Set* b) {
	// Выбираем меньшее множество для перебора
	int cap = (a->ht->capacity < b->ht->capacity) ? a->ht->capacity : b->ht->capacity;
	Set* result = set_create(cap);

	// Перебираем элементы множества A
	for (int i = 0; i < a->ht->capacity; i++) {
		HNode* current = a->ht->nodes[i];
		while (current) {
			// Если элемент есть в B - добавляем
			if (set_contains(b, current->key)) set_add(result, current->key);
			current = current->next;
		}
	}

	return result;
}

// Разность множеств: элементы из A, которых нет в B
Set* set_difference(const Set* a, const Set* b) {
	Set* result = set_create(a->ht->capacity);

	for (int i = 0; i < a->ht->capacity; i++) {
		HNode* current = a->ht->nodes[i];
		while (current) {
			if (!set_contains(b, current->key)) set_add(result, current->key);
			current = current->next;
		}
	}

	return result;
}

void set_print(const Set* set) {
	printf("{ ");
	for (int i = 0; i < set->ht->capacity; i++) {
		HNode* current = set->ht->nodes[i];
		while (current) {
			printf("\"%s\" ", current->key);
			current = current->next;
		}
	}

	printf("}\n");
}

// Мультимножество - хэш-таблица, где значение - целое число (сколько раз элемент был добавлен)
MultiSet* multiset_create(int capacity) {
	MultiSet* ms = malloc(sizeof(MultiSet));
	ms->ht = ht_create(capacity);
	return ms;
}

void multiset_destroy(MultiSet* ms) {
	if (!ms) return;
	ht_destroy(ms->ht);
	free(ms);
}

// Добавляем элемент count раз
void multiset_add(MultiSet* ms, const char* key, int count) {
	int current = 0;
	//Смотрим сколько уже есть
	ht_get(ms->ht, key, &current);

	// Кладем сумму
	ht_put(ms->ht, key, current + count);
}

// Удаляет элемент count раз
bool multiset_remove(MultiSet* ms, const char* key, int count) {
	int current = 0;

	// Если элемента нет - удалять нечего
	if (!ht_get(ms->ht, key, &current)) return false;

	// Считаем, сколько останется
	int new_count = current - count;

	if (new_count <= 0) {
		//Убрали всё (или больше) - удаляем ключ
		return ht_remove(ms->ht, key);
	}
	else {
		// Иначе - обновляем счетчик
		ht_put(ms->ht, key, new_count);
		return true;
	}
}

// Возвращает число вхождение элемента
int multiset_count(const MultiSet* ms, const char* key) {
	int current = 0;
	ht_get(ms->ht, key, &current);
	return current;
}

bool multiset_contains(const MultiSet* ms, const char* key) {
	return multiset_count(ms, key) > 0;
}

void multiset_print(const MultiSet* ms) {
	printf("{ ");
	for (size_t i = 0; i < ms->ht->capacity; i++) {
		HNode* current = ms->ht->nodes[i];
		while (current) {
			printf("\"%s\":%d ", current->key, current->value);
			current = current->next;
		}
	}
	printf("}\n");
}