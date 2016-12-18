#include <xcb/xcb.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define log(msg, args...) \
    do {                                                                \
        printf("[%s - %s:%d]" msg "\n", __func__, __FILE__, __LINE__,   \
        ##args);                                                        \
    }while(0)



int main(void){
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_window_t window;

    connection = xcb_connect(NULL, NULL);

    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    
    window = xcb_generate_id(connection);

    xcb_create_window(connection,
            XCB_COPY_FROM_PARENT,
            window,
            screen->root,
            0, 0,
            150, 150,
            10,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen->root_visual,
            0, NULL);

    xcb_map_window(connection, window);

    xcb_flush(connection);

    pause ();

    return 0;
}

