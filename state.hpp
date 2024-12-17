#define STATE_M 0
#define STATE_I 1
#define STATE_F 2
#define STATE_H 3

#define KEY_F1 265
#define KEY_TTAB 9
#define KEY_BTAB 353
#define KEY_CTRL_S 19 

void save_to_csv();
void save_to_excel();

struct ShortCuts{
    string key;
    string desc;
    attr_t colorPair;
    // vector<short> keys;
};

class State {
public:
    State(vector<BaseParentWin*> windows) : windows(windows) {
        // mainWin = new Window(max_y - 2, max_x, 1, 0);
        // for(auto window : windows) {
        //     window->CreateWin(mainWin->win);
        // }
    }
    
    // Window* mainWin;
    vector<BaseParentWin*> windows;
    short currentWin = 0;
    short prevWin;

    static vector<State*> states;
    static short currentState, prevState;
    // map<string, ShortCuts> menuOptions;
    vector<ShortCuts> menuOptions;
    vector<string> menuOrder;

    virtual ~State() = default;

    void DrawState(){
        UpdateMenu();
        for (size_t i = 0; i < windows.size(); i++){
            auto var = windows[i];
            var->DrawBorder(currentWin == i? COLOR_PAIR(COLORS::RED) : COLOR_PAIR(COLORS::NORMAL));
            var->SetTitle();
            var->DrawSubWindow();
        }
        
        // for(auto var : windows) {
        //     var->DrawBorder(COLORS::NORMAL);
        //     var->SetTitle();
        //     var->DrawSubWindow();
        // }
    }

    void EraseState() {
        for(auto parent : windows) {
            parent->erase();
            parent->EraseSubWindow();
            parent->refresh();
        }
    }

    void ChangeState(short stateID) {
        prevState = currentState;
        currentState = stateID;
        states[prevState]->EndState();
        states[prevState]->EraseState();
        // states[currentState]->UpdateMenu();
        states[currentState]->DrawState();
        states[currentState]->StateBehaviour();
    }
    
    void SetWinLink(short win, short winTarget) {
        windows[win]->AddLinkedWin(windows[winTarget]);
    }

    // void AddMenuOption(const string& key, const string& desc, const vector<short>& keys) {
    //     menuOptions.insert({key, {desc, keys}});
    //     menuOrder.push_back(key);
    // }

    void UpdateMenu() {
        menuWin->erase();
        wmove(menuWin->win, 0, 1);
        for (const auto& shortcut : menuOptions) {
            string tmp = ' ' + shortcut.key + ": " + shortcut.desc + ' ';
            wattron(menuWin->win, shortcut.colorPair);
            wprintw(menuWin->win, "%s", tmp.c_str());
            wattroff(menuWin->win, shortcut.colorPair);
            wmove(menuWin->win, 0, getcurx(menuWin->win) + 1);
        }
        menuWin->refresh();
    }

    virtual void HandleKeyPress(short& currentState, int keyPressed) = 0;
    virtual void StateBehaviour() = 0;
    virtual void EndState() = 0;
    // virtual void UpdateMenu() = 0;
};

class StateM : public State{
public:
    StateM(vector<BaseParentWin*> windows) : State(windows){
        menuOptions.push_back({"F1", "Help", COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"P", "Start capturing", COLOR_PAIR(COLORS::GREEN_INV)});
        menuOptions.push_back({"F", "Filters", COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"ctrl+S", "Save log", COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"Up", "Select prev", COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"Down", "Select next", COLOR_PAIR(COLORS::NORMAL_INV)});
        // menuOptions.push_back({"shift + tab", "focus prev", COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"Tab", "Change focus", COLOR_PAIR(COLORS::NORMAL_INV)});
    }
    void HandleKeyPress(short& currentState, int keyPressed) override {
        switch (keyPressed) {
            case KEY_F1:
                ChangeState(STATE_H);
                break;
            case 'W':
            case 'w':
            case KEY_UP:
                if(!currentWin) autoScroll = false;
                windows[currentWin]->moveSelection(-1);
                windows[currentWin]->DrawSubWindow();
                break;
            case 'S':
            case 's':
            case KEY_DOWN:
                if(!currentWin) autoScroll = false;
                windows[currentWin]->moveSelection(1);
                windows[currentWin]->DrawSubWindow();
                break;
            case KEY_HOME:
                if(!currentWin) autoScroll = false;
                windows[currentWin]->SelectFirst();
                windows[currentWin]->moveSelection(-1);
                windows[currentWin]->DrawSubWindow();
                break;
            case KEY_END:
                if(!currentWin) autoScroll = true;
                windows[currentWin]->SelectLast();
                windows[currentWin]->moveSelection(1);
                windows[currentWin]->DrawSubWindow();
                // current_selection = packets.size() - 1;
                // move_selection(1);
                break;
            case KEY_TTAB:
                prevWin = currentWin++;
                if(currentWin >= windows.size()) currentWin = 0;
                autoScroll = !currentWin;
                windows[prevWin]->DrawBorder(COLOR_PAIR(COLORS::NORMAL));
                windows[prevWin]->SetTitle();
                windows[currentWin]->DrawBorder(COLOR_PAIR(COLORS::RED));
                windows[currentWin]->SetTitle();
                break;
            case KEY_BTAB:
                prevWin = currentWin--;
                if(currentWin < 0) currentWin = windows.size() - 1;
                autoScroll = !currentWin;
                windows[prevWin]->DrawBorder(COLOR_PAIR(COLORS::NORMAL));
                windows[prevWin]->SetTitle();
                windows[currentWin]->DrawBorder(COLOR_PAIR(COLORS::RED));
                windows[currentWin]->SetTitle();
                break;
            case 'F':
            case 'f':
                ChangeState(STATE_F);
                break;
            case 'P':
            case 'p':
                if(packetCapture->isCapturing){
                    packetCapture->StopCapture();
                    menuOptions[1].desc = "start capturing";
                    menuOptions[1].colorPair = COLOR_PAIR(COLORS::GREEN_INV);
                    UpdateMenu();
                }
                else {
                    packetCapture->StartCapture(true);
                    menuOptions[1].desc = "stop capturing";
                    menuOptions[1].colorPair = COLOR_PAIR(COLORS::RED_INV);
                    UpdateMenu();
                    windows[0]->EraseSubWindow();
                    packets->clear();
                    windows[0]->SelectFirst();
                }
                break;
            case KEY_CTRL_S:
                save_to_csv();
                save_to_excel();
                conWin->PrintM("Logs saved successfully");
                break;
            case KEY_BACKSPACE:
                EraseState();
                DrawState();
                break;
        }
        // if(currentWin >= windows.size())
        //     currentWin = 0;
        // else if (currentWin < 0)
        //     currentWin = windows.size() - 1;
    }
    
