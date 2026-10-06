// ==================== Headers ====================

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// ==================== Forward Declarations ====================

class Data;
class Request;
class IO;
class IOFactory;
class CMDIO;
class CMDIOFactory;
class Strategy;
class CreateDesignStrategy;
class ValidateDesignStrategy;
class Step;
class StepState;
class IdleState;
class RunningState;
class SuccessState;
class FailedState;
class StepMemento;
class Iterator;
class StepIterator;
class Collection;
class StepCollection;
class Team;
class StaffTeam;
class MiddleManagementTeam;
class ExecutiveTeam;
class Authoriser;
class StepRunner;

// ==================== Data ====================

class Data
{
private:
    string value;
public:
    Data();
    Data(string value);
    string getValue() const;
    void setValue(string value);
};

// ==================== PermLevel ====================

enum class PermLevel { LOW, MEDIUM, HIGH };

// ==================== StateKind ====================

enum class StateKind { IDLE, RUNNING, SUCCESS, FAILED };

// ==================== Request ====================

class Request
{
private:
    PermLevel permLevel;
    string action;
public:
    Request(PermLevel permLevel, string action);
    PermLevel getPermLevel() const;
    string getAction() const;
};

// ==================== IOFactory ====================

class IOFactory
{
protected:
    virtual IO* create() = 0;
    friend class IO;
public:
    virtual ~IOFactory();
};

// ==================== CMDIOFactory ====================

class CMDIOFactory : public IOFactory
{
protected:
    virtual IO* create();
public:
    virtual ~CMDIOFactory();
};

// ==================== IO ====================

class IO
{
private:
    static IO* io;
    static IOFactory* factory;
protected:
    IO();
public:
    static void setIOFactory(IOFactory* factory);
    static IO* getInstance();
    static void shutdown();
    virtual string getInput() = 0;
    virtual void setOutput(string output) = 0;
    virtual ~IO();
};

// ==================== CMDIO ====================

class CMDIO : public IO
{
protected:
    CMDIO();
    friend class CMDIOFactory;
public:
    string getInput() override;
    void setOutput(string output) override;
    ~CMDIO();
};

// ==================== Strategy ====================

class Strategy
{
private:
    PermLevel level;
public:
    Strategy(PermLevel level);
    PermLevel getLevel() const;
    virtual Data& execute(Data& data) = 0;
    virtual ~Strategy();
};

// ==================== CreateDesignStrategy ====================

class CreateDesignStrategy : public Strategy
{
public:
    CreateDesignStrategy(PermLevel level);
    virtual Data& execute(Data& data);
};

// ==================== ValidateDesignStrategy ====================

class ValidateDesignStrategy : public Strategy
{
public:
    ValidateDesignStrategy(PermLevel level);
    virtual Data& execute(Data& data);
};

// ==================== StepState ====================

class StepState
{
protected:
    Step& step;
public:
    StepState(Step& step);
    virtual bool execute(Data& data) = 0;
    virtual StateKind getKind() = 0;
    virtual ~StepState();
};

// ==================== SuccessState ====================

class SuccessState : public StepState
{
public:
    SuccessState(Step& step);
    bool execute(Data& data) override;
    StateKind getKind() override;
};

// ==================== FailedState ====================

class FailedState : public StepState
{
public:
    FailedState(Step& step);
    bool execute(Data& data) override;
    StateKind getKind() override;
};

// ==================== RunningState ====================

class RunningState : public StepState
{
public:
    RunningState(Step& step);
    bool execute(Data& data) override;
    StateKind getKind() override;
};

// ==================== IdleState ====================

class IdleState : public StepState
{
public:
    IdleState(Step& step);
    bool execute(Data& data) override;
    StateKind getKind() override;
};

// ==================== StepMemento ====================

