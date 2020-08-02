// Akhil Gandu
// 920128024

#include <kernel.h>


int train_x = 7, train_y = 1, train_width = 70, train_height = 20;  // location and size of the train window

int window_id; 
int sleep_ticks = 10;       // amount of time to sleep before sending another message to the coms 
int dock_ticks = 70; 

void send_message(char* msg_string) {
    sleep(sleep_ticks); 
    COM_Message msg;
    msg.output_buffer = msg_string;

    char a[0];
    msg.input_buffer = a;
    msg.len_input_buffer = 0;
    send(com_port, &msg); 
}

BOOL probe_message(char* msg_string) {
    sleep(sleep_ticks);
    COM_Message msg;
    msg.output_buffer = msg_string;
    
    char a[3];
    msg.input_buffer = a;
    msg.len_input_buffer = 3;
    send(com_port, &msg); 
    
    if (msg.input_buffer[1] == '1') 
        return TRUE;
    else
        return FALSE; 
}

void wait_for_contact(char* msg_string) {
    while (42) {
        send_message("R\015");
        if (probe_message(msg_string) == TRUE) { 
            return; 
        }
    }
}

void cone_nz() {

    // set the path
    wm_print(window_id, "Setting the course for wagon\n");
    send_message("M3R\015");
    send_message("M4R\015");
    send_message("M5R\015");
    send_message("M6G\015");
    send_message("M7G\015");
    send_message("M2R\015");
    send_message("M1R\015");
    
    // start the train
    wm_print(window_id, "Starting the train\n");
    send_message("L20S5\015");

    // wait until it reaches track segment 9
    wait_for_contact("C9\015");
    sleep(dock_ticks - 15);
    send_message("L20S0\015");
    wm_print(window_id, "Docked with the wagon\n");
    
    // reverse and start heading back
    wm_print(window_id, "Heading back home\n");
    send_message("L20D\015");
    send_message("L20S5\015");
    
    // wait until it reaches track segment 5
    wait_for_contact("C5\015");
    send_message("L20S0\015");
    wm_print(window_id, "Got back home\n");
}

void cone_z() {

    // setting the inner path (off the main outer loop)
    send_message("M3R\015");
    send_message("M6G\015");
    send_message("M7G\015");
    send_message("M2R\015");
            
    // wait for the zamboni to pass switch 4 
    wait_for_contact("C6\015");
    wm_print(window_id, "Zamboni passed by switch 4\n");    
    
    // start the train, get the wagon
    send_message("M4R\015");
    send_message("L20S5\015");
    
    // Wait for the zamboni, to pass switch 5
    wait_for_contact("C10\015");
    wm_print(window_id, "Zamboni passed by switch 5\n");
    
    // construct the path for the train to get to the wagon 
    send_message("M5R\015");
    send_message("M4G\015");
    
    // Wait for the train, to pass switch 6
    wait_for_contact("C9\015"); 
    sleep(dock_ticks - 10); 
    wm_print(window_id, "Docked with the wagon\n"); 
    send_message("L20S0\015"); 
    send_message("L20D\015"); 
    
    // reset outer loop for zamboni
    send_message("M5G\015");
    
    // move train back up to segment 12
    send_message("L20S5\015");
    wait_for_contact("C12\015");
    send_message("L20S0\015");
    
    // Wait for the zamboni, to pass switch 5
    wait_for_contact("C10\015");
    wm_print(window_id, "Zamboni passed by switch 5\n");

    // set the path for train to get back to home base     
    send_message("M5R\015");
    send_message("M4R\015");
    
    // reverse and start heading back
    wm_print(window_id, "Heading back home\n"); 
    send_message("L20S5\015"); 
    
    // wait until it reaches track segment 5
    wait_for_contact("C5\015"); 
    send_message("L20S0\015");
    wm_print(window_id, "Got back to home base\n");

    // reconstruct the circuit for zamboni    
    send_message("M4G\015");
    send_message("M5G\015");
}

