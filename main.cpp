//please define POISON and FREE (not equal)
const int POISON = 2281337;
const int FREE   = 228228228;
typedef int Type_t;
#define LOG
#include "list/list.h"

int main() {
    List list = {0};
    list_init(list);
    
    list_resize(&list, 5);

    list_push_back(&list, 0);
    
    list_push_back(&list, 1);

    list_push_back(&list, 0);

    list_erace_index(&list, 2);

    list_linerization(&list);

    list_resize(&list, 3);

    list_pop_front(&list);

    list_dec(list);
}