class Window {
public:
    Window(int height, int width, int y, int x){
        this->height = height;
        this->width = width;
        this->y = y;
        this->x = x;
        // win = newwin(height, width, y, x);
    }

    void refresh() {
        wrefresh(win);
    }

    void erase() {
        werase(win);
    }

    WINDOW *win;
    int width, height;
    int y, x;
};

template<typename T>
class SubWindow : public Window {
public:
    SubWindow(int height, int width, int y, int x) : Window(height, width, y, x) {}
    SubWindow(int height, int width, int y, int x, void (*ListManager)(WINDOW*, vector<T>&, int, int, int)) : Window(height, width, y, x), ListManager(ListManager) {}

    List<T>* objList;
    void (*ListManager)(WINDOW*, vector<T>&, int, int, int) = nullptr;

    void SetSub(WINDOW *parent) {
        this->win = derwin(parent, this->height, this->width, this->y, this->x);
    }

    void SetList(vector<T>* list) {
        this->objList = new List<T>(*list, this->height);
    }

    void DrawList() {
        if (objList) {
            int available_rows = this->height;

            for (int i = 0; i < available_rows; i++) {
                int option_index = objList->scroll_start + i;

                if (option_index < static_cast<int>(objList->list.size())) {
                    if (option_index == objList->current_selection) {
                        wattron(this->win, A_REVERSE);
                    }

                    ListManager(this->win, objList->list, option_index, i, width);

                    wattroff(this->win, A_REVERSE);
                }
            }
            this->refresh();
        }
    }

    // void moveSelection(int direction) {
    //     if (objList) {
    //         objList->move_selection(direction);
    //     }
    // }
};

class BaseParentWin : public Window {
public:
    BaseParentWin(int height, int width, int y, int x) 
        : Window(height, width, y, x) {}

    virtual ~BaseParentWin() {}

    virtual void DrawBorder() = 0;
    virtual void DrawSubWindow() = 0;
    virtual void EraseSubWindow() = 0;
    virtual int GetSubHeight() = 0;
    virtual int GetCurrSelect() = 0;
    virtual void ResetListPosition() = 0;
    virtual void moveSelection(int direction) = 0;
    virtual void AddLinkedWin(BaseParentWin* linkedWin) = 0;
    virtual const std::vector<BaseParentWin*>& GetLinkedWins() const = 0;
    virtual void UpdateList() = 0;
};

template<typename T>
class ParentWin : public BaseParentWin {
public:
    ParentWin(int height, int width, int y, int x) : BaseParentWin(height, width, y, x), subw(height - 2, width - 2, 1, 1){
    // parentwin(int height, int width, int y, int x) : window(height, width, y, x){
        this->win = newwin(height, width, y, x);
        // subw = new subwindow(this->win, height - 2, width - 2, 1, 1);
        subw.SetSub(this->win);
    }

    ParentWin(int height, int width, int y, int x, void (*ListManager)(WINDOW*, vector<T>&, int, int, int)) : BaseParentWin(height, width, y, x), subw(height - 2, width - 2, 1, 1, ListManager){
        this->win = newwin(height, width, y, x);
        subw.SetSub(this->win);
        list = new vector<T>;
        subw.SetList(list);
    }

    ~ParentWin() {
        delete list;
    }

    void DrawBorder() override{
        box(win, 0, 0);
        refresh();
    }

    void DrawSubWindow() override {
        subw.DrawList();
    }

    void EraseSubWindow() override {
        subw.erase();
    }

    int GetSubHeight() override {
        return subw.height;
    }

    int GetCurrSelect() override {
        return subw.objList->current_selection;
    }

    void ResetListPosition() override {
        subw.objList->current_selection = 0;
        subw.objList->scroll_start = 0;
    }

    int* GetCurrIndex() {
        return subw.objList->GetCurrIndex();
    }

    void moveSelection(int direction) override {
        if (subw.objList) {
            subw.objList->move_selection(direction);
        }

        for(auto linked : linkedWins) {
            linked->UpdateList();
            linked->ResetListPosition();
            linked->EraseSubWindow();
            linked->DrawSubWindow();
        }
    }

    void AddLinkedWin(BaseParentWin* linkedWin) override{
        linkedWins.push_back(linkedWin);
    }

    const std::vector<BaseParentWin*>& GetLinkedWins() const override {
        return linkedWins;
    }

    vector<T>* GetList() {
        return list;
    }

    // template<typename U>
    // void SetLinkedlist(vector<T>* list) {
    //     if (linkedlist != nullptr) {
    //         delete linkedlist; // Liberar la lista actual, si existe
    //     }
    //     linkedlist = list;
    // }

    // Getter para ListGenerator
    void (*GetListGenerator() const)(vector<T>*) {
        return ListGenerator;
    }

    // Setter para ListGenerator
    void SetListGenerator(void (*func)(vector<T>*)) {
        ListGenerator = func;
    }

    void UpdateList() {
        if(ListGenerator) ListGenerator(list);
    }

    SubWindow<T> subw;
    vector<BaseParentWin*> linkedWins;
    // void (*ListGenerator)(vector<T>*) = nullptr;
    void (*ListGenerator)(vector<T>*) = nullptr;
    // static PacketData* currentPacket;
    vector<T>* list = nullptr;
};

void InitDisplay(int &height, int &width){
    initscr();            // Inicializa ncurses
    start_color();
    //cbreak();             // Deshabilita el buffering de línea
    raw();
    noecho();             // No mostrar la entrada de teclas
    keypad(stdscr, TRUE); // Habilitar teclas especiales
    curs_set(0);          // Ocultar el cursor
    nodelay(stdscr, TRUE);// No bloquear en getch()
    timeout(100);         // Esperar 100ms en cada iteración
    getmaxyx(stdscr, height, width);

    // init_pair(1, COLOR_BLACK, COLOR_WHITE);
}

void EndDisplay() {
    curs_set(1);
    endwin();
    printf("bye bye\n");
    // exit(0);
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

// ParentWin *win1, *win2, *win3, *win4;

void PrintTitles(){
    //         1         2         3         4         5         6         7 
    //1234567890123456789012345678901234567890123456789012345678901234567890
    // [No.]    [Time]   [Source]        [Destination]   [Prot]  [Len]
    //12345 1234.678901  192.168.115.199 192.168.115.199  ICMP  99999
    //  111   19.210  20.190.157.96   192.168.0.8       TCP   1434│  
    //01 9a 6f 54 00 00 01 01 08 0a bc 51 af 3a 15 95   ..oT.......Q.:..
    // mvwprintw(win1->win, 0, 2, "[No.]");
    // mvwprintw(win1->win, 0, 9, "[Time]");
    // mvwprintw(win1->win, 0, 20, "[Source]");
    // mvwprintw(win1->win, 0, 36, "[Destination]");
    // mvwprintw(win1->win, 0, 52, "[Prot]");
    // mvwprintw(win1->win, 0, 60, "[Len]");
    // wmove(win, 1, 1);
    // int h, w;
    // getmaxyx(win, h, w);
    // mvprintw(0, 0, "%d,%d", h, w);
    // win1->refresh();
}