void ctwo_nz() {

    // take train out onto outer track
    wm_print(window_id, "Going out onto the main track\n");
    send_message("M8R\015");
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C13\015");
    send_message("L20S0\015");
    
    // set the path to the wagon 
    wm_print(window_id, "Configuring the path to the wagon\n");
    send_message("M8G\015");
    send_message("M4R\015");
    send_message("M3G\015");
    send_message("M2G\015");
    
    // go to wagon
    wm_print(window_id, "Going to rendevous with wagon\n");
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C6\015");
    sleep(dock_ticks + 70); 
    send_message("L20S0\015");
    wm_print(window_id, "Docked with the wagon\n");
    send_message("L20D\015");
    
    // go back 
    wm_print(window_id, "Going back home\n");
    send_message("L20S5\015");
    wait_for_contact("C13\015");
    send_message("L20S0\015");
    
    // back up into the home base
    send_message("M8R\015");
    send_message("M7R\015");
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C11\015");
    send_message("L20S0\015");
    wm_print(window_id, "Got back home\n");
}

void ctwo_z() {
    
    // wait until zamboni passes switch 8
    send_message("M7R\015");
    wait_for_contact("C10\015");
    
    // take train out onto outer track
    wm_print(window_id, "Going out onto the main track\n");
    send_message("M8R\015");
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C13\015");
    send_message("L20S0\015");
    
    // set the path to the wagon and start 
    wm_print(window_id, "Configuring the path to the wagon\n");
    send_message("M8G\015");
    send_message("M3G\015");
    send_message("M2G\015");
    
    // start the train 
    wm_print(window_id, "Going to rendevous with wagon\n");
    send_message("L20D\015");
    send_message("L20S5\015");
    
    // wait for zamboni to pass switch 4
    wait_for_contact("C4\015");
    send_message("M4R\015");
    
    // arrive at the wagon 
    wait_for_contact("C6\015");
    sleep(dock_ticks + 70); 
    send_message("L20S0\015");
    wm_print(window_id, "Docked with the wagon\n");
    send_message("M4G\015");
    
    // wait for zamboni to pass switch 8
    wait_for_contact("C10\015");
    send_message("M8R\015");
    send_message("M1R\015");
    send_message("L20S5\015");
    wait_for_contact("C13\015");
    sleep(dock_ticks - 30); 
    send_message("L20S0\015");
    send_message("M1G\015");
    send_message("M8G\015");
    wm_print(window_id, "Got back to home base\n");
}

void cthree_nz() {
    // take the train out onto the track
    wm_print(window_id, "Taking the train out onto the track\n");
    send_message("M9G\015");
    send_message("L20S5\015");
    wait_for_contact("C14\015");
    send_message("L20S0\015");
    send_message("L20D\015");
    
    // configure the route to the wagon
    wm_print(window_id, "Configuring the route to the wagon\n");
    send_message("M9R\015");
    send_message("M1R\015");
    send_message("M2G\015");
    send_message("M3G\015");
    
    // rendevouz with the wagon 
    wm_print(window_id, "Rendezvous with the wagon\n");
    send_message("L20S5\015");
    wait_for_contact("C1\015");
    sleep(dock_ticks);
    send_message("L20S0\015");
    send_message("L20D\015");
    
    // go back home
    wm_print(window_id, "Dockd with wagon, heading back to home base\n");
    send_message("L20S5\015");
    wait_for_contact("C14\015");
    send_message("L20S0\015");
    
    // park it off the track
    wm_print(window_id, "Parking the wagon\n");
    send_message("M9G\015");
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C16\015");
    send_message("L20S0\015");
}

