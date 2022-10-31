#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

const char *log_file_path = "log/log.html";

int first_img_file_num = 0;

const size_t MAX_COMMAND_SIZE = 30;

enum List_errors {
    INDEX_IS_TOO_LARGE    = 1,
    PUSH_AFTER_FREE_INDEX = 2,
    POP_ZERO_ELEMENT      = 4,
};

struct List {
    Type_t *data;
    size_t *left;
    size_t *right;

    size_t size;
    size_t capacity;

    size_t first_free;

    bool liner;
};

#define MIN(a, b) (((a) < (b) )? (a):(b))

#define MAX(a, b) (((a) > (b) )? (a):(b))

#define list_init(list) _list_init((&(list)))

#define list_dec(list) _list_dex((&(list)))



#ifdef LOG

#define check_list(list) _check_list(list, __PRETTY_FUNCTION__, __FILE__, __LINE__)

#define warning(list, message) _log_warning(list, message, __PRETTY_FUNCTION__, __FILE__, __LINE__)

#define info(list, message)    _log_info(list, message, __PRETTY_FUNCTION__, __FILE__, __LINE__)

#define error(list, message)   _log_error(list, message, __PRETTY_FUNCTION__, __FILE__, __LINE__)

void dump(List *list); //only for Type_t == int

void _check_list(List *list, const char *func, const char *file, const size_t line);

#else 

#define check_list(list)    assert(list != nullptr);       \
                            assert(list->data != nullptr); \
                            assert(list->left != nullptr); \
                            assert(list->right != nullptr);

#define warning(message)

#define info(message)

#define error(message)

#endif

int list_push_front(List *list, Type_t a);

int list_push_back(List *list, Type_t a);

void _list_init(List *list);

void _list_dec(List *list);

int list_resize(List *list, size_t new_size);

int list_push_index(List *list, Type_t a, size_t index);

size_t list_num_to_index(List *list, size_t num);

size_t list_previous(List *list, size_t num);

size_t list_next(List *list, size_t num);

int list_pop_front(List *list);

int list_pop_back(List *list);

int list_pop_index(List *list, size_t num);

int list_linerization(List *list);

#ifdef LOG

void _log_info(List *list, const char *message, const char *func, const char *file, const size_t line);

void _log_error(List *list, const char *message, const char *func, const char *file, const size_t line);

void _log_warning(List *list, const char *message, const char *func, const char *file, const size_t line);

void _log_warning(List *list, const char *message, const char *func, const char *file, const size_t line) {
    assert(message != nullptr);

    assert(log_file_path != nullptr);
    FILE *file_ptr = fopen(log_file_path, "a");
    assert(file_ptr != nullptr);

    fprintf(file_ptr, "<font color=\"orange\">warning:\"%s\" in file:%s in func:%s in line:%zu</font>\n\n", message, file, func, line);

    fclose(file_ptr);

    dump(list);
}

void _log_info(List *list, const char *message, const char *func, const char *file, const size_t line) {
    assert(message != nullptr);

    assert(log_file_path != nullptr);
    FILE *file_ptr = fopen(log_file_path, "a");
    assert(file_ptr != nullptr);

    fprintf(file_ptr, "<font color=\"green\">info:\"%s\" in file:%s in func:%s in line:%zu</font>\n\n", message, file, func, line);

    fclose(file_ptr);

    dump(list);
}

void _log_error(List *list, const char *message, const char *func, const char *file, const size_t line) {
    assert(message != nullptr);

    assert(log_file_path != nullptr);
    FILE *file_ptr = fopen(log_file_path, "a");
    assert(file_ptr != nullptr);

    fprintf(file_ptr, "<font color=\"red\">info:\"%s\" in file:%s in func:%s in line:%zu</font>\n\n", message, file, func, line);

    fclose(file_ptr);

    dump(list);
}

#endif

