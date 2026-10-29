
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_WORDS 8

// структура для столбцов сложения
typedef struct
{
    int letter[MAX_WORDS];
    int order[7];
} Column;

static const char *words[MAX_WORDS];
static size_t lengths[MAX_WORDS], column_count;
static int letters[10], letter_count, term_count;
static int digit[26], used[10], leading[26];

static Column *columns;

static int parse(const char *text)
{
    int count = 0, after_equal = 0;
    const char *p = text;
    letter_count = 0;
    column_count = 0;
    memset(used, 0, sizeof used);
    memset(leading, 0, sizeof leading);
    for (int i = 0; i < 26; i++)
        digit[i] = -1;

    while (1)
    {
        while (isspace((unsigned char)*p))
            p++;
        if (count == MAX_WORDS || *p < 'A' || *p > 'Z')
            return 0;
        words[count] = p;

        while (*p >= 'A' && *p <= 'Z')
        {
            int id = *p - 'A';
            int found = 0;
            for (int i = 0; i < letter_count; i++)
            {
                if (letters[i] == id)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                if (letter_count == 10)
                    return 0;
                letters[letter_count++] = id;
            }
            p++;
        }
        lengths[count] = (size_t)(p - words[count]);
        if (lengths[count] > 1)
            leading[words[count][0] - 'A'] = 1;
        if (lengths[count] > column_count)
            column_count = lengths[count];
        count++;
        while (isspace((unsigned char)*p))
            p++;
        if (after_equal)
            return *p == '\0';
        if (*p == '+' && count < 7)
        {
            p++;
        }
        else if (*p == '=' && count >= 2)
        {
            term_count = count;
            after_equal = 1;
            p++;
        }
        else
        {
            return 0;
        }
    }
}

// функция подготовки столбцов
static int prepare_columns(void)
{
    if (column_count > SIZE_MAX / sizeof *columns)
        return 0;
    columns = (Column *)malloc(column_count * sizeof *columns);
    if (columns == NULL)
        return 0;
    for (size_t col = 0; col < column_count; col++)
    {
        for (int row = 0; row <= term_count; row++)
        {
            columns[col].letter[row] = col < lengths[row]
                                           ? words[row][lengths[row] - 1 - col] - 'A'
                                           : -1;
        }
        for (int row = 0; row < term_count; row++)
            columns[col].order[row] = row;
    }
    return 1;
}

static int check_sum(void)
{
    int carry = 0;
    for (size_t col = 0; col < column_count; col++)
    {
        int sum = carry;
        for (int row = 0; row < term_count; row++)
        {
            // чтение готовых индексов букв
            int id = columns[col].letter[row];
            if (id != -1)
            {
                sum += digit[id];
            }
        }

        int result_id = columns[col].letter[term_count];
        int expected_digit = result_id != -1 ? digit[result_id] : 0;

        if (sum % 10 != expected_digit)
            return 0;
        carry = sum / 10;
    }
    return carry == 0;
}

static int search(int index)
{
    if (index == letter_count)
    {
        return check_sum();
    }
    int id = letters[index];
    for (int d = 0; d <= 9; d++)
    {
        if (used[d] || (d == 0 && leading[id]))
            continue;
        digit[id] = d;
        used[d] = 1;
        if (search(index + 1))
            return 1;
        used[d] = 0;
        digit[id] = -1;
    }
    return 0;
}

char *solve_rebus(const char *text)
{
    if (text == NULL || !parse(text))
        return NULL;
    // вызов перед перебором для ускорения
    if (!prepare_columns())
        return NULL;

    int found = search(0);

    free(columns);
    columns = NULL;

    if (!found)
        return NULL;
    size_t length = strlen(text);
    if (length == SIZE_MAX)
        return NULL;
    char *result = (char *)malloc(length + 1);
    if (result == NULL)
        return NULL;
    for (size_t i = 0; i <= length; i++)
    {
        char c = text[i];
        result[i] = c >= 'A' && c <= 'Z' ? (char)('0' + digit[c - 'A']) : c;
    }
    return result;
}

static char *read_line(void)
{
    size_t size = 100, length = 0;
    char *text = (char *)malloc(size);
    if (text == NULL)
        return NULL;
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        if (length == size - 1)
        {
            if (size > SIZE_MAX / 2)
            {
                free(text);
                return NULL;
            }
            char *larger = (char *)realloc(text, size * 2);
            if (larger == NULL)
            {
                free(text);
                return NULL;
            }
            text = larger;
            size *= 2;
        }
        text[length++] = (char)c;
    }
    if (ferror(stdin) || (c == EOF && length == 0))
    {
        free(text);
        return NULL;
    }
    text[length] = '\0';
    return text;
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    printf("Введите ребус (например, SEND + MORE = MONEY):\n");
    char *text = read_line();
    if (text == NULL)
    {
        printf("Не удалось прочитать строку или выделить память.\n");
        return 1;
    }
    char *result = solve_rebus(text);
    free(text);
    if (result == NULL)
    {
        printf("Нет решения, неверная запись ребуса или недостаточно памяти.\n");
        return 1;
    }
    printf("Решение: %s\n", result);
    free(result);
    return 0;
}