class StepMemento
{
private:
    string stepName;
    PermLevel requiredLevel;
    StateKind stateKind;
    Data dataSnapshot;
public:
    StepMemento(string stepName,
                PermLevel requiredLevel,
                StateKind stateKind,
                Data dataSnapshot);
    string getStepName() const;
    PermLevel getRequiredLevel() const;
    StateKind getStateKind() const;
    Data getDataSnapshot() const;
};

// ==================== Step ====================

class Step
{
private:
    string name;
    Strategy* strategy;
    StepState* state;
    friend class StepState;
    friend class IdleState;
    friend class RunningState;
    friend class FailedState;
    friend class SuccessState;
    bool setStateAndExecute(StepState* newState, Data& data);
    StepState* makeState(StateKind kind);
public:
    Step(string name, Strategy* strategy);
    string getName();
    PermLevel getRequiredLevel() const;
    virtual bool execute(Data& data);
    virtual StepMemento createMemento(const Data& data);
    virtual void restoreFrom(const StepMemento& memento, Data& data);
    virtual ~Step();
};

// ==================== Collection ====================

class Collection
{
public:
    virtual Iterator* createIterator() = 0;
    virtual ~Collection();
};

// ==================== StepCollection ====================

class StepCollection : public Collection
{
private:
    vector<Step*> steps; // owns steps
    friend class StepIterator;
public:
    StepCollection(vector<Step*> steps);
    Iterator* createIterator();
    ~StepCollection();
};

// ==================== Iterator ====================

class Iterator
{
public:
    virtual Step* getNext() = 0;
    virtual ~Iterator();
};

// ==================== StepIterator ====================

class StepIterator : public Iterator
{
private:
    const StepCollection& collection;
    int currentPos;
public:
    StepIterator(StepCollection& collection);
    Step* getNext();
};

// ==================== Team ====================

class Team
{
private:
    Team* next;         // owns the rest of the chain
    PermLevel level;
public:
    Team(PermLevel level, Team* next);
    virtual bool authorise(Request& request);
    virtual string getName() = 0;
    virtual ~Team();
};

// ==================== StaffTeam ====================

class StaffTeam : public Team
{
public:
    StaffTeam(PermLevel level, Team* next);
    string getName() override;
};

// ==================== MiddleManagementTeam ====================

class MiddleManagementTeam : public Team
{
public:
    MiddleManagementTeam(PermLevel level, Team* next);
    string getName() override;
};

// ==================== ExecutiveTeam ====================

class ExecutiveTeam : public Team
{
public:
    ExecutiveTeam(PermLevel level, Team* next);
    string getName() override;
};

// ==================== Authoriser ====================

class Authoriser
{
private:
    static Authoriser* instance;
    Team* team;                       // head of the chain; owns it
    Authoriser(Team* team);
public:
    static void setTeam(Team* team);  // <-- inject from main, like IO::setIOFactory
    static Authoriser* getInstance();
    static void shutdown();           // <-- symmetric teardown
    virtual bool authorise(Request& request);
    virtual ~Authoriser();
};

// ==================== StepRunner ====================

class StepRunner
{
public:
    virtual bool runSteps(vector<Step*> steps, Data& data,
                          vector<StepMemento>& history);
    virtual ~StepRunner();
};

// ==================== Data Definitions ====================

Data::Data() : value("") {}

Data::Data(string value) : value(value) {}

string Data::getValue() const
{
    return value;
}

void Data::setValue(string value)
{
    this->value = value;
}

// ==================== Request Definitions ====================

Request::Request(PermLevel permLevel, string action)
    : permLevel(permLevel), action(action) {}

PermLevel Request::getPermLevel() const
{
    return permLevel;
}

string Request::getAction() const
{
    return action;
}

// ==================== IOFactory Definitions ====================

IOFactory::~IOFactory() = default;

// ==================== CMDIOFactory Definitions ====================

IO* CMDIOFactory::create()
{
    return new CMDIO();
}

CMDIOFactory::~CMDIOFactory() = default;

// ==================== IO Definitions ====================

IO* IO::io = nullptr;
IOFactory* IO::factory = nullptr;

IO::IO() {}

