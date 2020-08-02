// version 9

#include <kernel.h>

int max_user_input_len = 40;        // the maximum size a command may be

// 10, 3, 50, 17

int upper_x = 3, upper_y = 3, shell_width = 70, shell_height = 20;      // the location and size of the shell window

struct Historical_Command {
    int index; 
    char* user_input; 
    int user_input_len; 
    struct Historical_Command* next; 
};

const char* standard_command[9] = {   // the list of accepted commands
    "help", "clear", "shell", "pong", "echo", "ps", "history", "about", "train"
};

int standard_command_len[9] = {4, 5, 5, 4, 4, 2, 7, 5, 5};             // the length of each standard command


void shell_process(PROCESS self, PARAM param);      // declaring the shell process function


void print_process_details_shell(int window_id, PROCESS p) {
    static const char *state[] = { "READY          ",
        "ZOMBIE         ",
        "SEND_BLOCKED   ",
        "REPLY_BLOCKED  ",
        "RECEIVE_BLOCKED",
        "MESSAGE_BLOCKED",
        "INTR_BLOCKED   "
    };
    if (!p->used) {
        wm_print(window_id, "PCB slot unused!\n");
        return;
    }
    /* State */
    wm_print(window_id, state[p->state]);
    /* Check for active_proc */
    if (p == active_proc)
        wm_print(window_id, " *      ");
    else
        wm_print(window_id, "        ");
    /* Priority */
    wm_print(window_id, "  %2d", p->priority);
    /* Name */
    wm_print(window_id, " %s\n", p->name);
}

void print_process_table(int window_id) {
    int             i;
    PCB            *p = pcb;

    wm_print(window_id, "\nState           Active Prio Name\n");
    wm_print(window_id, "------------------------------------------------\n");
    for (i = 0; i < MAX_PROCS; i++, p++) {
        if (!p->used)
            continue;
        print_process_details_shell(window_id, p);
    }
}

// print out the history of the commands entered by the user
void print_command_history(int window_id, struct Historical_Command* history_head, struct Historical_Command* history_tail, int history_len) {
    
    struct Historical_Command* a = history_head; 
    
    for (int i = 1; i <= history_len; i++) {
        wm_print(window_id, "\n%3d - %s", a->index, a->user_input);
        a = a->next;  
    }
    wm_print(window_id, "\n"); 
}

