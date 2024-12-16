#define STATE_M 0
#define STATE_I 1
#define STATE_F 2
#define STATE_S 3

#define KEY_F1 265
#define KEY_TTAB 9
#define KEY_BTAB 353 

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

    virtual ~State() = default;

    void DrawState(){
        UpdateMenu();
        for(auto var : windows) {
            var->DrawBorder();
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
        states[prevState]->EraseState();
        // states[currentState]->UpdateMenu();
        states[currentState]->DrawState();
        states[currentState]->StateBehaviour();
    }
    
    void SetWinLink(short win, short winTarget) {
        windows[win]->AddLinkedWin(windows[winTarget]);
    }

    virtual void HandleKeyPress(short& currentState, int keyPressed) = 0;
    virtual void StateBehaviour() = 0;
    virtual void UpdateMenu() = 0;
};

class StateM : public State{
public:
    StateM(vector<BaseParentWin*> windows) : State(windows){
        menuOptions.insert({"<F1> help", {KEY_F1}});
        menuOptions.insert({"<Up> Select prev", {'W', 'w', KEY_UP}});
        menuOptions.insert({"<Down> Select next", {'S', 's', KEY_DOWN}});
        menuOptions.insert({"<TAB> focus next", {KEY_TTAB}});
        menuOptions.insert({"<shift + TAB> focus prev", {KEY_BTAB}});
        menuOptions.insert({"<P> start capturing", {'P', 'p'}});
        menuOptions.insert({"<F> Filters", {'F', 'f'}});
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
                if(packetCapture->isCapturing)
                    packetCapture->StopCapture();
                else
                    packetCapture->StartCapture();
                break;
        }
        // if(currentWin >= windows.size())
        //     currentWin = 0;
        // else if (currentWin < 0)
        //     currentWin = windows.size() - 1;
    }
    
    void StateBehaviour() override {}

    void UpdateMenu() override {
        // menuWin->PrintM("%s %s %s %s %s", );
    }

    map<string, vector<short>> menuOptions;
};

class StateI : public State{
public:
    StateI(vector<BaseParentWin*> windows) : State(windows){}
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

    void UpdateMenu() override {
        menuWin->PrintM("menu I");
    }
};

class StateF : public State{
public:
    StateF(vector<BaseParentWin*> windows) : State(windows){}
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
    }

    void UpdateMenu() override {
        menuWin->PrintM("menu F");
    }
};