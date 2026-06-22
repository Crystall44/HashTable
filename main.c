#include <stdio.h>
#include <string.h>
#include "hashtable.h"
setlocale(LC_ALL, "Russian");
// Счетчики для итогов
static int passed = 0;
static int failed = 0;

// Функция для проверки условий и вывода результата
static void check(const char* description, int condition) {
	if (condition) {
		printf("OK\n", description);
		passed++;
	}
	else {
		printf("FAIL\n", description);
		failed++;
	}
}

// Тест 1: хэш-функция FNV-1a
static void test_hach_funcvtion(void) {
	printf("Тест 1: хэш-функция FNV-1a\n");
	// 1.1 детерминированность - одинаковый ключ дает одинаковый хэш
	printf("1.1 Детерминированность\n");
	uint32_t h1 = fnv1a_hash("hello");
	uint32_t h2 = fnv1a_hash("hello");
	check("одинаковый ключ -> одинаковый хэш", h1 == h2);

	// 1.2 Разные ключи - разные хэши
	printf("\n1.2 Разные ключи - разные хэши\n");
	uint32_t h3 = fnv1a_hash("apple");
	uint32_t h4 = fnv1a_hash("banana");
	uint32_t h5 = fnv1a_hash("cherry");
	check("apple != banana", h3 != h4);
	check("apple != cherry", h3 != h5);
	check("banana != cherry", h4 != h5);

	// 1.3 Лавинный эффект - маленькое изменение дает совсем другой хэш
	printf("\n1.3 Лавинный эффект\n");
	uint32_t h_cat = fnv1a_hash("cat");
	uint32_t h_cut = fnv1a_hash("cut");
	uint32_t h_cap = fnv1a_hash("cap");
	check("cat != cut (изменилась 1 буква)", h_cat != h_cut);
	check("cat != cap", h_cat != h_cap);
	check("cut != cap", h_cut != h_cap);

	// 1.4 Пустая строка
	printf("\n1.4 Крайние случаи\n");
	uint32_t h_empty = fnv1a_hash("");
	check("пустая строка даёт хеш", h_empty > 0);  // не ноль

	/* 1.5 Длинная строка */
	uint32_t h_long = fnv1a_hash("Ну_очень_длинная_строка_для_теста_хеш_функции_1234567890");
	check("длинная строка даёт хеш", h_long > 0);
}

// Тест 2: создание и удаление хэш-таблицы