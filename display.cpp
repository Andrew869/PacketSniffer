#include <ncurses.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <ctime>    // Para srand y time
#include <cstdlib>
#include <chrono>

using namespace std;
using namespace chrono;

void ExitProgram();
void ExitProgram(int signum);

int main() {
    
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
    signal(SIGINT, ExitProgram);
    signal(SIGWINCH, ExitProgram);
    signal(SIGKILL, ExitProgram);

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
                ExitProgram();
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

void ExitProgram(int signum) {
    ExitProgram();
}

void ExitProgram() {
    curs_set(1);
    endwin();
    printf("bye bye");
    exit(0);
}