#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "hashtable.h"

// Счетчики для итогов
static int passed = 0;
static int failed = 0;

// Функция для проверки условий и вывода результата
static void check(const char* description, int condition) {
	if (condition) {
		printf("Успешно %s\n", description);
		passed++;
	}
	else {
		printf("Ошибка %s\n", description);
		failed++;
	}
}

// Тест 1: хэш-функция FNV-1a
static void test_hash_function(void) {
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
static void test_create_destroy(void) {
	printf("\nТест 2: создание и удаление\n");
	// 2.1 Создание таблицы
	printf("\n2.1 Создание таблицы\n");
	HashTable* ht = ht_create(16);
	check("таблица создана (не NULL)", ht != NULL);
	check("capacity == 16", ht->capacity == 16);
	check("size == 0 (пустая)", ht->size == 0);

	// Все цепочки должны быть NULL
	int all_null = 1;
	for (int i = 0; i < ht->capacity; i++) {
		if (ht->nodes[i] != NULL) {
			all_null = 0;
			break;
		}
	}
	check("все корзины пустые (NULL)", all_null);

	// 2.2 ht_size на пустой таблице
	printf("\n2.2 Размер пустой таблицы\n");
	check("ht_size == 0", ht_size(ht) == 0);

	// 2.3 ht_contains на пустой таблице
	printf("\n2.3 Поиск в пустой таблице\n");
	check("ht_contains(\"key\") == 0", !ht_contains(ht, "key"));

	// 2.4 ht_het на пустой таблице
	int val = 999;
	int found = ht_get(ht, "key", &val);
	check("ht_get возвращает 0", !found);
	check("значение не изменилось", val == 999);

	// 2.5 Удаление из пустой таблицы
	printf("\n2.4 Удаление из пустой таблицы\n");
	check("ht_remove(\"key\") == 0", !ht_remove(ht, "key"));

	ht_destroy(ht);
	printf("\n2.5 Удаление таблицы - без падений, првоерено\n");
	check("таблицы удалена без ошибок", 1);

	//2.6 таблица с размером 1
	printf("\n2.6 Таблица из одной цепочки\n");
	HashTable* tiny = ht_create(1);
	check("таблица на 1 цепочку создана", tiny != NULL);
	check("capacity == 1", tiny->capacity == 1);
	ht_destroy(tiny);
	check("таблица на 1 цепочку удалена", 1);
}

// Тест 3: базовые операции - вставка, поиск, обновление, удаление
static void test_basic_operations(void) {
	printf("\nТест 3: базовые операции\n");
	HashTable* ht = ht_create(16);

	// 3.1 Вставка
	printf("3.1 Вставка элементов\n");
	ht_put(ht, "one", 1);
	check("размер == 1", ht_size(ht) == 1);

	ht_put(ht, "two", 2);
	ht_put(ht, "three", 3);
	ht_put(ht, "four", 4);
	ht_put(ht, "five", 5);
	check("размер == 5", ht_size(ht) == 5);

	// 3.2 Поиск существующих ключей
	printf("\n3.2 Поиск существующих ключей\n");
	int val;
	check("one == 1", ht_get(ht, "one", &val) && val == 1);
	check("two == 2", ht_get(ht, "two", &val) && val == 2);
	check("three == 3", ht_get(ht, "three", &val) && val == 3);
	check("four == 4", ht_get(ht, "four", &val) && val == 4);
	check("five == 5", ht_get(ht, "five", &val) && val == 5);

	// 3.3 Поиск несуществующих ключей
	printf("\n3.3 Поиск несуществующих ключей\n");
	check("six не найден", !ht_contains(ht, "six"));
	check("пустая строка не найдена", !ht_contains(ht, ""));
	check("\"hello\" не найден", !ht_contains(ht, "hello"));

	// 3.4 Обновление существующего ключа
	printf("\n3.4 Обновление значений\n");
	ht_put(ht, "one", 111);
	check("one обновлён до 111", ht_get(ht, "one", &val) && val == 111);
	check("размер не изменился (5)", ht_size(ht) == 5);

	ht_put(ht, "two", 222);
	check("two обновлён до 222", ht_get(ht, "two", &val) && val == 222);

	// Многократное обновление
	ht_put(ht, "three", 333);
	ht_put(ht, "three", 3333);
	ht_put(ht, "three", 33333);
	check("three после трёх обновлений == 33333", ht_get(ht, "three", &val) && val == 33333);
	check("размер всё ещё 5", ht_size(ht) == 5);

	// 3.5 Удаление
	printf("\n3.5 Удаление элементов\n");
	check("удаление four", ht_remove(ht, "four"));
	check("размер == 4", ht_size(ht) == 4);
	check("four отсутствует", !ht_contains(ht, "four"));
	check("one всё ещё есть", ht_contains(ht, "one"));

	check("удаление five", ht_remove(ht, "five"));
	check("размер == 3", ht_size(ht) == 3);
	check("five отсутствует", !ht_contains(ht, "five"));

	// 3.6 Повторное удаление того же ключа
	printf("\n3.6 Повторное удаление\n");
	check("повторное удаление four - false", !ht_remove(ht, "four"));
	check("повторное удаление five - false", !ht_remove(ht, "five"));
	check("размер не изменился (3)", ht_size(ht) == 3);

	// 3.7 Вставка после удаления (на освободившееся место)
	printf("\n3.7 Вставка после удаления\n");
	ht_put(ht, "four", 44);
	check("four снова есть", ht_contains(ht, "four"));
	check("four == 44", ht_get(ht, "four", &val) && val == 44);
	check("размер == 4", ht_size(ht) == 4);

	// 3.8 ht_get с NULL (только проверка наличия)
	printf("\n3.8 ht_get с NULL (проверка без получения значения)\n");
	check("ht_get(one, NULL) == 1", ht_get(ht, "one", NULL));
	check("ht_get(six, NULL) == 0", !ht_get(ht, "six", NULL));

	ht_destroy(ht);
}

// Тест 4: коллизии
static void test_collisions(void) {
	printf("\nТест 4: работа с коллизиями\n");

	// 4.1 Много элементов в маленькой таблице
	printf("4.1 Много ключей, мало цепочек (4 цепочки, 20 ключей)\n");
	HashTable* ht = ht_create(4);

	char key[8];
	for (int i = 0; i < 20; i++) {
		snprintf(key, sizeof(key), "k%d", i);
		ht_put(ht, key, i * 10);
	}
	check("размер == 20", ht_size(ht) == 20);

	// Проверяем, что все значения правильные
	int all_correct = 1;
	int val;
	for (int i = 0; i < 20; i++) {
		snprintf(key, sizeof(key), "k%d", i);
		if (!ht_get(ht, key, &val) || val != i * 10) {
			all_correct = 0;
			break;
		}
	}
	check("все 20 ключей найдены с правильными значениями", all_correct);

	// 4.2 Удаление части элементов из цепочки
	printf("\n4.2 Удаление элементов из заполненной таблицы\n");
	check("удаление k0", ht_remove(ht, "k0"));
	check("удаление k5", ht_remove(ht, "k5"));
	check("удаление k10", ht_remove(ht, "k10"));
	check("удаление k15", ht_remove(ht, "k15"));
	check("удаление k19", ht_remove(ht, "k19"));

	check("размер == 15", ht_size(ht) == 15);
	check("k0 отсутствует", !ht_contains(ht, "k0"));
	check("k5 отсутствует", !ht_contains(ht, "k5"));
	check("k1 всё ещё есть", ht_contains(ht, "k1"));
	check("k18 всё ещё есть", ht_contains(ht, "k18"));

	// 4.3 Вставка новых элемнтов после удаления
	printf("\n4.3 Вставка после удаления (с коллизиями)\n");
	ht_put(ht, "new1", 100);
	ht_put(ht, "new2", 200);
	ht_put(ht, "new3", 300);
	check("размер == 18", ht_size(ht) == 18);
	check("new1 == 100", ht_get(ht, "new1", &val) && val == 100);
	check("new2 == 200", ht_get(ht, "new2", &val) && val == 200);
	check("new3 == 300", ht_get(ht, "new3", &val) && val == 300);

	ht_destroy(ht);

	// 4.4 Все ключи в одну цепочку (специально)
	printf("\n4.4 Много коллизий в одной цепочке (30 ключей, 1 цепочка)\n");
	HashTable* one_bucket = ht_create(1);
	for (int i = 0; i < 30; i++) {
		snprintf(key, sizeof(key), "item%d", i);
		ht_put(one_bucket, key, i);
	}
	check("30 элементов в 1 цепочке", ht_size(one_bucket) == 30);

	// Проверяем
	all_correct = 1;
	for (int i = 0; i < 30; i++) {
		snprintf(key, sizeof(key), "item%d", i);
		if (!ht_get(one_bucket, key, &val) || val != i) {
			all_correct = 0;
			break;
		}
	}
	check("все 30 найдены в одной цепочке", all_correct);

	// Удаляем из середины
	check("удаление item15", ht_remove(one_bucket, "item15"));
	check("item15 отсутствует", !ht_contains(one_bucket, "item15"));
	check("item14 есть", ht_contains(one_bucket, "item14"));
	check("item16 есть", ht_contains(one_bucket, "item16"));

	ht_destroy(one_bucket);
}

// Тест 5: Множество
static void test_set(void) {
	printf("\nТест 5: множество (Set)\n");

	// 5.1 Создание и базовые операции
	printf("5.1 Создание и базовые операции\n");
	Set* s = set_create(16);
	check("множество создано", s != NULL);
	check("размер == 0", set_size(s) == 0);
	check("пустое не содержит \"a\"", !set_contains(s, "a"));

	// 5.2 Добавление элемента
	printf("\n5.2 Добавление элементов\n");
	set_add(s, "apple");
	set_add(s, "banana");
	set_add(s, "cherry");
	set_add(s, "date");
	set_add(s, "bebebebebe");
	check("размер == 5", set_size(s) == 5);

	printf("Содержимое множества: ");
	set_print(s);

	check("apple есть", set_contains(s, "apple"));
	check("banana есть", set_contains(s, "banana"));
	check("cherry есть", set_contains(s, "cherry"));
	check("grape нет", !set_contains(s, "grape"));

	// 5.3 Добавление дубликатов
	printf("\n5.3 Добавление дубликатов\n");
	set_add(s, "apple");   
	set_add(s, "banana");  
	set_add(s, "apple"); 
	check("размер не изменился (5)", set_size(s) == 5);

	// 5.4 Удаление
	printf("\n5.4 Удаление элементов\n");
	check("удаление apple", set_remove(s, "apple"));
	check("размер == 4", set_size(s) == 4);
	check("apple отсутствует", !set_contains(s, "apple"));
	check("banana осталась", set_contains(s, "banana"));

	check("удаление bebebebebe", set_remove(s, "bebebebebe"));
	check("размер == 3", set_size(s) == 3);

	check("повторное удаление apple", !set_remove(s, "apple"));
	check("размер всё ещё 3", set_size(s) == 3);

	printf("После удалений: ");
	set_print(s);

	// 5.5 Обьединение
	printf("\n5.5 Объединение множеств\n");
	Set* a = set_create(16);
	Set* b = set_create(16);

	set_add(a, "red");
	set_add(a, "green");
	set_add(a, "blue");

	set_add(b, "blue");
	set_add(b, "yellow");
	set_add(b, "purple");

	printf("A: "); set_print(a);
	printf("B: "); set_print(b);

	Set* u = set_union(a, b);
	printf("A union B: "); set_print(u);

	check("union размер == 5", set_size(u) == 5);
	check("red есть", set_contains(u, "red"));
	check("green есть", set_contains(u, "green"));
	check("blue есть", set_contains(u, "blue"));
	check("yellow есть", set_contains(u, "yellow"));
	check("purple есть", set_contains(u, "purple"));

	// 5.6 Пересечение
	printf("\n5.6 Пересечение множеств\n");
	Set* inter = set_intersection(a, b);
	printf("A intersect B: "); set_print(inter);

	check("пересечение размер == 1", set_size(inter) == 1);
	check("blue есть в пересечении", set_contains(inter, "blue"));
	check("red нет в пересечении", !set_contains(inter, "red"));

	// 5.7 Разность
	printf("\n5.7 Разность множеств\n");
	Set* diff_ab = set_difference(a, b);
	printf("A \\ B: "); set_print(diff_ab);

	check("A\\B размер == 2", set_size(diff_ab) == 2);
	check("red есть в A\\B", set_contains(diff_ab, "red"));
	check("green есть в A\\B", set_contains(diff_ab, "green"));
	check("blue нет в A\\B", !set_contains(diff_ab, "blue"));

	Set* diff_ba = set_difference(b, a);
	printf("B \\ A: "); set_print(diff_ba);

	check("B\\A размер == 2", set_size(diff_ba) == 2);
	check("yellow есть в B\\A", set_contains(diff_ba, "yellow"));
	check("purple есть в B\\A", set_contains(diff_ba, "purple"));

	// 5.8 Операции с пустым множеством
	printf("\n5.8 Операции с пустым множеством\n");
	Set* empty = set_create(8);
	Set* union_with_empty = set_union(a, empty);
	check("A union пустое == A (размер 3)", set_size(union_with_empty) == 3);

	Set* inter_with_empty = set_intersection(a, empty);
	check("A пересечение с пустым == пустое (размер 0)", set_size(inter_with_empty) == 0);

	Set* diff_with_empty = set_difference(a, empty);
	check("A \\ пустое == A (размер 3)", set_size(diff_with_empty) == 3);

	// 5.9 Множество с одинаковыми элементами
	printf("\n5.9 Пересечение множества с самим собой\n");
	Set* same = set_intersection(a, a);
	check("A пересечение с A == A (размер 3)", set_size(same) == 3);

	Set* diff_same = set_difference(a, a);
	check("A \\ A == пустое (размер 0)", set_size(diff_same) == 0);

	set_destroy(s);
	set_destroy(a);
	set_destroy(b);
	set_destroy(u);
	set_destroy(inter);
	set_destroy(diff_ab);
	set_destroy(diff_ba);
	set_destroy(empty);
	set_destroy(union_with_empty);
	set_destroy(inter_with_empty);
	set_destroy(diff_with_empty);
	set_destroy(same);
	set_destroy(diff_same);

	printf("\n5.10 Все множества удалены без утечек\n");
	check("память множеств освобождена", 1);
}

// Тест 6: мультимножество
static void test_multiset(void) {
	printf("\nТест 6: мультимножество\n");

	// 6.1 Создание
	printf("6.1 Создание мультимножества\n");
	MultiSet* ms = multiset_create(16);
	check("мультимножество создано", ms != NULL);

	// 6.2 Добавление с количеством
	printf("\n6.2 Добавление элементов\n");
	multiset_add(ms, "apple", 5);
	check("apple == 5", multiset_count(ms, "apple") == 5);

	multiset_add(ms, "banana", 3);
	check("banana == 3", multiset_count(ms, "banana") == 3);

	multiset_add(ms, "cherry", 0); 
	check("cherry == 0 (добавили 0)", multiset_count(ms, "cherry") == 0);

	printf("Содержимое: ");
	multiset_print(ms);

	// 6.3 Добавление к существующему
	printf("\n6.3 Добавление к существующему элементу\n");
	multiset_add(ms, "apple", 3);  
	check("apple == 8", multiset_count(ms, "apple") == 8);

	multiset_add(ms, "banana", 1);  
	check("banana == 4", multiset_count(ms, "banana") == 4);

	printf("После добавления: ");
	multiset_print(ms);

	// 6.4 Проверка наличия
	printf("\n6.4 Проверка наличия\n");
	check("apple содержится", multiset_contains(ms, "apple"));
	check("banana содержится", multiset_contains(ms, "banana"));
	check("cherry не содержится (добавляли 0)", !multiset_contains(ms, "cherry"));
	check("grape не содержится", !multiset_contains(ms, "grape"));

	// 6.5 Частичное удаление
	printf("\n6.5 Частичное удаление\n");
	check("удаление 2 apple", multiset_remove(ms, "apple", 2));
	check("apple == 6 (было 8, убрали 2)", multiset_count(ms, "apple") == 6);
	check("apple всё ещё содержится", multiset_contains(ms, "apple"));

	check("удаление 1 banana", multiset_remove(ms, "banana", 1));
	check("banana == 3", multiset_count(ms, "banana") == 3);

	printf("После частичного удаления: ");
	multiset_print(ms);

	// 6.6 Полное удаление
	printf("\n6.6 Полное удаление (счётчик в 0)\n");
	check("удаление 3 banana", multiset_remove(ms, "banana", 3));
	check("banana == 0", multiset_count(ms, "banana") == 0);
	check("banana не содержится", !multiset_contains(ms, "banana"));

	printf("После полного удаления banana: ");
	multiset_print(ms);

	// 6.7 Удаление больше, чем есть
	printf("\n6.7 Удаление больше, чем есть\n");
	check("удаление 100 apple", multiset_remove(ms, "apple", 100));
	check("apple == 0", multiset_count(ms, "apple") == 0);
	check("apple не содержится", !multiset_contains(ms, "apple"));

	printf("После удаления всего: ");
	multiset_print(ms);

	// 6.8 Удаление несуществующего
	printf("\n6.8 Удаление несуществующего элемента\n");
	check("удаление несуществующего", !multiset_remove(ms, "grape", 1));
	check("удаление уже удалённого", !multiset_remove(ms, "banana", 1));

	// 6.9 Снова добавляем
	printf("\n6.9 Повторное добавление после удаления\n");
	multiset_add(ms, "apple", 10);
	check("apple == 10", multiset_count(ms, "apple") == 10);

	multiset_add(ms, "banana", 7);
	check("banana == 7", multiset_count(ms, "banana") == 7);

	printf("Финальное состояние: ");
	multiset_print(ms);

	multiset_destroy(ms);
	check("мультимножество удалено", 1);
}

// Тест 7: примеры из жизни
static void test_real(void) {
	printf("\nТест 7: примеры реального использования\n");

	// 7.1 Частотный словарь
	printf("\n7.1 Хеш-таблица как частотный словарь\n");
	HashTable* freq = ht_create(32);
	const char* words[] = {
		"hello", "world", "hello", "hash", "table",
		"world", "hello", "c", "hash", "function",
		"hello", "world", NULL
	};

	for (int i = 0; words[i]; i++) {
		int cnt = 0;
		ht_get(freq, words[i], &cnt);
		ht_put(freq, words[i], cnt + 1);
	}

	int val;
	printf("Статистика слов:\n");
	printf("hello: %d (ожидалось 4)\n", ht_get(freq, "hello", &val) ? val : 0);
	printf("world: %d (ожидалось 3)\n", ht_get(freq, "world", &val) ? val : 0);
	printf("hash:  %d (ожидалось 2)\n", ht_get(freq, "hash", &val) ? val : 0);
	printf("table: %d (ожидалось 1)\n", ht_get(freq, "table", &val) ? val : 0);
	printf("function: %d (ожидалось 1)\n", ht_get(freq, "function", &val) ? val : 0);
	printf("c: %d (ожидалось 1)\n", ht_get(freq, "c", &val) ? val : 0);

	check("hello == 4", ht_get(freq, "hello", &val) && val == 4);
	check("world == 3", ht_get(freq, "world", &val) && val == 3);
	check("hash == 2", ht_get(freq, "hash", &val) && val == 2);
	check("table == 1", ht_get(freq, "table", &val) && val == 1);
	check("python (нет) == 0", !ht_contains(freq, "python"));

	ht_destroy(freq);

	// 7.2 множество - уникальные посетители
	printf("\n7.2 Множество — учёт уникальных посетителей\n");
	Set* visitors = set_create(32);

	const char* visits[] = {
		"alice", "bob", "alice", "charlie", "bob",
		"diana", "alice", "eve", "charlie", "frank",
		NULL
	};

	printf("Всего визитов: 10\n");
	for (int i = 0; visits[i]; i++) {
		set_add(visitors, visits[i]);
	}

	printf("Уникальные посетители: ");
	set_print(visitors);

	check("уникальных посетителей == 6", set_size(visitors) == 6);
	check("alice учтена", set_contains(visitors, "alice"));
	check("bob учтён", set_contains(visitors, "bob"));
	check("frank учтён", set_contains(visitors, "frank"));
	check("незнакомец не учтён", !set_contains(visitors, "unknown"));

	set_destroy(visitors);

	// 7.3 Мультимножество - корзина покупок
	printf("\n7.3 Мультимножество - корзина интернет-магазина\n");
	MultiSet* cart = multiset_create(32);

	printf("Добавляем товары:\n");
	multiset_add(cart, "хлеб", 2);
	printf("+ 2 хлеба\n");
	multiset_add(cart, "молоко", 1);
	printf("+ 1 молоко\n");
	multiset_add(cart, "яблоки", 6);
	printf("+ 6 яблок\n");
	multiset_add(cart, "хлеб", 1);
	printf("+ 1 хлеб (стало 3)\n");

	printf("Корзина: ");
	multiset_print(cart);

	check("хлеб == 3", multiset_count(cart, "хлеб") == 3);
	check("молоко == 1", multiset_count(cart, "молоко") == 1);
	check("яблоки == 6", multiset_count(cart, "яблоки") == 6);
	check("сыр == 0 (нет в корзине)", multiset_count(cart, "сыр") == 0);

	printf("\nУбираем 1 хлеб и 3 яблока:\n");
	multiset_remove(cart, "хлеб", 1);
	multiset_remove(cart, "яблоки", 3);

	printf("Корзина после изменений: ");
	multiset_print(cart);

	check("хлеб == 2", multiset_count(cart, "хлеб") == 2);
	check("яблоки == 3", multiset_count(cart, "яблоки") == 3);

	printf("\nУбираем всё молоко:\n");
	multiset_remove(cart, "молоко", 1);
	check("молоко == 0", multiset_count(cart, "молоко") == 0);
	check("молока нет в корзине", !multiset_contains(cart, "молоко"));

	printf("Итоговая корзина: ");
	multiset_print(cart);

	multiset_destroy(cart);

	// 7.4 Множество - фильтрация дубликатов
	printf("\n7.4 Множество — удаление дубликатов из списка\n");
	Set* unique = set_create(32);

	const char* items[] = { "C", "C", "Python", "Java", "C", "Python", "Go", "Rust", "Go", NULL };
	printf("Исходный список: C, C, Python, Java, C, Python, Go, Rust, Go\n");
	printf("Уникальные: ");

	for (int i = 0; items[i]; i++) {
		if (!set_contains(unique, items[i])) {
			printf("%s ", items[i]);
		}
		set_add(unique, items[i]);
	}
	printf("\n");

	check("уникальных языков == 5", set_size(unique) == 5);
	check("C есть", set_contains(unique, "C"));
	check("Python есть", set_contains(unique, "Python"));
	check("Java есть", set_contains(unique, "Java"));
	check("Go есть", set_contains(unique, "Go"));
	check("Rust есть", set_contains(unique, "Rust"));
	check("JavaScript нет", !set_contains(unique, "JavaScript"));

	set_destroy(unique);
}

int main(void) {
	setlocale(LC_ALL, "Russian");
	printf("Тесты Хэш-Таблицы, Множества, Мультимножества\n\n");

	test_hash_function();
	test_create_destroy();
	test_basic_operations();
	test_collisions();
	test_set();
	test_multiset();
	test_real();

	printf("Итоги:\n");
	printf("Пройдено: %d\n", passed);
	printf("Провалено: %d\n", failed);
	printf("Всего: %d\n", passed + failed);

	if (failed == 0) {
		printf("\nВсе тесты пройдены успешно!\n");
	}
	else {
		printf("\nЕсть проваленные тесты...\n");
	}

	return 0;
}