int list_linerization(List *list) {
    check_list(list);
    info(list, "linerization start");

    Type_t *new_data  = (Type_t *)calloc(list->capacity, sizeof(Type_t));

    size_t     index = 0;
    size_t new_index = 0;

    do {
        new_data[new_index] = list->data[index];

        index = list->right[index];
        new_index++;
    } while(index!=0);

    free(list->data);
    list->data = new_data;

    list->first_free = list->size + 1;

    for (size_t i = 0; i < list->capacity; i++) {
        list->right[i] = i + 1;
    }

    for (size_t i = 1; i < list->first_free; i++) {
        list->left[i] = i - 1;
    }

    for (size_t i = list->first_free; i < list->capacity; i++) {
        list->left[i] = 0;
        list->data[i] = FREE;
    }

    list->right[list->capacity - 1] = 0;

    list->left[0] = list->first_free - 1;
    list->right[    list->first_free - 1] = 0;

    check_list(list);
    info(list, "linerization end");

    list->liner = true;

    return 0;
}


int list_pop_index(List *list, size_t num) {
    check_list(list);
    info(list, "start pop index");

    if (num != list->left[0] && num != list->right[0]) {
        list->liner = false;
    }

    if (num == 0) {
        return POP_ZERO_ELEMENT;
    }

    if (list->data[num] == FREE) {
        warning(list, "maybe pop free");
    }

    list->right[list->left[num]] = list->right[num];
    list->left[list->right[num]] = list->left[num];

    list->data[num]  = FREE;
    list->left[num]  = 0;
    list->right[num] = list->first_free;

    list->first_free = num;

    list->size--;

    check_list(list);
    info(list, "end pop index");

    return 0;
}

int list_resize(List *list, size_t new_size) {
    check_list(list);
    info(list, "start resize");

    {
        size_t index = 0;

        do {
            new_size = MAX(new_size, index + 1);
            index = list->right[index];
        } while (index);
    }

    Type_t *new_data  = (Type_t *)calloc(sizeof(Type_t), new_size);
    size_t *new_left  = (size_t *)calloc(sizeof(size_t), new_size);
    size_t *new_right = (size_t *)calloc(sizeof(size_t), new_size);
    assert(new_data  != nullptr);
    assert(new_left  != nullptr);
    assert(new_right != nullptr);

    {
        while(list->first_free >= new_size) {
            list->first_free = list->right[list->first_free];
        }

        size_t free_index = list->first_free;

        while (free_index) {
            while (list->right[free_index] >= new_size) {
                list->right[free_index] = list->right[list->right[free_index]];
            }
            free_index = list->right[free_index];
        }
    }

    for (size_t i = 0; i < MIN(list->capacity, new_size); i++) {
        new_data[i]  = list->data[i];
        new_left[i]  = list->left[i];
        new_right[i] = list->right[i];
    }

    free(list->data);
    free(list->left);
    free(list->right);

    list->data  = new_data;
    list->left  = new_left;
    list->right = new_right;

    for (size_t i = MIN(list->capacity, new_size); i < new_size; i++) {
        list->data[i]  = FREE;
        list->left[i]  = 0;
        list->right[i] = (i + 1) % new_size;
    }

    if (list->first_free == 0) {
        list->first_free = MIN(list->capacity, new_size) % new_size;
    }
    else{
        size_t free_index = list->first_free;
        while (list->right[free_index]) {
            free_index = list->right[free_index];
        }
        list->right[free_index] = MIN(list->capacity, new_size) % new_size;
    }

    list->capacity = new_size;

    check_list(list);
    info(list, "end resize");

    return 0;
}