// a function to process the input from the user
void process_user_input(int window_id, char* user_input, int user_input_len, struct Historical_Command** history_head, struct Historical_Command** history_tail, int* history_len) {
    int execute_commands_list[20];
    int execute_commands_list_len = 0; 
    char* echo_strings_list[20];
    int echo_strings_list_len = 0; 
    int history_execute_list[20];
    int history_execute_list_len = 0; 
    BOOL EXIT_CONDITION = FALSE; 
    
    for (int i = 0; i <= user_input_len; i++) {
        
        BOOL started_command = FALSE; 
        BOOL started_echo = FALSE;
        BOOL started_history_execute = FALSE; 
        int current_command_scan_len = 0; 
        
        // Keeps track of which standard command prefix we have matched
        BOOL standard_command_prefix_match[9];    
        
        for (int j = 0; j < 9; j++) {
            standard_command_prefix_match[j] = TRUE; 
        }
        
        // how much of the standard prefex command have we matched? 
        int standard_command_prefix_len_match[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; 
        
        // first we read the command
        for ( ; i <= user_input_len; i++) {
            
            // there are only two ways to end a command, with ';' or '\0'
            
            if (started_command) {  // now we are in the middle of scanning the command, check if the next character is the end of the command
            
                // we use either a blank space, or a semicolon or an end of line character
                
                if (user_input[i] == ' ' || user_input[i] == ';' || user_input[i] == '\0') {
                    // we have reached the end of the command 
                    
                    
                    BOOL command_matched = FALSE; 
                    BOOL echo_matched = FALSE; 
                    // Find out which command we matched (if any)
                    for (int j = 0; j < 9; j++) {
                        if ((standard_command_prefix_match[j] == TRUE) && (standard_command_prefix_len_match[j] == standard_command_len[j])) {
                                // we have matched a command

                                if (j == 4) {
                                    // echo matched 
                                    echo_matched = TRUE; 
                                } else {
                                    // regular command matched 
                                    command_matched = TRUE; 
                                }
                                                                
                                execute_commands_list[execute_commands_list_len] = j; 
                                execute_commands_list_len++; 
                                break; 
                        }   
                    }  
                    
                    if (command_matched) {
                        // good to go, continue scanning
                        
                    } else if (echo_matched) {
                        started_echo = TRUE; 

                        break;  // get out of reading commands, go to next command
                    } else {
                        // nothing was matched, exit 
                        
                        EXIT_CONDITION = TRUE; 
                        break; 
                    }
                    
                    // RESET ALL VARIABLES
                    started_command = FALSE;    // done reading the command
                    for (int j = 0; j < 9; j++) {
                        standard_command_prefix_match[j] = TRUE; 
                        standard_command_prefix_len_match[j] = 0; 
                    }
                    current_command_scan_len = 0; 
                    
                    while (user_input[i] == ' ')
                        i++;        // get rid of all the trailing white spaces
                    
                    if (user_input[i] != ';' && user_input[i] != '\0') {
                        EXIT_CONDITION = TRUE;
                        break;
                    }
                    
                } else {    // We are in the middle of a command, read the next character, as it's not the end

                    current_command_scan_len++;
                    for (int j = 0; j < 9; j++) {
                        if (current_command_scan_len <= standard_command_len[j]) {
                            if (user_input[i] != standard_command[j][current_command_scan_len - 1]) {
                                // the scanned command no longer matches the prefix of standard command j
                                standard_command_prefix_match[j] = FALSE; 
                            } else {
                                // the scanned command still matches the prefix of standard command j, update matched prefix length
                                // this is how many characters of the prefix we have matched so far
                                standard_command_prefix_len_match[j]++;
                            }
                        } else {
                            // the scanned command thus far is too large to match command j
                            standard_command_prefix_match[j] = FALSE; 
                        }
                    }
                }
                
            } else {  // We haven't started reading the command yet, still looking for the first letter
                
                if (user_input[i] == ' ') {
                    // go ahead and ignore all whitespaces
                } else if (user_input[i] == '!') {
                    // found the history execute character !
                    started_history_execute = TRUE; 
                    i++;
                    break; 
                } else {
                    // found our first character, so the command scan has started
                    started_command = TRUE; 
                    current_command_scan_len = 1; 
                    for (int j = 0; j < 9; j++) {
                        if (user_input[i] != standard_command[j][0]) {
                            standard_command_prefix_match[j] = FALSE; 
                        } else {
                            // update the number of prefix letters we matched for this command so far
                            standard_command_prefix_len_match[j]++;      
                        }
                    }
                }
            }
        }
        
        if (!started_history_execute) {
            struct Historical_Command* a = (struct Historical_Command*) malloc(sizeof(struct Historical_Command)); 
            *history_len += 1;
            a -> index = *history_len; 
            a -> user_input = user_input;
            a -> user_input_len = user_input_len; 
            if (*history_len == 1) {     // this is the first command entered by the user
                (*history_head) = a;
                (*history_tail) = a; 
                (*history_head)->next = history_tail; 
            } else {
                (*history_tail)->next = a;
                (*history_tail) = a; 
            }
        }
        
        
        
        if (EXIT_CONDITION) {
        
            break; 
        } else if (started_history_execute) {
            // this is a history execute command, read the integer
            execute_commands_list[execute_commands_list_len] = 9;           // we will denote this command, as being mapped to number 9
            execute_commands_list_len++; 
            int number_in_history = 0; 
            char char_digit; 
            int digit; 
            
            while (user_input[i] != ' ' && user_input[i] != ';' && user_input[i] != '\0') {
                char_digit = user_input[i];
                if (char_digit >= '0' && char_digit <= '9') {
                  digit = (int)char_digit - 48; 
                  number_in_history *= 10;
                  number_in_history += digit; 
                } else {
                    EXIT_CONDITION = TRUE; 
                    break;
                }
                i++;
            }
            
            while (user_input[i] == ' ')
                i++;        // get rid of all the trailing white spaces
            
            if (number_in_history < history_len) {
                history_execute_list[history_execute_list_len] = number_in_history;
                history_execute_list_len++;
            } else {
                EXIT_CONDITION = TRUE; 
                break; 
            }
            
            struct Historical_Command* a = (struct Historical_Command*) malloc(sizeof(struct Historical_Command)); 
            *history_len += 1;
            a -> index = *history_len; 
            a -> user_input = user_input;
            a -> user_input_len = user_input_len; 
            if (*history_len == 1) {     // this is the first command entered by the user
                (*history_head) = a;
                (*history_tail) = a; 
                (*history_head)->next = history_tail; 
            } else {
                (*history_tail)->next = a;
                (*history_tail) = a; 
            }
            
            started_history_execute = FALSE; 

        } else if (started_echo) {
            // if it's an echo command we also read the argument to it  
            echo_strings_list[echo_strings_list_len] = (char*) malloc(max_user_input_len * sizeof(char)); 
            int current_echo_string_length = 0; 
            
            for (; i <= user_input_len; i++) {
                if (user_input[i] == ';' || user_input[i] == '\0') {
                    // we've reached the end of the user input echo string
                    
                    echo_strings_list[echo_strings_list_len][current_echo_string_length] = '\0';
                    current_echo_string_length++;
                    break; 
                    
                } else {
                    echo_strings_list[echo_strings_list_len][current_echo_string_length] = user_input[i];
                    current_echo_string_length++;    
                }
            }
            
            echo_strings_list_len++;
            
            started_echo = FALSE;       // finshed echo
        }
    }
    
    if (EXIT_CONDITION) {
        wm_print(window_id, "\nSorry, the command you have entered is not recognized, please try again or type 'help' to review the list of supported commands\n"); 

    } else {
        // NOW WE HAVE ALL THE COMMANDS THE USER HAS TYPED, NOW WE BEGIN EXECUTING THEM
        int current_echo_string_index = 0; 
        int current_history_execute_index = 0; 
        
        //wm_print(window_id, "\n The number of commands entered is: ");
        //wm_print(window_id, "%d", execute_commands_list_len);
        
        for (int i = 0; i < execute_commands_list_len; i++) {
            //wm_print(window_id, "\n we have receieved the command: ");
            
            if (execute_commands_list[i] == 9) { // execute a history command, i.e. one we previously executed 
                int index_of_command = history_execute_list[current_history_execute_index];
                
                struct Historical_Command* a = *history_head; 
                
                while (a -> index != index_of_command) {
                    a = a -> next; 
                }
                process_user_input(window_id, a->user_input, a-> user_input_len, history_head, history_tail, history_len); 
                
                current_history_execute_index++;
                
            } else if (execute_commands_list[i] == 4) {
                // this is an echo command
                
                wm_print(window_id, "\n %s \n", echo_strings_list[current_echo_string_index]); 
                free(echo_strings_list[current_echo_string_index]);
                current_echo_string_index++;
            } else {
                int command_number = execute_commands_list[i]; 
                
                switch(command_number) {
                    case 0:
                        // execute the help command
                        //     "help(0)", "clear(1)", "shell(2)", "pong(3)", "echo(4)", "ps(5)", "history(6)", "about(7)"
                        
                        wm_print(window_id, "\nHi, welcome to the help menu, below are the list of commands supported by this shell"); 
                        wm_print(window_id, "\n%10s - %s", standard_command[0], "Opens the help menu");        // help
                        wm_print(window_id, "\n%10s - %s", standard_command[1], "Clears the shell window");    // clear
                        wm_print(window_id, "\n%10s - %s", standard_command[2], "Launches another shell");     // shell
                        wm_print(window_id, "\n%10s - %s", standard_command[3], "Launches the PONG game");     // pong
                        wm_print(window_id, "\n%4s%6s - %s", standard_command[4], "<msg>", "Echoes to the console the message (<msg>) entered by the user");     // echo
                        wm_print(window_id, "\n%10s - %s", standard_command[5], "Prints out the process table");  // ps
                        wm_print(window_id, "\n%10s - %s", standard_command[6], "Prints out all the commands that have been typed into the shell so far, in order, prefixed with an index");      // history
                        wm_print(window_id, "\n%10s - %s", "!<number>", "Executes the command with the index matching the number specified");        // !<number>
                        wm_print(window_id, "\n%10s - %s\n", standard_command[7], "Prints out my name and a fun fact");
                        
                        break;
                    case 1:
                        // execute the clear command
                        wm_clear(window_id);                         
                        break;
                    case 2:
                        // open up another shell
                        create_process(shell_process, 4, 0, "shell_process");
                        wm_print(window_id, "\n"); 
                        break;
                    case 3:
                        // open up a pong game
                        start_pong(); 
                        wm_print(window_id, "\n"); 
                        break;
                    case 5:
                        // Print out the process table 
                        print_process_table(window_id); 
                        break;
                    case 6: 
                        // execute history command
                        print_command_history(window_id, *history_head, *history_tail, *history_len); 
                        break;
                    case 7:
                        // execute the about command
                        wm_print(window_id, "\nHi my name is Akhil and here's a fun fact: The word 'Arno' in German actually means Eagle.\n"); 
                        break;    
                    case 8:
                        // execute the train program
                        wm_print(window_id, "\nFiring up the train simulator\n"); 
                        init_train(); 
                        break;
                    default:
                        // none of the cases matched, should not be here
                        break;
                }
                
                //wm_print(window_id, "\n executing one of the standard commands: ");
                //wm_print(window_id, "%s", standard_command[command_number]); 
            }
        }
    }
}

void shell_process(PROCESS self, PARAM param) {

    //int window_id = wm_create(10, 3, 50, 17);       // initialize the shell
    int window_id = wm_create(upper_x, upper_y, shell_width, shell_height); 
    wm_print(window_id, ">> ");
    struct Historical_Command* history_head;
    struct Historical_Command* history_tail;
    int history_len = 0; 

    char* user_input = (char*) malloc((max_user_input_len+1) * sizeof(char)); // leave enough space for the end of line character
    int user_input_len = 0;         // initially the user input's length is 0
    
    while (1) {
    
        char ch = keyb_get_keystroke(window_id, TRUE);
        if (ch == '\n' || ch == '\r') {
            // the user pressed the enter key, signifying the end of the user input
            
            user_input[user_input_len] = '\0';
            
            if (user_input_len > 0) {
                process_user_input(window_id, user_input, user_input_len, &history_head, &history_tail, &history_len);      // process the user's input
            } else {
                free(user_input);
                wm_print(window_id, "\n");
            }
            
            // re initialize the shell
            //free(user_input);
            user_input = (char*) malloc((max_user_input_len+1) * sizeof(char));
            user_input_len = 0;
            wm_print(window_id, ">> ");
        
        } else if (ch == '\b') {
            // the user pressed the backspace key, decrement current user input length
            
            if (user_input_len > 0) {               // make sure there are characters to erase
                wm_print(window_id, "%c", ch);      // remove most recently written character
                user_input_len--;        // update length of the user input
            }
        
        } else {
            // the user typed another key, giving us input
            if (user_input_len < max_user_input_len) {      // make sure user isn't trying to write too many characters

                user_input[user_input_len] = ch;      // user_input_len is the position in the user input array we need to write to next
                user_input_len++;               // now user_input_len signifies the total number of characters in user input

                wm_print(window_id, "%c", ch);  // write this key out to the shell
            }
            
        }    
    }
    
}


void start_shell() {
    create_process(shell_process, 4, 0, "shell_process");
    //resign();
}
