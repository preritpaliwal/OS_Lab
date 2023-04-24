#include <iostream>
#include<stdlib.h>
#include<string>
#include<vector>
#include<map>
#include<queue>
#include<stack>
#include<math.h>
#include<utility>
#include<unordered_map>
#include<unordered_set>

using namespace std;

int main(){
    map <pair<int, string>, unsigned long> myMap;

    myMap[{1, "hi"}] = 3;
    myMap[{2, "hello"}] = 5;

    cout << myMap[{1, "hi"}] << endl;
    cout << myMap[{2, "hello"}] << endl;


    // if an entry does not exist for the same scope and list name, return error
    if (sym_table->list_map.find(make_pair(curr_scope, list_name)) == sym_table->list_map.end()){
        cout << "Error: No entry in the Symbol table for the given list_name and scope!" << endl;
        return;
    }

    unsigned long entry_num = sym_table->list_map[make_pair(curr_scope, list_name)];

    // check if the offset is within the size of the list
    size_t size = sym_table->entries[entry_num].list_pages.size() * PAGE_SIZE;
    if (offset >= size){
        cout << "Error: Offset is out of bounds!" << endl;
        return;
    }

    // find the page number and offset within the page
    size_t page_num = offset/PAGE_SIZE;
    size_t offset_within_page = offset%PAGE_SIZE;

    // find the page number in the page mapping
    size_t page_num_in_mapping = sym_table->entries[entry_num].list_pages[page_num].first;

    // assign the value to the page
    mem->assignVal(page_num_in_mapping, offset_within_page, value);

    return;

    return 0;
}



// struct myComp2 {
//     constexpr bool operator()(
//         pair<unsigned long, unsigned long> const& a,
//         pair<unsigned long, unsigned long> const& b)
//         const noexcept
//     {
//         return a.first > b.first;
//     }
// };


// typedef struct {
//     int val;
//     unsigned long next;
//     unsigned long prev;
// } element;