int list_push_index(List *list, Type_t a, size_t index) {
    check_list(list);
    info(list, "start push index");

    if (index != list->left[0]) {
        list->liner = false;
    }

    if (list->first_free == 0) {
        list_resize(list, list->capacity * 2);
    }

    if (index >= list->capacity) {
        return INDEX_IS_TOO_LARGE;
    }

    if (list->data[index] == FREE) {
        return PUSH_AFTER_FREE_INDEX;
    }

    size_t first_free = list->first_free;
    list->first_free  = list->right[first_free];

    if (first_free != index + 1) {
        list->liner = false;
    }

    list->data[ first_free] = a;
    list->left[ first_free] = index;
    list->right[first_free] = list->right[index];

    list->right[index] = first_free;

    list->left[list->right[first_free]] = first_free;

    list->size++;

    check_list(list);
    info(list, "end pop index");

    return 0;
}

void _list_init(List *list) {
    assert(list != nullptr);
#ifdef LOG
    FILE *file = fopen(log_file_path, "w");
    assert(file != nullptr);
    fprintf(file, "<pre>\n\n");
    fclose(file);
#endif
    list->data = (Type_t *)calloc(sizeof(Type_t), 1);
    assert(list->data != nullptr);
    list->data[0] = POISON;

    list->left = (size_t *)calloc(sizeof(size_t), 1);
    assert(list->left != nullptr);
    list->left[0] = 0;

    list->right = (size_t *)calloc(sizeof(size_t), 1);
    assert(list->right != nullptr);
    list->right[0] = 0;

    list->size = 0;
    list->capacity = 1;

    list->first_free = 0;

    list->liner = true;
}

void _list_dec(List *list) {
    check_list(list);

    free(list->data);
    free(list->left);
    free(list->right);

    list->capacity = 0;
    list->size     = 0;
    list->liner    = false;
}

int list_push_back(List *list, Type_t a) {
    check_list(list);
    info(list, "start push back");

    return list_push_index(list, a, list->left[0]);
}

int list_push_front(List *list, Type_t a) {
    check_list(list);
    info(list, "start push front");

    return list_push_index(list, a, 0);    
}

int list_pop_back(List *list) {
    check_list(list);
    info(list, "start pop back");

    return list_pop_index(list, list->left[0]);
}

int list_pop_front(List *list) {
    check_list(list);
    info(list, "start pop front");

    return list_pop_index(list, list->right[0]);
}

size_t list_num_to_index(List *list, size_t num) {
    check_list(list);

    if (list->liner) {
        return list->right[0] + num;
    }

    size_t index = list->right[0];
    for (size_t i = 0; i < num; i++) {
        index = list->left[index];
    }
    return index;
}

size_t list_next(List *list, size_t num) {
    check_list(list);

    if (num >= list->capacity) {
        return 0;
    }

    return list->right[num];
}

size_t list_previous(List *list, size_t num) {
    check_list(list);

    if (num >= list->capacity) {
        return 0;
    }

    return list->left[num];
}

#ifdef LOG

void _check_list(List *list, const char *func, const char *file, const size_t line) {
    if (list == nullptr) {
        _log_error(list, "list ptr = nullptr", func, file, line);
    }

    if (list->left == nullptr) {
        _log_error(list, "list.left ptr = nullptr", func, file, line);
    }

    if (list->right == nullptr) {
        _log_error(list, "list.right ptr = nullptr", func, file, line);
    }

    if (list->data == nullptr) {
        _log_error(list, "list.data ptr = nullptr", func, file, line);
    }

    if (list->size >= list->capacity) {
        _log_error(list, "list.size too large", func, file, line);
    }

    for (size_t i = 0; i < list->capacity; i++) {
        if (list->right[i] >= list->capacity) {
            _log_error(list, "list.right element is too large", func, file, line);
        }
        if (list->left[i] >= list->capacity) {
            _log_error(list, "list.left element is too large", func, file, line);
        }
    }

    bool *usage = (bool *)calloc(list->capacity, sizeof(bool));

    size_t index = 0;

    for (size_t i = 0; i <= list->size; i++) {
        if (usage[index]) {
            _log_error(list, "syckl in vertexes", func, file, line);
        }
        usage[index] = true;

        index = list->right[index];
    }

    if (index) {
        _log_error(list, "vertices are larger than the size", func, file, line);
    }

    size_t free_itr = list->first_free;

    if (free_itr != 0) {
        for (size_t i = 0; i < list->capacity - list->size - 1; i++) {
            if (usage[free_itr]) {
                _log_error(list, "syckl in free vertexes or free vertex refer to filled vertexes", func, file, line);
            }
            usage[free_itr] = true;

            if (list->data[free_itr] != FREE) {
                _log_error(list, "data free element != FREE const", func, file, line);
            }

            if (list->left[free_itr] != 0) {
                _log_error(list, "list.left != 0 in free element", func, file, line);
            }

            free_itr = list->right[free_itr];
        }

        if (free_itr) {
            _log_error(list, "free vertices are larger than the size", func, file, line);
        }
    }
    else {
        if (list->size != list->capacity - 1) {
            _log_error(list, "list.first_free == 0 bat size == capacity", func, file, line);
        }
    }

    free(usage);
}