void IO::setIOFactory(IOFactory* factory)
{
    delete IO::io;
    IO::io = nullptr;
    delete IO::factory;
    IO::factory = factory;
}

IO* IO::getInstance()
{
    if (!factory)
        factory = new CMDIOFactory();
    if (!io)
        io = factory->create();
    return io;
}

void IO::shutdown()
{
    delete io;
    io = nullptr;
    delete factory;
    factory = nullptr;
}

IO::~IO() = default;

// ==================== CMDIO Definitions ====================

CMDIO::CMDIO() {}

string CMDIO::getInput()
{
    string res;
    cin >> res;
    return res;
}

void CMDIO::setOutput(string output)
{
    cout << output << endl;
}

CMDIO::~CMDIO() = default;

// ==================== Strategy Definitions ====================

Strategy::Strategy(PermLevel level) : level(level) {}

PermLevel Strategy::getLevel() const
{
    return level;
}

Strategy::~Strategy() = default;

// ==================== CreateDesignStrategy Definitions ====================

CreateDesignStrategy::CreateDesignStrategy(PermLevel level) : Strategy(level) {}

Data& CreateDesignStrategy::execute(Data& data)
{
    IO::getInstance()->setOutput("please submit a design");
    data.setValue(IO::getInstance()->getInput());
    IO::getInstance()->setOutput("design created");
    return data;
}

// ==================== ValidateDesignStrategy Definitions ====================

ValidateDesignStrategy::ValidateDesignStrategy(PermLevel level) : Strategy(level) {}

Data& ValidateDesignStrategy::execute(Data& data)
{
    IO::getInstance()->setOutput("validating design");
    if (data.getValue() == "bad")
        throw "Invalid design";
    else
        IO::getInstance()->setOutput("design validated");
    return data;
}

// ==================== StepState Definitions ====================

StepState::StepState(Step& step) : step(step) {}

StepState::~StepState() = default;

// ==================== SuccessState Definitions ====================

SuccessState::SuccessState(Step& step) : StepState(step) {}

bool SuccessState::execute(Data& data)
{
    return true;
}

StateKind SuccessState::getKind()
{
    return StateKind::SUCCESS;
}

// ==================== FailedState Definitions ====================

FailedState::FailedState(Step& step) : StepState(step) {}

bool FailedState::execute(Data& data)
{
    return false;
}

StateKind FailedState::getKind()
{
    return StateKind::FAILED;
}

// ==================== RunningState Definitions ====================

RunningState::RunningState(Step& step) : StepState(step) {}

bool RunningState::execute(Data& data)
{
    try
    {
        step.strategy->execute(data);
        return step.setStateAndExecute(new SuccessState(step), data);
    }
    catch(...)
    {
        return step.setStateAndExecute(new FailedState(step), data);
    }
}

StateKind RunningState::getKind()
{
    return StateKind::RUNNING;
}

// ==================== IdleState Definitions ====================

IdleState::IdleState(Step& step) : StepState(step) {}

bool IdleState::execute(Data& data)
{
    return step.setStateAndExecute(new RunningState(step), data);
}

StateKind IdleState::getKind()
{
    return StateKind::IDLE;
}

// ==================== StepMemento Definitions ====================

StepMemento::StepMemento(string stepName,
                         PermLevel requiredLevel,
                         StateKind stateKind,
                         Data dataSnapshot)
    : stepName(stepName),
      requiredLevel(requiredLevel),
      stateKind(stateKind),
      dataSnapshot(dataSnapshot) {}

string StepMemento::getStepName() const
{
    return stepName;
}

PermLevel StepMemento::getRequiredLevel() const
{
    return requiredLevel;
}

StateKind StepMemento::getStateKind() const
{
    return stateKind;
}

Data StepMemento::getDataSnapshot() const
{
    return dataSnapshot;
}

// ==================== Step Definitions ====================

Step::Step(string name, Strategy* strategy)
    : name(name), strategy(strategy), state(new IdleState(*this)) {}

