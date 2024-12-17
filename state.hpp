#define STATE_M 0
#define STATE_I 1
#define STATE_F 2
#define STATE_S 3

#define KEY_F1 265
#define KEY_TTAB 9
#define KEY_BTAB 353
#define KEY_CTRL_S 19 

void save_to_csv();
void save_to_excel();

struct ShortCuts{
    string key;
    string desc;
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

    static vector<State*> states;
    static short currentState, prevState;
    // map<string, ShortCuts> menuOptions;
    vector<ShortCuts> menuOptions;
    vector<string> menuOrder;

    virtual ~State() = default;

    void DrawState(){
        UpdateMenu();
        for(auto var : windows) {
            var->DrawBorder();
            var->SetTitle();
            var->DrawSubWindow();
        }
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
            wattron(menuWin->win, A_REVERSE);
            wprintw(menuWin->win, "%s", tmp.c_str());
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
        menuOptions.push_back({"F1", "Help"});
        menuOptions.push_back({"P", "Start capturing"});
        menuOptions.push_back({"F", "Filters"});
        menuOptions.push_back({"ctrl + S", "Save log"});
        menuOptions.push_back({"Up", "Select prev"});
        menuOptions.push_back({"Down", "Select next"});
        // menuOptions.push_back({"shift + tab", "focus prev"});
        menuOptions.push_back({"Tab", "Change focus"});
    }
    void HandleKeyPress(short& currentState, int keyPressed) override {
        switch (keyPressed) {
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
                currentWin++;
                if(currentWin >= windows.size()) currentWin = 0;
                autoScroll = !currentWin;
                break;
            case KEY_BTAB:
                currentWin--;
                if(currentWin < 0) currentWin = windows.size() - 1;
                autoScroll = !currentWin;
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
                    UpdateMenu();
                }
                else {
                    packetCapture->StartCapture(true);
                    menuOptions[1].desc = "stop capturing";
                    UpdateMenu();
                }
                break;
            case KEY_CTRL_S:
                save_to_csv();
                save_to_excel();
                conWin->PrintM("Logs saved successfully");
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
        menuOptions.push_back({"F1", "help"});
        menuOptions.push_back({"Up", "Select prev"});
        menuOptions.push_back({"Down", "Select next"});
        menuOptions.push_back({"Enter", "Continue"});
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
        menuOptions.push_back({"F1", "help"});
        menuOptions.push_back({"Enter", "set Filter"});
    }
    void HandleKeyPress(short& currentState, int keyPressed) override {
        switch (keyPressed) {
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
        packetCapture->SetFilters(input);
        menuOptions[1].desc = "continue";
        UpdateMenu();
        conWin->PrintM("filter set successfully: %s", input.c_str());
    }

    void EndState() override {
        menuOptions[1].desc = "set Filter";
    }
};