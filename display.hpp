using namespace std;
using namespace chrono;

class window {
public:
    window(int height, int width, int y, int x) {
        this->height = height;
        this->width = width;
        this->y = y;
        this->x = x;
        max_y = height - 2;
        win = newwin(height, width, y, x);
    }

    void init() {
        box(win, 0, 0);
        //         1         2         3         4         5         6         7 
        //1234567890123456789012345678901234567890123456789012345678901234567890
        // [No.]    [Time]   [Source]        [Destination]   [Prot]  [Len]
        //12345    4.721809  192.168.115.199 192.168.115.199  ICMP    9999
        mvwprintw(win, 0, 2, "[No.]");
        mvwprintw(win, 0, 9, "[Time]");
        mvwprintw(win, 0, 20, "[Source]");
        mvwprintw(win, 0, 36, "[Destination]");
        mvwprintw(win, 0, 52, "[Prot]");
        mvwprintw(win, 0, 60, "[Len]");
        wmove(win, 1, 1);
        refresh();
    }

    void refresh() {
        wrefresh(win);
    }

    WINDOW *win;
    int width, height;
    int max_y;
    int y, x;
};

void InitDisplay(int &height, int &width){
    initscr();            // Inicializa ncurses
    start_color();
    cbreak();             // Deshabilita el buffering de línea
    noecho();             // No mostrar la entrada de teclas
    keypad(stdscr, TRUE); // Habilitar teclas especiales
    curs_set(0);          // Ocultar el cursor
    nodelay(stdscr, TRUE);// No bloquear en getch()
    timeout(100);         // Esperar 100ms en cada iteración
    getmaxyx(stdscr, height, width);

    init_pair(1, COLOR_BLACK, COLOR_WHITE);
}

void EndDisplay() {
    curs_set(1);
    endwin();
    printf("bye bye");
    exit(0);
}

int nomain() {
    
    // FILE *fp = popen("ls -l", "r"); // Ejecutar el comando y abrir una tubería de lectura

    // // Leer la salida del comando línea por línea
    // if (fp) {
    //     char buffer[128];
    //     while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
    //         printf("%s", buffer); // Imprimir la salida del comando
    //     }
    //     pclose(fp); // Cerrar la tubería
    // }

    steady_clock::time_point start_time, end_time;
    duration<double> elapsed_seconds;

    int width, height;
    WINDOW *win1, *win2, *win3;
    vector<string> list;
    string device = "wlo1";
    // signal(SIGINT, ExitProgram);
    // signal(SIGWINCH, ExitProgram);
    // signal(SIGKILL, ExitProgram);

    initscr();            // Inicializa ncurses
    start_color();
    cbreak();             // Deshabilita el buffering de línea
    noecho();             // No mostrar la entrada de teclas
    keypad(stdscr, TRUE); // Habilitar teclas especiales
    curs_set(0);          // Ocultar el cursor
    nodelay(stdscr, TRUE);// No bloquear en getch()
    timeout(50);         // Esperar 100ms en cada iteración
    getmaxyx(stdscr, height, width);

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);

    // box(stdscr, 0, 0);
    // bkgd(COLOR_PAIR(1));
    refresh();
    // mvprintw(4, 30, "Hello World");

    int width1 = width, height1 = height / 2;
    int max_y = height1 - 2;
    win1 = newwin(height1, width1, 0, 0);
    box(win1, 0, 0);
    wbkgd(win1, COLOR_PAIR(1));
    wmove(win1, 1, 1);
    // wattron(win1, COLOR_PAIR(2));
    // mvprintw(11, 5, "main: (%d, %d)", width, height);
    // mvaddch(1, 0, 'L');

    // mvwprintw(win1, 5, 10, "win1: (%d, %d)", width, height);
        // refresh();
    // wrefresh(win1);

    int x = 0;
    int y = 0;
    start_time = steady_clock::now();
    while (true) {
        end_time = steady_clock::now();
        elapsed_seconds = duration_cast<duration<double>>(end_time - start_time);
        list.push_back(to_string(elapsed_seconds.count()));
        int length = list.size();
        int index = 0;
        
        int cur_y = length < max_y ? length : max_y;
        for (size_t y = 1; y <= cur_y; y++) {
            index = length - y;
            
            mvwprintw(win1, y, 1, list[index].c_str());
        }
        wrefresh(win1);
        
        // mvwprintw(win1, height1 - 1, 1, "(%03d,%d)", x, y);
        // mvwaddch(win1, y, x, 'L');
        // wrefresh(win1);
        // if(x < width1 - 1)
        //     x++;
        // else{
        //     x = 0;
        //     if(y < height1 - 1)
        //         y++;
        //     else {
        //         endwin();
        //         exit(0);
        //     }
                
        // }
        
        

        int ch = getch();
        switch (ch) {
            case 'q':
                // ExitProgram();
                break;
            case 'a':
                endwin();
                system("ls -l");
                break;
        }
        // napms(100);
        // getch();
    }
    // wattroff(win1, COLOR_PAIR(2));
    // getch();

    endwin();

    return 0;
}