void cthree_z() {
    // wait for zamboni to reach segment 13
    send_message("M2G\015");
    send_message("M3G\015");
    wait_for_contact("C13\015");
    
    // take the train out onto the track
    wm_print(window_id, "Taking the train out onto the track\n");
    send_message("M9G\015");
    send_message("L20S5\015");
    wait_for_contact("C14\015");
    send_message("L20S0\015");
    send_message("L20D\015");
    
    // configure the route to the wagon
    wm_print(window_id, "Configuring the route to the wagon\n");
    send_message("M9R\015");
    send_message("M1R\015");
    
    // rendevouz with the wagon 
    wm_print(window_id, "Rendezvous with the wagon\n");
    send_message("L20S5\015");
    wait_for_contact("C1\015");
    send_message("M1G\015");
    sleep(dock_ticks-20);
    send_message("L20S0\015");
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C1\015");
    send_message("L20S0\015");
    
    // wait for zamboni to pass switch 9
    wait_for_contact("C14\015");
    
    // go back home
    wm_print(window_id, "Dockd with wagon, heading back to home base\n");
    send_message("M1R\015");
    send_message("L20S5\015");
    wait_for_contact("C14\015");
    send_message("L20S0\015");
    send_message("M1G\015");
    
    // park it off the track
    wm_print(window_id, "Parking the wagon\n");
    send_message("M9G\015");
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C16\015");
    send_message("L20S0\015");
    send_message("M9R\015");
    
}

void cfour_nz() {
    // configure the route to the wagon
    wm_print(window_id, "Configuring the route to the wagon\n");
    send_message("M9G\015");
    
    // start on path to the wagon 
    wm_print(window_id, "Start on the path to the wagon\n");
    send_message("L20S5\015");
    wait_for_contact("C7\015");
    send_message("L20S0\015");
    
    // configure the final bit of route to wagon
    wm_print(window_id, "Configure the final bit of route to the wagon\n");
    send_message("M5R\015");
    send_message("M6R\015");
    
    // dock with the wagon
    send_message("L20D\015");
    send_message("L20S5\015");
    sleep(dock_ticks+40);
    send_message("L20S0\015");
    wm_print(window_id, "Docked with the wagon\n");

    // start to head back home
    wm_print(window_id, "Heading back to home base\n");
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C7\015");
    send_message("L20S0\015");
    
    // configure the route back home
    send_message("M5G\015");
    
    // head back to home base
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C14\015");
    sleep(dock_ticks - 20); 
    send_message("L20S0\015");
    
}

void cfour_z() {

    // wait for zamboni to pass switch 9
    wait_for_contact("C14\015");
    
    // configure the route to the wagon
    wm_print(window_id, "Configuring the route to the wagon\n");
    send_message("M9G\015");
    
    // start on path to the wagon 
    wm_print(window_id, "Start on the path to the wagon\n");
    send_message("L20S5\015");
    wait_for_contact("C4\015");
    send_message("L20S0\015");
    send_message("M9R\015");
    
    // configure the final bit of route to wagon
    wm_print(window_id, "Configure the final bit of route to the wagon\n");
    send_message("M5R\015");
    send_message("M6R\015");
    
    // dock with the wagon
    send_message("L20D\015");
    send_message("L20S5\015");
    wait_for_contact("C7\015");
    sleep(dock_ticks + 40);
    send_message("L20S0\015");
    wm_print(window_id, "Docked with the wagon\n");
    send_message("M5G\015");
    
    // wait for the zamboni to pass switch 5
    wm_print(window_id, "Waiting for zamboni to pass switch 5\n");
    send_message("L20D\015");
    wait_for_contact("C7\015");
    
    // follow the zamboni
    wm_print(window_id, "Follow the zamboni\n");
    send_message("M5R\015");
    send_message("L20S5\015");
    
    // wait for zamboni to pass switch 4
    wait_for_contact("C4\015");
    send_message("M5G\015");
    
    // wait for zamboni to pass switch 8
    wait_for_contact("C10\015");
    sleep(dock_ticks - 30); 

    // park train in home base
    wm_print(window_id, "Parking the train in home base\n");
    send_message("L20S0\015");
    send_message("L20D\015");
    send_message("M9G\015");
    send_message("L20S5\015");
    wait_for_contact("C14\015");
    sleep(dock_ticks - 25); 
    send_message("L20S0\015");
    send_message("M9R\015");
}

