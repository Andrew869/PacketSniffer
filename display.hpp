// class BaseWinParam {
// public:
//     int width, height;
//     int y, x;
// };

// template<typename T>
// struct WinParam : public BaseWinParam {
//     void (*ListGenerator)(vector<T>*) = nullptr;
// };

enum COLORS {
    NORMAL = 1,
    NORMAL_INV,
    RED,
    RED_INV,
    GREEN,
    GREEN_INV
};


struct Title {
    int x;
    string text;
};

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

class BasicWin : public Window {
public:
    BasicWin(int height, int width, int y, int x) : Window(height, width, y, x) {
        win = newwin(height, width, y, x);
    }

    void PrintM(const char* msg, ...){
        erase();
        va_list args;                // Inicializa la lista de argumentos variables
        va_start(args, msg);         // Comienza a procesar los argumentos a partir de 'msg'
        vw_printw(win, msg, args);   // Usa vw_printw para manejar la lista de argumentos variables
        va_end(args);                // Finaliza el procesamiento de argumentos
        refresh();
    }
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
    BaseParentWin(int height, int width, int y, int x, vector<Title> *title) : Window(height, width, y, x), title(title) {
        // SetTitle(title);
    }

    virtual ~BaseParentWin() {}

    void SetTitle() {
        for(auto &var : (*title)) {
            mvwprintw(this->win, 0, var.x, var.text.c_str());
            this->refresh();
        }
    }

    // virtual void CreateWin(WINDOW* win) = 0;
    virtual void DrawBorder(attr_t colorPair) = 0;
    virtual void DrawSubWindow() = 0;
    virtual void EraseSubWindow() = 0;
    virtual int GetSubHeight() = 0;
    virtual int GetCurrSelect() = 0;
    virtual void SelectLast() = 0;
    virtual void SelectFirst() = 0;
    virtual void moveSelection(int direction) = 0;
    virtual void AddLinkedWin(BaseParentWin* linkedWin) = 0;
    virtual const vector<BaseParentWin*>& GetLinkedWins() const = 0;
    virtual void UpdateList() = 0;
    // virtual void EnableTypeMode() = 0;
    virtual string EnableTypeMode() = 0;
    // virtual char* GetInput() = 0;

    vector<Title> *title;
};

BasicWin *menuWin, *mainWin, *conWin;

template<typename T>
class ParentWin : public BaseParentWin {
public:
    ParentWin(int height, int width, int y, int x, vector<Title> *title) : BaseParentWin(height, width, y, x, title), subw(height - 2, width - 2, 1, 1){
        this->win = derwin(mainWin->win, height, width, y, x);
        subw.SetSub(this->win);
    }

    ParentWin(int height, int width, int y, int x, vector<Title> *title, void (*ListManager)(WINDOW*, vector<T>&, int, int, int)) : BaseParentWin(height, width, y, x, title), subw(height - 2, width - 2, 1, 1, ListManager){
        this->win = derwin(mainWin->win, height, width, y, x);
        subw.SetSub(this->win);
        list = new vector<T>;
        subw.SetList(list);
    }

    // void CreateWin(WINDOW* win) {
    //     this->win = derwin(win, height, width, y, x);
    //     subw.SetSub(this->win);
    // }

    ~ParentWin() {
        delete list;
    }

    void DrawBorder(attr_t colorPair) override{
        wattron(this->win, colorPair);
        box(win, 0, 0);
        wattroff(this->win, colorPair);
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

    void SelectLast() override {
        subw.objList->current_selection = list->size() - 1;
        subw.objList->move_selection(1);
    }

    void SelectFirst() override {
        subw.objList->current_selection = 0;
        subw.objList->move_selection(-1);
    }

    int* GetCurrIndex() {
        return subw.objList->GetCurrIndex();
    }

    void moveSelection(int direction) override {
        if (!subw.objList || list->size() <= 1) return;
        subw.objList->move_selection(direction);

        for(auto linked : linkedWins) {
            linked->UpdateList();
            linked->SelectFirst();
            linked->EraseSubWindow();
            linked->DrawSubWindow();
        }
    }

    void AddLinkedWin(BaseParentWin* linkedWin) override{
        linkedWins.push_back(linkedWin);
    }

    const vector<BaseParentWin*>& GetLinkedWins() const override {
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

    // void EnableTypeMode() {
    //     wmove(subw.win, 0, 0);
    //     memset(input, '\0', sizeof(input));
    //     wscanw(subw.win, "%s", &input);
    // }

    string EnableTypeMode() {
        string input;  // Cadena para almacenar el texto ingresado
        int cursor_x = 0;   // Posición inicial del cursor
        int cursor_y = 0;

        keypad(subw.win, TRUE);

        wmove(subw.win, cursor_y, cursor_x);  // Colocar el cursor al inicio
        wrefresh(subw.win);

        while (true) {
            int ch = wgetch(subw.win);  // Leer una tecla

            switch (ch) {
                case 27:  // ESC para salir
                    return input;

                case KEY_BACKSPACE:  // Retroceso
                case 127:            // Compatibilidad con teclados
                    if (cursor_x > 0) {
                        input.pop_back();
                        cursor_x--;
                        mvwaddch(subw.win, cursor_y, cursor_x, ' ');  // Borra el carácter
                        wmove(subw.win, cursor_y, cursor_x);          // Mueve el cursor atrás
                    }
                    break;

                case KEY_LEFT:  // Mover cursor a la izquierda
                    if (cursor_x > 0) cursor_x--;
                    wmove(subw.win, cursor_y, cursor_x);
                    break;

                case KEY_RIGHT:  // Mover cursor a la derecha
                    if (cursor_x < (int)input.size()) cursor_x++;
                    wmove(subw.win, cursor_y, cursor_x);
                    break;

                case '\n':  // Enter (finalizar entrada)
                    return input;

                default:  // Cualquier otra tecla (añadir texto)
                    if (isprint(ch) && cursor_x < subw.width) {
                        input.push_back(ch);
                        mvwaddch(win, cursor_y, cursor_x, ch);
                        cursor_x++;
                    }
                    break;
            }

            wrefresh(subw.win);  // Refresca la ventana para mostrar cambios
        }
    }

    // char* GetInput() {
    //     return input;
    // }

    SubWindow<T> subw;
    vector<BaseParentWin*> linkedWins;
    // void (*ListGenerator)(vector<T>*) = nullptr;
    void (*ListGenerator)(vector<T>*) = nullptr;
    // static PacketData* currentPacket;
    vector<T>* list = nullptr;
    // char input[100];
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

    init_pair(COLORS::NORMAL, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLORS::NORMAL_INV, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLORS::RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLORS::RED_INV, COLOR_BLACK, COLOR_RED);
    init_pair(COLORS::GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLORS::GREEN_INV, COLOR_BLACK, COLOR_GREEN);
}

void EndDisplay() {
    curs_set(1);
    endwin();
    printf("bye bye\n");
    // exit(0);
}

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