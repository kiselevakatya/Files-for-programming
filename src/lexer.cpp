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
    &Lexer::number_proc()
};

Lexer::Final_proc Lexer::finals[] = {
    &Lexer::none_proc(),            &Lexer::unknown_final_proc(),   
    &Lexer::idkeyword_final_proc(), &Lexer::delimiter_final_proc(), 
    &Lexer::number_final_proc()
};

enum Category {
    SPACES,     DELIMITER_BEGIN, 
    NUMBER0,    NUMBER3,         
    NUMBER4,    NUMBER_BEGIN,    
    NUMBER1,    NUMBER2,         
    NUMBER5,    NUMBER6,         
    NUMBER7,    NUMBER8,         
    NUMBER9,    NUMBER10,        
    NUMBER11,   IDKEYWORD_BEGIN, 
    IDKEYWORD0, IDKEYWORD1,      
    IDKEYWORD2, IDKEYWORD3,      
    Other
};

static const std::map<char32_t, uint32_t> categories_table = {
    {'\0', 1},      {'\X01', 1},    {'\X02', 1},    {'\X03', 1},    
    {'\X04', 1},    {'\X05', 1},    {'\X06', 1},    {'\a', 1},      
    {'\b', 1},      {'\t', 1},      {'\n', 1},      {'\v', 1},      
    {'\f', 1},      {'\r', 1},      {'\X0e', 1},    {'\X0f', 1},    
    {'\X10', 1},    {'\X11', 1},    {'\X12', 1},    {'\X13', 1},    
    {'\X14', 1},    {'\X15', 1},    {'\X16', 1},    {'\X17', 1},    
    {'\X18', 1},    {'\X19', 1},    {'\X1a', 1},    {'\X1b', 1},    
    {'\X1c', 1},    {'\X1d', 1},    {'\X1e', 1},    {'\X1f', 1},    
    {' ', 1},       {'!', 2},       {'#', 2},       {'%', 2},       
    {'&', 2},       {', 128},       {'(', 2},       {')', 2},       
    {'*', 2},       {'+', 16386},   {',', 2},       {'-', 16386},   
    {'.', 1024},    {'/', 2},       {'0', 262716},  {'1', 262776},  
    {'2', 262768},  {'3', 262768},  {'4', 262768},  {'5', 262768},  
    {'6', 262768},  {'7', 262768},  {'8', 262752},  {'9', 262752},  
    {':', 2},       {';', 2},       {'<', 2},       {'=', 2},       
    {'>', 2},       {'?', 2},       {'A', 360960},  {'B', 363008},  
    {'C', 360960},  {'D', 360960},  {'E', 361216},  {'F', 360960},  
    {'G', 360448},  {'H', 360448},  {'I', 360448},  {'J', 360448},  
    {'K', 360448},  {'L', 360448},  {'M', 360448},  {'N', 360448},  
    {'O', 360448},  {'P', 360448},  {'Q', 360448},  {'R', 360448},  
    {'S', 360448},  {'T', 360448},  {'U', 360448},  {'V', 360448},  
    {'W', 360448},  {'X', 364544},  {'Y', 360448},  {'Z', 360448},  
    {'[', 2},       {']', 2},       {'^', 2},       {'_', 360448},  
    {'a', 360960},  {'b', 363008},  {'c', 360960},  {'d', 360960},  
    {'e', 361216},  {'f', 360960},  {'g', 360448},  {'h', 360448},  
    {'i', 360448},  {'j', 360448},  {'k', 360448},  {'l', 360448},  
    {'m', 360448},  {'n', 360448},  {'o', 368640},  {'p', 360448},  
    {'q', 360448},  {'r', 360448},  {'s', 360448},  {'t', 360448},  
    {'u', 360448},  {'v', 360448},  {'w', 360448},  {'x', 364544},  
    {'y', 360448},  {'z', 360448},  {'{', 2},       {'|', 2},       
    {'}', 2},       {'~', 2},       {'Ё', 360448}, {'А', 360448}, 
    {'Б', 360448}, {'В', 360448}, {'Г', 360448}, {'Д', 360448}, 
    {'Е', 360448}, {'Ж', 360448}, {'З', 360448}, {'И', 360448}, 
    {'Й', 360448}, {'К', 360448}, {'Л', 360448}, {'М', 360448}, 
    {'Н', 360448}, {'О', 360448}, {'П', 360448}, {'Р', 360448}, 
    {'С', 360448}, {'Т', 360448}, {'У', 360448}, {'Ф', 360448}, 
    {'Х', 360448}, {'Ц', 360448}, {'Ч', 360448}, {'Ш', 360448}, 
    {'Щ', 360448}, {'Ъ', 360448}, {'Ы', 360448}, {'Ь', 360448}, 
    {'Э', 360448}, {'Ю', 360448}, {'Я', 360448}, {'а', 622592}, 
    {'б', 360448}, {'в', 688128}, {'г', 622592}, {'д', 688128}, 
    {'е', 688128}, {'ж', 622592}, {'з', 688128}, {'и', 688128}, 
    {'й', 622592}, {'к', 688128}, {'л', 688128}, {'м', 688128}, 
    {'н', 688128}, {'о', 622592}, {'п', 425984}, {'р', 622592}, 
    {'с', 688128}, {'т', 688128}, {'у', 622592}, {'ф', 425984}, 
    {'х', 360448}, {'ц', 425984}, {'ч', 622592}, {'ш', 622592}, 
    {'щ', 622592}, {'ъ', 360448}, {'ы', 622592}, {'ь', 622592}, 
    {'э', 360448}, {'ю', 360448}, {'я', 622592}, {'ё', 622592}
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