    void StateBehaviour() override {}
    void EndState() {}
};

class StateI : public State{
public:
    StateI(vector<BaseParentWin*> windows) : State(windows){
        menuOptions.push_back({"F1", "help", COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"Up", "Select prev", COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"Down", "Select next ",COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"Enter", "Continue", COLOR_PAIR(COLORS::NORMAL_INV)});
    }
    void HandleKeyPress(short& currentState, int keyPressed) override {
        switch (keyPressed) {
            case KEY_F1:
                ChangeState(STATE_H);
                break;
            case 'W':
            case 'w':
            case KEY_UP:
                this->windows[0]->moveSelection(-1);
                this->windows[0]->DrawSubWindow();
                break;
            case 'S':
            case 's':
            case KEY_DOWN:
                this->windows[0]->moveSelection(1);
                this->windows[0]->DrawSubWindow();
                break;
            case 10:
                vector<string>* tmp = dynamic_cast<ParentWin<string>*>(this->windows[currentWin])->GetList();
                // devname = tmp->at(this->windows[currentWin]->GetCurrSelect()).c_str();
                packetCapture->currDevName = tmp->at(this->windows[currentWin]->GetCurrSelect());
                packetCapture->open();
                // SetupPCAP(tmp->at(this->windows[currentWin]->GetCurrSelect()).c_str());
                ChangeState(STATE_M);
                // mvprintw(0,0, "StateM");
                break;
        }
    }

    void StateBehaviour() override {}
    void EndState() {}
};

class StateF : public State{
public:
    StateF(vector<BaseParentWin*> windows) : State(windows){
        menuOptions.push_back({"F1", "help", COLOR_PAIR(COLORS::NORMAL_INV)});
        menuOptions.push_back({"Enter", "set Filter", COLOR_PAIR(COLORS::NORMAL_INV)});
    }
    void HandleKeyPress(short& currentState, int keyPressed) override {
        switch (keyPressed) {
            case KEY_F1:
                ChangeState(STATE_H);
                break;
            case 10:
                ChangeState(STATE_M);
                break;
        }
    }

    void StateBehaviour() override {
        echo();
        nodelay(stdscr, false);
        curs_set(1);
        string input = windows[0]->EnableTypeMode();
        noecho();
        nodelay(stdscr, true);
        curs_set(0);
        // int FilterStatus =  packetCapture->SetFilters(input);
        if(packetCapture->SetFilters(input)) conWin->PrintM("filter set successfully: %s", input.c_str());
        menuOptions[1].desc = "continue";
        UpdateMenu();
    }

    void EndState() override {
        menuOptions[1].desc = "set Filter";
    }
};

class StateH : public State{
public:
    StateH(vector<BaseParentWin*> windows) : State(windows){
        menuOptions.push_back({"Enter", "Return", COLOR_PAIR(COLORS::NORMAL_INV)});
    }
    void HandleKeyPress(short& currentState, int keyPressed) override {
        switch (keyPressed) {
            case 'W':
            case 'w':
            case KEY_UP:
                this->windows[0]->moveSelection(-1);
                this->windows[0]->DrawSubWindow();
                break;
            case 'S':
            case 's':
            case KEY_DOWN:
                this->windows[0]->moveSelection(1);
                this->windows[0]->DrawSubWindow();
                break;
            case 10:
                ChangeState(prevState);
                break;
        }
    }

    void StateBehaviour() override {}

    void EndState() override {}
};