void train_process(PROCESS self, PARAM param) {
    window_id = wm_create(train_x, train_y, train_width, train_height); 
    wm_print(window_id, "Welcome to the train app\n");
        
    // change all the switches so an outer loop is created
    wm_print(window_id, "Creating loop, setting switches\n");
    send_message("M4G\015");
    send_message("M5G\015");
    send_message("M8G\015");
    send_message("M9R\015");
    send_message("M1G\015");
    
    // detect the presence of the zamboni
    int com_cycles = 1000; // number of com messages we must send to probe the track for zambonie for 35 seconds
    int divisor = (sleep_ticks*2 + 20);
    com_cycles /= divisor;
    com_cycles /= 2; 
    com_cycles += 2; 
    
    wm_print(window_id, "Probing %d cycles\n", com_cycles); 
    
    BOOL zamboni_detected = FALSE; 

    for (int i = 0; i < com_cycles; i++) {
        send_message("R\015");
        if (probe_message("C3\015") == TRUE) { 
            zamboni_detected = TRUE;
            wm_print(window_id, "Probe %d, contact made\n", i); 
            break;
        } else {
            wm_print(window_id, "Probe %d, no contact\n", i); 
        }
    }
    
    // check if config_1
    wm_print(window_id, "Checking if configuration 1\n");
    send_message("R\015");
    BOOL check_11 = probe_message("C5\015");
    send_message("R\015");
    BOOL check_12 = probe_message("C12\015");
    
    // check if config_2
    wm_print(window_id, "Checking if configuration 2\n");
    send_message("R\015");
    BOOL check_21 = probe_message("C2\015");
    send_message("R\015");
    BOOL check_22 = probe_message("C11\015");
    
    // check if config_3
    wm_print(window_id, "Checking if configuration 3\n");
    send_message("R\015");
    BOOL check_31 = probe_message("C2\015");
    send_message("R\015");
    BOOL check_32 = probe_message("C16\015");
    
    // check if config_4
    wm_print(window_id, "Checking if configuration 4\n");
    send_message("R\015");
    BOOL check_41 = probe_message("C8\015");
    send_message("R\015");
    BOOL check_42 = probe_message("C16\015");
    
    if (check_11 == TRUE && check_12 == TRUE) {
        if (zamboni_detected == FALSE) {
            // config 1, no zamboni
            wm_print(window_id, "Detected Configuration 1, without a zamboni present\n");
            cone_nz(); 
        } else {
            // config 1, zamboni present
            wm_print(window_id, "Detected Configuration 1, with a zamboni present\n");
            cone_z();
        }
    } else if (check_21 == TRUE && check_22 == TRUE) {
        if (zamboni_detected == FALSE) {
            // config 2, no zamboni
            wm_print(window_id, "Detected Configuration 2, without a zamboni present\n");
            ctwo_nz();
        } else {
            // config 2, zamboni present
            wm_print(window_id, "Detected Configuration 2, with a zamboni present\n");
            ctwo_z();
        }
    } else if (check_31 == TRUE && check_32 == TRUE) {
        if (zamboni_detected == FALSE) {
            // config 3, no zamboni
            wm_print(window_id, "Detected Configuration 3, without a zamboni present\n");
            cthree_nz();
        } else {
            // config 3, zamboni present
            wm_print(window_id, "Detected Configuration 3, with a zamboni present\n");
            cthree_z();
        }
    } else if (check_41 == TRUE && check_42 == TRUE) {
        if (zamboni_detected == FALSE) {
            // config 4, no zamboni
            wm_print(window_id, "Detected Configuration 4, without a zamboni present\n");
            cfour_nz();
        } else {
            // config 4, zamboni present
            wm_print(window_id, "Detected Configuration 4, with a zamboni present\n");
            cfour_z();
        }
    }

    wm_print(window_id, "Program terminated\n");   
    become_zombie(); 
}


void init_train() {
    create_process(train_process, 5, 0, "shell_process");
    
}
