#include "../include/lexer.h"
#include "../include/get_init_state.h"
#include "../include/search_char.h"
#include "../include/belongs.h"
#include <set>
#include <string>
#include <vector>
#include "../include/operations_with_sets.h"

#include <quadmath>
size_t digit2int(char32_t ch)
{
size_t v = ch - U'0';
return (v<=9)? v:(v&0b1101'1111) - 7;
};
__float128 Lexer::build_float()
{
__float128 num;
num = int_value + fract_part*powq(10, num_of_digits);
num = num*powq(10, sign_of_degree*exponent);
return num;}
Lexer::Automaton_proc Lexer::procs[] = {
    &Lexer::start_proc(),     &Lexer::unknown_proc(),   
    &Lexer::idkeyword_proc(), &Lexer::delimiter_proc(), 
    &Lexer::number_proc(),    &Lexer::string_proc()
};

Lexer::Final_proc Lexer::finals[] = {
    &Lexer::none_proc(),            &Lexer::unknown_final_proc(),   
    &Lexer::idkeyword_final_proc(), &Lexer::delimiter_final_proc(), 
    &Lexer::number_final_proc(),    &Lexer::string_final_proc()
};

enum Category {
    SPACES,          DELIMITER_BEGIN, 
    STRING_BEGIN,    NUMBER0,         
    NUMBER3,         NUMBER4,         
    NUMBER_BEGIN,    NUMBER1,         
    NUMBER2,         NUMBER5,         
    NUMBER6,         NUMBER7,         
    NUMBER8,         NUMBER9,         
    NUMBER10,        NUMBER11,        
    IDKEYWORD_BEGIN, IDKEYWORD0,      
    IDKEYWORD1,      IDKEYWORD2,      
    IDKEYWORD3,      Other
};

static const std::map<char32_t, uint32_t> categories_table = {
    {'\0', 1},       {'\X01', 1},     {'\X02', 1},     {'\X03', 1},     
    {'\X04', 1},     {'\X05', 1},     {'\X06', 1},     {'\a', 1},       
    {'\b', 1},       {'\t', 1},       {'\n', 1},       {'\v', 1},       
    {'\f', 1},       {'\r', 1},       {'\X0e', 1},     {'\X0f', 1},     
    {'\X10', 1},     {'\X11', 1},     {'\X12', 1},     {'\X13', 1},     
    {'\X14', 1},     {'\X15', 1},     {'\X16', 1},     {'\X17', 1},     
    {'\X18', 1},     {'\X19', 1},     {'\X1a', 1},     {'\X1b', 1},     
    {'\X1c', 1},     {'\X1d', 1},     {'\X1e', 1},     {'\X1f', 1},     
    {' ', 1},        {'!', 2},        {", 4},          {'#', 2},        
    {'%', 2},        {'&', 2},        {', 256},        {'(', 2},        
    {')', 2},        {'*', 2},        {'+', 32770},    {',', 2},        
    {'-', 32770},    {'.', 2048},     {'/', 2},        {'0', 525432},   
    {'1', 525552},   {'2', 525536},   {'3', 525536},   {'4', 525536},   
    {'5', 525536},   {'6', 525536},   {'7', 525536},   {'8', 525504},   
    {'9', 525504},   {':', 2},        {';', 2},        {'<', 2},        
    {'=', 2},        {'>', 2},        {'?', 2},        {'A', 721920},   
    {'B', 726016},   {'C', 721920},   {'D', 721920},   {'E', 722432},   
    {'F', 721920},   {'G', 720896},   {'H', 720896},   {'I', 720896},   
    {'J', 720896},   {'K', 720896},   {'L', 720896},   {'M', 720896},   
    {'N', 720896},   {'O', 720896},   {'P', 720896},   {'Q', 720896},   
    {'R', 720896},   {'S', 720896},   {'T', 720896},   {'U', 720896},   
    {'V', 720896},   {'W', 720896},   {'X', 729088},   {'Y', 720896},   
    {'Z', 720896},   {'[', 2},        {']', 2},        {'^', 2},        
    {'_', 720896},   {'a', 721920},   {'b', 726016},   {'c', 721920},   
    {'d', 721920},   {'e', 722432},   {'f', 721920},   {'g', 720896},   
    {'h', 720896},   {'i', 720896},   {'j', 720896},   {'k', 720896},   
    {'l', 720896},   {'m', 720896},   {'n', 720896},   {'o', 737280},   
    {'p', 720896},   {'q', 720896},   {'r', 720896},   {'s', 720896},   
    {'t', 720896},   {'u', 720896},   {'v', 720896},   {'w', 720896},   
    {'x', 729088},   {'y', 720896},   {'z', 720896},   {'{', 2},        
    {'|', 2},        {'}', 2},        {'~', 2},        {'Ё', 720896},  
    {'А', 720896},  {'Б', 720896},  {'В', 720896},  {'Г', 720896},  
    {'Д', 720896},  {'Е', 720896},  {'Ж', 720896},  {'З', 720896},  
    {'И', 720896},  {'Й', 720896},  {'К', 720896},  {'Л', 720896},  
    {'М', 720896},  {'Н', 720896},  {'О', 720896},  {'П', 720896},  
    {'Р', 720896},  {'С', 720896},  {'Т', 720896},  {'У', 720896},  
    {'Ф', 720896},  {'Х', 720896},  {'Ц', 720896},  {'Ч', 720896},  
    {'Ш', 720896},  {'Щ', 720896},  {'Ъ', 720896},  {'Ы', 720896},  
    {'Ь', 720896},  {'Э', 720896},  {'Ю', 720896},  {'Я', 720896},  
    {'а', 1245184}, {'б', 720896},  {'в', 1376256}, {'г', 1245184}, 
    {'д', 1376256}, {'е', 1376256}, {'ж', 1245184}, {'з', 1376256}, 
    {'и', 1376256}, {'й', 1245184}, {'к', 1376256}, {'л', 1376256}, 
    {'м', 1376256}, {'н', 1376256}, {'о', 1245184}, {'п', 851968},  
    {'р', 1245184}, {'с', 1376256}, {'т', 1376256}, {'у', 1245184}, 
    {'ф', 851968},  {'х', 720896},  {'ц', 851968},  {'ч', 1245184}, 
    {'ш', 1245184}, {'щ', 1245184}, {'ъ', 720896},  {'ы', 1245184}, 
    {'ь', 1245184}, {'э', 720896},  {'ю', 720896},  {'я', 1245184}, 
    {'ё', 1245184}
};


uint64_t get_categories_set(char32_t c){
    auto it = categories_table.find(c);
    if(it != categories_table.end()){
        return it->second;
    }else{
        return 1ULL << Other;
    }
}
bool Lexer::start_proc(){
    bool t = true;
    state = -1;
    /* For an automaton that processes a token, the state with the number (-1) is
     * the state in which this automaton is initialized. */
    if(belongs(SPACES, char_categories)){
        loc->current_line += U'\n' == ch;
        return t;
    }
    lexem_begin_line = loc->current_line;
    if(belongs(DELIMITER_BEGIN, char_categories)){
        (loc->pcurrent_char)--; automaton = A_delimiter;
        state = -1;
        return t;
    }

    if(belongs(STRING_BEGIN, char_categories)){
        (loc->pcurrent_char)--; automaton = A_string;
        state = 0;
        buffer.clean();
        return t;
    }

    if(belongs(NUMBER_BEGIN, char_categories)){
        (loc->pcurrent_char)--; automaton = A_number;
        state = 0;
        int_value = 0; fract_part = 0; exponent = 0;sign_of_degree=1; num_of_digits = 0; is_float = false
        return t;
    }

    if(belongs(IDKEYWORD_BEGIN, char_categories)){
        (loc->pcurrent_char)--; automaton = A_idKeyword;
        state = 0;
        return t;
    }

    automaton = A_unknown;
    return t;
}

bool Lexer::unknown_proc(){
    return belongs(Other, char_categories);
}

struct Keyword_list_elem{
    std::u32string keyword;
    lexemcodes kw_code;
};

static const Keyword_list_elem kwlist[] = {
    {U"ввод", Read},             {U"вещ", Kw_float},         
    {U"возврат", Return},     {U"всё", Endif},            
    {U"вывод", Print},          {U"выйди", Exit},         
    {U"для", For},                {U"если", If},             
    {U"завершение", End},  {U"из", In},                 
    {U"иначе", Else},           {U"инес", Elif},           
    {U"истина", True},         {U"конст", Const},        
    {U"лог", Kw_bool},            {U"ложь", False},          
    {U"массив", Array},        {U"ничто", Kw_void},      
    {U"перем", Var},            {U"повторяй", Repeat}, 
    {U"пока", While},            {U"покуда", Until},      
    {U"продолжи", Continue}, {U"прото", Proto},        
    {U"симв", Kw_char},          {U"строка", Kw_string},  
    {U"то", To},                   {U"функ", Func},           
    {U"цел", Kw_int}
};

#define NUM_OF_KEYWORDS 29

#define THERE_IS_NO_KEYWORD (-1)

static int search_keyword(const std::u32string& finded_keyword){
    int result      = THERE_IS_NO_KEYWORD;
    int low_bound   = 0;
    int upper_bound = NUM_OF_KEYWORDS - 1;
    int middle;
    while(low_bound <= upper_bound){
        middle             = (low_bound + upper_bound) / 2;
        auto& curr_kw      = kwlist[middle].keyword;
        int compare_result = finded_keyword.compare(curr_kw);
        if(0 == compare_result){
            return middle;
        }
        if(compare_result < 0){
            upper_bound = middle - 1;
        }else{
            low_bound   = middle + 1;
        }
    }
    return result;
}

static const std::set<size_t> final_states_for_idkeywords = {
    1
};

bool Lexer::idkeyword_proc(){
    bool t             = true;
    bool there_is_jump = false;
    switch(state){
        case 0:
            if(belongs(IDKEYWORD0, char_categories)){
                state = 1;
                there_is_jump = true;
            }
             else if(belongs(IDKEYWORD1, char_categories)){
                buffer += ch;
                state = 1;
                there_is_jump = true;
            }

            break;
        case 1:
            if(belongs(IDKEYWORD2, char_categories)){
                state = 1;
                there_is_jump = true;
            }
             else if(belongs(IDKEYWORD3, char_categories)){
                buffer += ch;
                state = 1;
                there_is_jump = true;
            }

            break;
        default:
            ;
    }

    if(!there_is_jump){
        t = false;
        if(!is_elem(state, final_states_for_idkeywords)){
            printf("At line %zu unexpectedly ended identifier or keyword.", loc->current_line);
            en->increment_number_of_errors();
        }
        
        int search_result = search_keyword(buffer);
        if(search_result != THERE_IS_NO_KEYWORD) {
            token.code = kwlist[search_result].kw_code;
        }
    }

    return t;
}

static const State_for_char init_table_for_delimiters[] ={
    {15, U'!'}, {40, U'#'}, {8, U'%'},  {25, U'&'}, {38, U'('}, 
    {39, U')'}, {3, U'*'},  {1, U'+'},  {45, U','}, {2, U'-'},  
    {6, U'/'},  {33, U':'}, {42, U';'}, {9, U'<'},  {0, U'='},  
    {12, U'>'}, {41, U'?'}, {43, U'['}, {44, U']'}, {27, U'^'}, 
    {36, U'{'}, {23, U'|'}, {37, U'}'}, {29, U'~'}
};

struct Elem {
    /** A pointer to a string of characters that can be crossed. */
    char32_t*       symbols;
    /** A lexeme code. */
    lexemcodes code;
    /** If the current character matches symbols[0], then the transition to the state
     *  first_state;
     *  if the current character matches symbols[1], then the transition to the state
     *  first_state + 1;
     *  if the current character matches symbols[2], then the transition to the state
     *  first_state + 2, and so on. */
    uint16_t        first_state;
};

static const Elem delim_jump_table[] = {
    {const_cast<char32_t*>(U""), Equal, 0},         
    {const_cast<char32_t*>(U""), Plus, 0},          
    {const_cast<char32_t*>(U""), Minus, 0},         
    {const_cast<char32_t*>(U"*"), Mul, 4},          
    {const_cast<char32_t*>(U"."), Pow, 5},          
    {const_cast<char32_t*>(U""), FPow, 0},          
    {const_cast<char32_t*>(U"."), Div, 7},          
    {const_cast<char32_t*>(U""), FDiv, 0},          
    {const_cast<char32_t*>(U""), Mod, 0},           
    {const_cast<char32_t*>(U"=<"), LT, 10},         
    {const_cast<char32_t*>(U""), LEQ, 0},           
    {const_cast<char32_t*>(U""), LShift, 0},        
    {const_cast<char32_t*>(U"=>"), GT, 13},         
    {const_cast<char32_t*>(U""), GEQ, 0},           
    {const_cast<char32_t*>(U""), RShift, 0},        
    {const_cast<char32_t*>(U"=|&^"), LNot, 16},     
    {const_cast<char32_t*>(U""), Nequal, 0},        
    {const_cast<char32_t*>(U"|"), Unknown, 18},     
    {const_cast<char32_t*>(U"&"), Unknown, 19},     
    {const_cast<char32_t*>(U"^"), Unknown, 20},     
    {const_cast<char32_t*>(U""), LNor, 0},          
    {const_cast<char32_t*>(U""), LNAnd, 0},         
    {const_cast<char32_t*>(U""), LNXor, 0},         
    {const_cast<char32_t*>(U"|"), Bor, 24},         
    {const_cast<char32_t*>(U""), Lor, 0},           
    {const_cast<char32_t*>(U"&"), BAnd, 26},        
    {const_cast<char32_t*>(U""), LAnd, 0},          
    {const_cast<char32_t*>(U"^"), BXor, 28},        
    {const_cast<char32_t*>(U""), LXor, 0},          
    {const_cast<char32_t*>(U"^|&"), BNot, 30},      
    {const_cast<char32_t*>(U""), BNXor, 0},         
    {const_cast<char32_t*>(U""), BNor, 0},          
    {const_cast<char32_t*>(U""), BNAnd, 0},         
    {const_cast<char32_t*>(U"=:"), Colon, 34},      
    {const_cast<char32_t*>(U""), Assign, 0},        
    {const_cast<char32_t*>(U""), after_label, 0},   
    {const_cast<char32_t*>(U""), Opening_brace, 0}, 
    {const_cast<char32_t*>(U""), Closing_brace, 0}, 
    {const_cast<char32_t*>(U""), Open_round, 0},    
    {const_cast<char32_t*>(U""), Close_round, 0},   
    {const_cast<char32_t*>(U""), Dim_size, 0},      
    {const_cast<char32_t*>(U""), cond_op, 0},       
    {const_cast<char32_t*>(U""), Semicolon, 0},     
    {const_cast<char32_t*>(U""), Open_square, 0},   
    {const_cast<char32_t*>(U""), Close_square, 0},  
    {const_cast<char32_t*>(U""), Comma, 0}
};

bool Lexer::delimiter_proc(){
    bool t = false;
    if(-1 == state){
        state = get_init_state(ch, init_table_for_delimiters,
                               sizeof(init_table_for_delimiters)/sizeof(State_for_char));
        token.code = delim_jump_table[state].code;
        t = true;
        return t;
    }
    Elem elem = delim_jump_table[state];
    token.code = delim_jump_table[state].code;
    int y = search_char(ch, elem.symbols);
    if(y != THERE_IS_NO_CHAR){
        state = elem.first_state + y; t = true;
    }
    return t;
}

static const std::set<size_t> final_states_for_numbers = {
    1, 2, 3, 4, 5, 6, 7
};

bool Lexer::number_proc(){
    bool t             = true;
    bool there_is_jump = false;
    switch(state){
        case 0:
            if(belongs(NUMBER0, char_categories)){
                int_value = int_value*10 + digit2int(ch);
                state = 7;
                there_is_jump = true;
            }
             else if(belongs(NUMBER1, char_categories)){
                int_value = int_value*10 + digit2int(ch);
                state = 6;
                there_is_jump = true;
            }

            break;
        case 1:
            if(belongs(NUMBER2, char_categories)){
                state = 8;
                there_is_jump = true;
            }
             else if(belongs(NUMBER3, char_categories)){
                int_value = (int_value << 1) + digit2int(ch);
                state = 1;
                there_is_jump = true;
            }

            break;
        case 2:
            if(belongs(NUMBER2, char_categories)){
                state = 9;
                there_is_jump = true;
            }
             else if(belongs(NUMBER4, char_categories)){
                int_value = (int_value << 3) + digit2int(ch);
                state = 2;
                there_is_jump = true;
            }

            break;
        case 3:
            if(belongs(NUMBER2, char_categories)){
                state = 10;
                there_is_jump = true;
            }
             else if(belongs(NUMBER_BEGIN, char_categories)){
                exponent = exponent*10 + digit2int(ch);
                state = 3;
                there_is_jump = true;
            }

            break;
        case 4:
            if(belongs(NUMBER2, char_categories)){
                state = 11;
                there_is_jump = true;
            }
             else if(belongs(NUMBER_BEGIN, char_categories)){
                fract_part = fract_part*10 + digit2int(ch); ++num_of_digits;
                state = 4;
                there_is_jump = true;
            }
             else if(belongs(NUMBER5, char_categories)){
                is_float = true;
                state = 14;
                there_is_jump = true;
            }

            break;
        case 5:
            if(belongs(NUMBER2, char_categories)){
                state = 13;
                there_is_jump = true;
            }
             else if(belongs(NUMBER6, char_categories)){
                int_value = (int_value << 4) + digit2int(ch);
                state = 5;
                there_is_jump = true;
            }

            break;
        case 6:
            if(belongs(NUMBER2, char_categories)){
                state = 12;
                there_is_jump = true;
            }
             else if(belongs(NUMBER7, char_categories)){
                is_float = true;
                state = 11;
                there_is_jump = true;
            }
             else if(belongs(NUMBER_BEGIN, char_categories)){
                int_value = int_value*10 + digit2int(ch);
                state = 6;
                there_is_jump = true;
            }
             else if(belongs(NUMBER5, char_categories)){
                is_float = true;
                state = 14;
                there_is_jump = true;
            }

            break;
        case 7:
            if(belongs(NUMBER2, char_categories)){
                state = 12;
                there_is_jump = true;
            }
             else if(belongs(NUMBER7, char_categories)){
                is_float = true;
                state = 11;
                there_is_jump = true;
            }
             else if(belongs(NUMBER_BEGIN, char_categories)){
                int_value = int_value*10 + digit2int(ch);
                state = 6;
                there_is_jump = true;
            }
             else if(belongs(NUMBER8, char_categories)){
                state = 8;
                there_is_jump = true;
            }
             else if(belongs(NUMBER5, char_categories)){
                is_float = true;
                state = 14;
                there_is_jump = true;
            }
             else if(belongs(NUMBER9, char_categories)){
                state = 13;
                there_is_jump = true;
            }
             else if(belongs(NUMBER10, char_categories)){
                state = 9;
                there_is_jump = true;
            }

            break;
        case 8:
            if(belongs(NUMBER3, char_categories)){
                int_value = (int_value << 1) + digit2int(ch);
                state = 1;
                there_is_jump = true;
            }

            break;
        case 9:
            if(belongs(NUMBER4, char_categories)){
                int_value = (int_value << 3) + digit2int(ch);
                state = 2;
                there_is_jump = true;
            }

            break;
        case 10:
            if(belongs(NUMBER_BEGIN, char_categories)){
                exponent = exponent*10 + digit2int(ch);
                state = 3;
                there_is_jump = true;
            }

            break;
        case 11:
            if(belongs(NUMBER_BEGIN, char_categories)){
                fract_part = fract_part*10 + digit2int(ch); ++num_of_digits;
                state = 4;
                there_is_jump = true;
            }

            break;
        case 12:
            if(belongs(NUMBER_BEGIN, char_categories)){
                int_value = int_value*10 + digit2int(ch);
                state = 6;
                there_is_jump = true;
            }

            break;
        case 13:
            if(belongs(NUMBER6, char_categories)){
                int_value = (int_value << 4) + digit2int(ch);
                state = 5;
                there_is_jump = true;
            }

            break;
        case 14:
            if(belongs(NUMBER11, char_categories)){
                state = 10;
                there_is_jump = true;
            }

            break;
        default:
            ;
    }

    if(!there_is_jump){
        t = false;
        if(!is_elem(state, final_states_for_numbers)){
            printf("At line %zu unexpectedly ended the number.", loc->current_line);
            en->increment_number_of_errors();
        }
        if(is_float)
{
token.code = Float;
token.float_val = build_value();
}
else
{
token.code = Int;
token.int_val = int_value;
}
    }

    return t;
}

static const std::set<size_t> final_states_for_strings = {
    1
};

bool Lexer::string_proc(){
    bool t             = true;
    bool there_is_jump = false;
    switch(state){
        case 0:
            if(belongs(STRING_BEGIN, char_categories)){
                state = 2;
                there_is_jump = true;
            }

            break;
        case 1:
            if(belongs(STRING_BEGIN, char_categories)){
                buffer += ch;
                state = 2;
                there_is_jump = true;
            }

            break;
        case 2:
            if(ch != U'\"'){
                buffer += ch;
                state = 2;
                there_is_jump = true;
            }
             else if(belongs(STRING_BEGIN, char_categories)){
                state = 1;
                there_is_jump = true;
            }

            break;
        default:
            ;
    }

    if(!there_is_jump){
        t = false;
        if(!is_elem(state, final_states_for_strings)){
            printf("At line %zu unexpectedly ended a string literal.", loc->current_line);
            en->increment_number_of_errors();
        }
        token.code=(buffer.length()==1)?Char:String;
            token.string_index = strs -> insert(buffer);
    }

    return t;
}

void Lexer::none_proc(){
    /* This subroutine will be called if, after reading the input text, it turned
     * out to be in the A_start automaton. Then you do not need to do anything. */
}

void Lexer::unknown_final_proc(){
    /* This subroutine will be called if, after reading the input text, it turned
     * out to be in the A_unknown automaton. Then you do not need to do anything. */
}

void Lexer::idkeyword_final_proc(){
    if(!is_elem(state, final_states_for_idkeywords)){
        printf("At line %zu unexpectedly ended identifier or keyword.", loc->current_line);
        en->increment_number_of_errors();
    }

    int search_result = search_keyword(buffer);
    if(search_result != THERE_IS_NO_KEYWORD) {
        token.code = kwlist[search_result].kw_code;
    }

}

void Lexer::delimiter_final_proc(){
        
    token.code = delim_jump_table[state].code;
    
}

void Lexer::number_final_proc(){
    if(!is_elem(state, final_states_for_numbers)){
        printf("At line %zu unexpectedly ended the number.", loc->current_line);
        en->increment_number_of_errors();
    }
    if(is_float)
{
token.code = Float;
token.float_val = build_value();
}
else
{
token.code = Int;
token.int_val = int_value;
}
}

void Lexer::string_final_proc(){
    if(!is_elem(state, final_states_for_strings)){
        printf("At line %zu unexpectedly ended a string literal.", loc->current_line);
        en->increment_number_of_errors();
    }
    token.code=(buffer.length()==1)?Char:String;
    token.string_index = strs -> insert(buffer);
}

Lexem_info Lexer::current_lexem(){
    automaton = A_start; token.code = None;
    lexem_begin = loc->pcurrent_char;
    bool t = true;
    while((ch = *(loc->pcurrent_char)++)){
        char_categories = get_categories_set(ch); //categories_table[ch];
        t = (this->*procs[automaton])();
        if(!t){
            /* We get here only if the lexeme has already been read. At the same time,
             * the current automaton reads the character immediately after the end of
             * the token read, based on this symbol, it is decided that the token has
             * been read and the transition to the next character has been made.
             * Therefore, in order to not miss the first character of the next lexeme,
             * we need to decrease the pcurrent_char pointer by one. */
            (loc->pcurrent_char)--;
            return token;
        }
    }
    /* Here we can be, only if we have already read all the processed text. In this
     * case, the pointer to the current symbol indicates a byte, which is immediately
     * after the zero character, which is a sign of the end of the text. To avoid
     * entering subsequent calls outside the text, we need to go back to the null
     * character. */
    (loc->pcurrent_char)--;
    /* Further, since we are here, the end of the current token (perhaps unexpected)
     * has not yet been processed. It is necessary to perform this processing, and,
     * probably, to display any diagnostics. */
    (this->*finals[automaton])();
    return token;
}