string Step::getName()
{
    return name;
}

PermLevel Step::getRequiredLevel() const
{
    return strategy->getLevel();
}

bool Step::setStateAndExecute(StepState* newState, Data& data)
{
    StepState* oldState = state;
    state = newState;
    delete oldState;
    return state->execute(data);
}

StepState* Step::makeState(StateKind kind)
{
    switch (kind)
    {
    case StateKind::IDLE:    return new IdleState(*this);
    case StateKind::RUNNING: return new RunningState(*this);
    case StateKind::SUCCESS: return new SuccessState(*this);
    case StateKind::FAILED:  return new FailedState(*this);
    }
    return new IdleState(*this);
}

bool Step::execute(Data& data)
{
    Request request(getRequiredLevel(), getName());
    if (!Authoriser::getInstance()->authorise(request))
    {
        IO::getInstance()->setOutput("Unauthorized: " + getName());
        return false;
    }
    return state->execute(data);
}

StepMemento Step::createMemento(const Data& data)
{
    return StepMemento(getName(), getRequiredLevel(), state->getKind(), data);
}

void Step::restoreFrom(const StepMemento& memento, Data& data)
{
    StepState* oldState = state;
    state = makeState(memento.getStateKind());
    delete oldState;
    data = memento.getDataSnapshot();
}

Step::~Step()
{
    delete state;
    delete strategy;
}

// ==================== Collection Definitions ====================

Collection::~Collection() = default;

// ==================== StepCollection Definitions ====================

StepCollection::StepCollection(vector<Step*> steps) : steps(steps) {}

Iterator* StepCollection::createIterator()
{
    return new StepIterator(*this);
}

StepCollection::~StepCollection()
{
    for (auto& step : steps)
        delete step;
}

// ==================== Iterator Definitions ====================

Iterator::~Iterator() = default;

// ==================== StepIterator Definitions ====================

StepIterator::StepIterator(StepCollection& collection) : collection(collection), currentPos(0) {}

Step* StepIterator::getNext()
{
    if (currentPos < collection.steps.size())
        return collection.steps[currentPos++];
    else
        return NULL;
}

// ==================== Team Definitions ====================

Team::Team(PermLevel level, Team* next) : next(next), level(level) {}

bool Team::authorise(Request& request)
{
    if (request.getPermLevel() == level)
    {
        IO::getInstance()->setOutput(getName());
        IO::getInstance()->setOutput("authorisation requested for action");
        IO::getInstance()->setOutput(request.getAction());
        string res = IO::getInstance()->getInput();
        return res.find('y') != string::npos;
    }
    else if (next)
        return next->authorise(request);
    else
        return false;
}

Team::~Team()
{
    delete next;
}

// ==================== StaffTeam Definitions ====================

StaffTeam::StaffTeam(PermLevel level, Team* next) : Team(level, next) {}

string StaffTeam::getName()
{
    return "Staff";
}

// ==================== MiddleManagementTeam Definitions ====================

MiddleManagementTeam::MiddleManagementTeam(PermLevel level, Team* next)
    : Team(level, next) {}

string MiddleManagementTeam::getName()
{
    return "Middle management";
}

// ==================== ExecutiveTeam Definitions ====================

ExecutiveTeam::ExecutiveTeam(PermLevel level, Team* next) : Team(level, next) {}

string ExecutiveTeam::getName()
{
    return "Executive";
}

// ==================== Authoriser Definitions ====================

Authoriser* Authoriser::instance = nullptr;

Authoriser::Authoriser(Team* team) : team(team) {}

void Authoriser::setTeam(Team* team)
{
    // If an Authoriser already exists, drop it — it was wired to a different
    // chain. The next getInstance() will build a fresh one from the new chain.
    delete instance;
    instance = nullptr;

    // Store the new chain head; getInstance() will pick it up on first use.
    // To keep the "pending team" separate from the live singleton, we
    // temporarily stash it in a static. Simpler alternative: construct the
    // Authoriser immediately here.
    instance = new Authoriser(team);
}