#define GEN_SVC

#define ADD_SVC_TO_LOG_HTML

#define ADD_PTR_TO_FIRST_FREE

void dump(List *list) {
    assert(list        != nullptr);

    assert(list->data  != nullptr);
    assert(list->left  != nullptr);
    assert(list->right != nullptr);

    GEN_SVC {
        FILE *file = fopen("log/input.dot", "w");

        fprintf(file, "digraph G {rankdir=LR;style=filled;graph [splines = headport splines=ortho];\n");
        for (size_t i = 0; i < list->capacity; i++) {
            if (list->data[i] == FREE) {
                fprintf(file, "VERTEX%zu[label=\"%zu | free | l = %zu | r = %zu\", shape=\"Mrecord\", style = filled, fillcolor = \"#c0ffee\"]\n",\
                                     i,          i,        list->left[i], list->right[i]);
            }
            else if (list->data[i] == POISON) {
                fprintf(file, "VERTEX%zu[label=\"%zu | poison | l = %zu | r = %zu\", shape=\"Mrecord\"]\n",\
                                     i,          i,        list->left[i], list->right[i]);
            }
            else{
                fprintf(file, "VERTEX%zu[label=\"%zu | data = %d | l = %zu | r = %zu\", shape=\"Mrecord\", style = filled, fillcolor = \"#decade\"]\n",\
                                     i,          i, list->data[i], list->left[i], list->right[i]);
            }
            
        }
        for (size_t i = 0; i < list->capacity - 1; i++) {
            fprintf(file, "VERTEX%zu->VERTEX%zu[style=\"invis\", weight = 100]\n", i, i + 1);
        }
        for (size_t i = 0; i < list->capacity; i++) {
            fprintf(file, "VERTEX%zu->VERTEX%zu[color=\"green\"]\n", i, list->right[i]);
        }
        for (size_t i = 0; i < list->capacity; i++ ) {
            fprintf(file, "VERTEX%zu->VERTEX%zu[color=\"red\"]\n", i, list->left[i]);
        }

        fprintf(file, "VERTEX_FREE[lable = \"ptr_free\", shape=\"Mrecord\", style = filled, fillcolor = \"#f1ee00\"]\n");

        fprintf(file, "VERTEX_FREE->VERTEX%zu[color=\"green\"]\n", list->first_free);

        fprintf(file, "}");

        fclose(file);
    }

    ADD_SVC_TO_LOG_HTML {
        char *command = (char *)calloc(sizeof(char), MAX_COMMAND_SIZE);
        assert(command != nullptr);

        sprintf(command, "dot -Tsvg log/input.dot > log/img%d.svg", first_img_file_num);

        system(command);

        free(command);

        FILE *file = fopen(log_file_path, "a");
        assert(file != nullptr);

        fprintf(file, "<img src=\"img%d.svg\" height=\"300px\" width=\"400px\"/>\n\n", first_img_file_num);

        fclose(file);

        first_img_file_num++;
    }
}

#undef GEN_SVC

#undef ADD_SVC_TO_LOG_HTML

#endif