Authoriser* Authoriser::getInstance()
{
    if (!instance)
    {
        // No chain was injected: build the default one.
        Team* executives = new ExecutiveTeam(PermLevel::HIGH, nullptr);
        Team* middleManagement = new MiddleManagementTeam(PermLevel::MEDIUM, executives);
        Team* staff = new StaffTeam(PermLevel::LOW, middleManagement);
        instance = new Authoriser(staff);
    }
    return instance;
}

void Authoriser::shutdown()
{
    delete instance;
    instance = nullptr;
}

bool Authoriser::authorise(Request& request)
{
    return team->authorise(request);
}

Authoriser::~Authoriser()
{
    delete team;
}

// ==================== StepRunner Definitions ====================

bool StepRunner::runSteps(vector<Step*> steps, Data& data,
                          vector<StepMemento>& history)
{
    Collection* collection = new StepCollection(steps);
    Iterator* iter = collection->createIterator();
    Step* next;
    bool ok = true;

    while ((next = iter->getNext()))
    {
        history.push_back(next->createMemento(data));

        if (!next->execute(data))
        {
            ok = false;
            break;
        }
    }

    IO::getInstance()->setOutput("=== step history ===");
    for (auto& m : history)
    {
        string stateName;
        switch (m.getStateKind())
        {
        case StateKind::IDLE:    stateName = "Idle";    break;
        case StateKind::RUNNING: stateName = "Running"; break;
        case StateKind::SUCCESS: stateName = "Success"; break;
        case StateKind::FAILED:  stateName = "Failed";  break;
        }
        IO::getInstance()->setOutput(
            m.getStepName() + " was in state " + stateName +
            " with data \"" + m.getDataSnapshot().getValue() + "\"");
    }

    delete iter;
    delete collection;
    return ok;
}

StepRunner::~StepRunner() = default;

// ==================== Main ====================

int main()
{
    // Wire up the authorisation chain from main, exactly like IO::setIOFactory.
    // Staff -> MiddleManagement -> Executive, each handling one level.
    Team* executives = new ExecutiveTeam(PermLevel::HIGH, nullptr);
    Team* middleManagement = new MiddleManagementTeam(PermLevel::MEDIUM, executives);
    Team* staff = new StaffTeam(PermLevel::LOW, middleManagement);

    Authoriser::setTeam(staff);   // <-- injection, mirrors IO::setIOFactory

    Data data("initial data");

    vector<Step*> steps;
    steps.push_back(new Step("step1", new CreateDesignStrategy(PermLevel::LOW)));
    steps.push_back(new Step("step2", new ValidateDesignStrategy(PermLevel::MEDIUM)));

    StepRunner runner;
    vector<StepMemento> history;
    bool ok = runner.runSteps(steps, data, history);

    IO::getInstance()->setOutput(ok ? "pipeline succeeded" : "pipeline failed");
    IO::getInstance()->setOutput("final data: " + data.getValue());

    // ---- Demonstrate full restore ----
    IO::getInstance()->setOutput("=== restoring steps from mementos ===");

    vector<Step*> restoredSteps;
    restoredSteps.push_back(new Step("step1", new CreateDesignStrategy(PermLevel::LOW)));
    restoredSteps.push_back(new Step("step2", new ValidateDesignStrategy(PermLevel::MEDIUM)));

    Data restoredData("initial data");
    for (size_t i = 0; i < history.size() && i < restoredSteps.size(); ++i)
    {
        restoredSteps[i]->restoreFrom(history[i], restoredData);
        IO::getInstance()->setOutput(
            "restored " + restoredSteps[i]->getName() +
            " with data \"" + restoredData.getValue() + "\"");
    }

    for (auto* s : restoredSteps)
        delete s;

    // Symmetric teardown: Authoriser first (it owns the chain), then IO.
    Authoriser::shutdown();
    IO::shutdown();
}
