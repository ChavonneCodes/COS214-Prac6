/**
 * @file live_server.cpp
 * @brief Live console: lets a person drive the real WorkflowEngine from a browser.
 *
 * gui/index.html replays a recorded run. This program serves gui/live.html and
 * answers its requests with the real engine, so nothing is scripted: a person
 * starts any process, inspects the live tree, and takes any action they like.
 * The State objects allow or refuse each action, not this file or the page.
 *
 * Build and run from the repo folder: make live
 */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "AllowedStatesRuntimeDecorator.h"
#include "AssignCommand.h"
#include "CancelCommand.h"
#include "ConditionalRouting.h"
#include "DemoRunner.h"
#include "EscalateCommand.h"
#include "FailCommand.h"
#include "LoggingRuntimeDecorator.h"
#include "Participant.h"
#include "RejectCommand.h"
#include "Request.h"
#include "RoleBasedAssignment.h"
#include "RuleBasedAssignment.h"
#include "SecurityRuntimeDecorator.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"
#include "ValidationApprovalHandler.h"
#include "WorkloadBasedAssignment.h"
#include "makeAvailableCommand.h"

namespace {

const int PORT = 8214;
const int EXECUTIVE_LIMIT = 500000; ///< Budgets above this (in rand) need executive sign-off.

typedef std::map<std::string, std::string> Params;
typedef std::vector<std::unique_ptr<ApprovalHandler>> Chain;

/** @brief Quotes a string as a JSON value. */
std::string quote(const std::string& text) {
    std::string out = "\"";
    for (char c : text) {
        if (c == '"' || c == '\\') out += std::string("\\") + c;
        else if (c == '\n') out += "\\n";
        else if (c == '\r' || c == '\t') out += ' ';
        else out += c;
    }
    return out + "\"";
}

/** @brief Writes a list of strings as a JSON array. */
std::string quoteAll(const std::vector<std::string>& lines) {
    std::string out = "[";
    for (size_t i = 0; i < lines.size(); ++i) out += (i ? "," : "") + quote(lines[i]);
    return out + "]";
}

/** @brief Joins finished JSON objects into an array. */
std::string array(const std::vector<std::string>& objects) {
    std::string out = "[";
    for (size_t i = 0; i < objects.size(); ++i) out += (i ? "," : "") + objects[i];
    return out + "]";
}

/** @brief One line of a case's story. Nothing is ever removed, so undone work stays visible. */
struct FeedLine {
    std::string kind;    ///< action, refused, route, undo, approval or info.
    std::string text;    ///< What happened.
    std::string pattern; ///< Design pattern doing the work.
};

/** @brief What the routing depends on when a step completes. */
struct Decision {
    std::vector<std::string> options;                        ///< Outcomes a person chooses from.
    std::function<std::string(const std::string&)> rule;     ///< Works the outcome out from case data instead.
    std::map<std::string, std::vector<std::string>> skips;   ///< Steps an outcome leaves behind.
};

/** @brief Everything the console needs to know about one process, as data. */
struct Process {
    std::string id, name, firstStep;
    std::vector<int> versions;
    bool hasBudget = false;
    bool guarded = false;                                      ///< Decorates the tree with Security checks.
    std::string approvalRole;                                  ///< Role that signs off the approval steps.
    std::vector<std::string> approvalSteps;                    ///< Steps that go through the approval chain.
    std::string deadlineStep;                                  ///< Step that can miss its deadline (may be empty).
    std::map<std::string, Decision> decisions;                 ///< Steps with more than one way out.
    std::map<std::string, std::shared_ptr<RoutingStrategy>> routers; ///< Steps that do not route sequentially.
    std::vector<Branch> branches;                              ///< Those routers' arrows.
};

/** @brief A running case: the engine's instance plus where routing has got to. */
struct Case {
    std::string id, title, processId, budget;
    int version = 1;
    Case* parent = nullptr;                  ///< Case waiting on this one, for a sub-process.
    std::string callStep;                    ///< Step of the parent that this case runs.
    AssignmentStrategy* strategy = nullptr;  ///< Assignment rule (not owned, may be null).
    bool finished = false;
    std::vector<std::string> active;         ///< Steps routing has reached and not left.
    std::vector<std::vector<std::string>> undoStack; ///< Active steps before each undoable command.
    std::vector<FeedLine> feed;
    std::vector<std::string> taken;          ///< Arrows routing has followed, as "from>to".
    Chain chain;
    SecurityRuntimeDecorator* guard = nullptr;
};

/** @brief Reads the live tree into a flat list of JSON nodes. */
class TreeReader : public RuntimeVisitor {
public:
    typedef std::function<std::string(const std::string&, WorkItem&)> ItemExtras;

    TreeReader(const std::string& caseId, ItemExtras extras) : caseId(caseId), extras(extras) {}

    void visitWorkItem(WorkItem& item) override {
        Participant* who = item.getAssignedParticipant();
        add("item", item.getId(), item.getName(), item.getState(),
            ",\"who\":" + quote(who ? who->getName() : "") + extras(caseId, item));
    }
    void visitStage(StageInstance& stage) override {
        add("stage", stage.getId(), stage.getName(), "", "");
        ++depth;
    }
    void leaveStage(StageInstance& /*stage*/) override { --depth; }
    void visitSubProcess(SubProcessInstance& subProcess) override {
        add("sub", subProcess.getId(), subProcess.getId(), subProcess.getState(), "");
        if (WorkflowInstance* child = subProcess.getChildInstance()) {
            std::string outer = caseId;
            caseId = child->getId();
            ++depth;
            child->getRoot()->accept(*this);
            --depth;
            caseId = outer;
        }
    }
    const std::vector<std::string>& getNodes() const { return nodes; }

private:
    std::string caseId;
    ItemExtras extras;
    std::vector<std::string> nodes;
    int depth = 0;

    void add(const std::string& kind, const std::string& id, const std::string& name,
             const std::string& state, const std::string& rest) {
        nodes.push_back("{\"kind\":" + quote(kind) + ",\"depth\":" + std::to_string(depth) +
                        ",\"case\":" + quote(caseId) + ",\"id\":" + quote(id) + ",\"name\":" + quote(name) +
                        ",\"state\":" + quote(state) + rest + "}");
    }
};

/** @brief One engine and every case running on it. "Start over" throws the whole session away. */
class Session {
public:
    Session() : engine(&repository, &bus, &history, &audit) {
        bus.attach(&audit);
        addPeople();
        defineProcesses();
        strategies["Least busy person"] = &workload;
        strategies["First person with the role"] = &byRole;
        strategies["Senior staff only (level 2+)"] = &senior;
        strategies["Nobody: leave work unassigned"] = nullptr;

        commands["assign"] = [](WorkItem* item, Participant* who) { return new AssignCommand(item, who); };
        commands["available"] = [](WorkItem* item, Participant*) { return new makeAvailableCommand(item); };
        commands["start"] = [](WorkItem* item, Participant*) { return new StartCommand(item); };
        commands["complete"] = [](WorkItem* item, Participant*) { return new CompleteCommand(item); };
        commands["reject"] = [](WorkItem* item, Participant*) { return new RejectCommand(item); };
        commands["escalate"] = [](WorkItem* item, Participant*) { return new EscalateCommand(item); };
        commands["fail"] = [](WorkItem* item, Participant*) { return new FailCommand(item); };
        commands["cancel"] = [](WorkItem* item, Participant*) { return new CancelCommand(item); };

        afterCommand["complete"] = [this](Case& c, WorkItem* item, const std::string& outcome) { route(c, item->getId(), outcome); };
        afterCommand["cancel"] = [this](Case& c, WorkItem*, const std::string&) { stop(c); };
    }

    /** @brief Starts a new case of a process and returns its id. */
    std::string start(const Params& params) {
        const Process& process = processes.at(params.at("process"));
        int version = std::stoi(params.at("version"));
        std::string budget = params.at("budget");
        if (process.hasBudget) std::stoi(budget);   // throws on anything that is not a number

        std::unique_ptr<Case> created(new Case());
        Case& c = *created;
        c.processId = process.id;
        c.version = version;
        c.budget = process.hasBudget ? budget : "";
        c.strategy = strategies.at(params.at("rule"));

        engine.setAssignment(c.strategy, everyone);
        engine.setTreeWrapper(process.guarded ? guardWrapper(c) : std::function<RuntimeComponent*(RuntimeComponent*)>());
        c.id = engine.startWorkflow(process.id, version);
        if (c.id.empty()) throw std::runtime_error(process.name + " has no version " + std::to_string(version));
        WorkflowInstance* instance = engine.getInstance(c.id);
        instance->getData().setValue(c.budget);
        c.title = process.name + " v" + std::to_string(version);
        c.chain = approvalChain(process, c);
        say(c, "info", "Case started from " + process.id + " v" + std::to_string(version) + ": its own tree, its own history",
            "Facade + Factory Method");
        enter(c, process.firstStep);
        cases.push_back(std::move(created));
        return c.id;
    }

    /** @brief Runs one action on one work item. The item's state decides whether it happens. */
    void act(const Params& params) {
        Case& c = find(params.at("case"));
        WorkflowInstance* instance = engine.getInstance(c.id);
        const std::string itemId = params.at("item"), action = params.at("action");
        WorkItem* item = instance->findWorkItem(itemId);
        if (!item) throw std::runtime_error("There is no work item " + itemId + " in this case");
        if (std::find(c.active.begin(), c.active.end(), itemId) == c.active.end()) {
            say(c, "refused", "Refused: " + action + " on " + item->getName() + ". Routing has not made this step available",
                "Strategy (routing)");
            return;
        }

        std::string outcome = param(params, "outcome");
        auto chosen = commands.at(action);
        if (action == "complete" && item->getState() == "In_Progress" && !prepareCompletion(c, item, outcome)) return;

        const std::vector<std::string> before = c.active;
        Participant* who = person(param(params, "who"));
        const std::string from = item->getState();
        if (!engine.submit(chosen(item, who))) {
            say(c, "refused", "Refused: " + action + " on " + item->getName() + ". The " + from + " state does not allow it", "State");
            return;
        }
        c.undoStack.push_back(before);
        say(c, "action", item->getName() + ": " + from + " -> " + item->getState() +
            (who ? " (" + who->getName() + ")" : ""), "Command + State");
        auto after = afterCommand.find(action);
        if (after != afterCommand.end()) after->second(c, item, outcome);
    }

    /** @brief Undoes this case's newest command, and only this case's. */
    void undo(const Params& params) {
        Case& c = find(params.at("case"));
        if (c.undoStack.empty()) {
            say(c, "refused", "Nothing to undo in this case", "Command");
            return;
        }
        engine.undoLast(c.id);
        c.active = c.undoStack.back();
        c.undoStack.pop_back();
        c.finished = false;
        engine.getInstance(c.id)->setState("RUNNING");
        say(c, "undo", "Undo restored the snapshot taken before the last command", "Command + Memento + Prototype");
    }

    /** @brief A step misses its deadline: the request runs down the approval chain to the timeout link. */
    void missDeadline(const Params& params) {
        Case& c = find(params.at("case"));
        const Process& process = processes.at(c.processId);
        WorkItem* item = engine.getInstance(c.id)->findWorkItem(process.deadlineStep);
        if (!item) throw std::runtime_error("This process has no deadline");
        if (std::find(c.active.begin(), c.active.end(), item->getId()) == c.active.end()) {
            say(c, "refused", "Refused: " + item->getName() + " has no running deadline yet", "Strategy (routing)");
            return;
        }
        const size_t commandsBefore = history.getAll().size();
        const std::vector<std::string> before = c.active;
        Request timeout("REQ-TIMEOUT", "TIMEOUT", item->getId(), c.id, item->getRequiredRoleId(), "system",
                        "Deadline exceeded", item->getState(), engine.getInstance(c.id)->getData());
        c.chain[0]->authorise(timeout);
        if (history.getAll().size() > commandsBefore) c.undoStack.push_back(before);
        say(c, "approval", "Deadline missed on " + item->getName() + ": the chain passed it to the timeout handler, now " +
            item->getState(), "Chain of Responsibility");
    }

    /** @brief Tries to change the stage as someone, through Security -> AllowedStates -> Logging. */
    void guardStage(const Params& params) {
        Case& c = find(params.at("case"));
        if (!c.guard) throw std::runtime_error("This process has no security decorators");
        Participant* who = person(param(params, "who"));
        const std::string state = params.at("state");
        try {
            c.guard->setActor(who);
            c.guard->setState(state);
            say(c, "action", who->getName() + " set the stage to " + state, "Decorator");
        } catch (const std::exception& error) {
            say(c, "refused", "Refused: " + (who ? who->getName() : std::string("Nobody")) + " setting the stage to " + state +
                ". " + error.what(), "Decorator");
        }
    }

    /** @brief Everything the page shows, as JSON. */
    std::string state(const std::string& selected) {
        std::string out = "{\"processes\":" + processesJson() + ",\"rules\":" + rulesJson() +
                          ",\"people\":" + peopleJson() + ",\"cases\":" + casesJson() + ",\"work\":" + workJson();
        for (auto& c : cases) {
            if (c->id == selected) out += ",\"selected\":" + caseJson(*c);
        }
        return out + "}";
    }

private:
    InMemoryDefinitionRepository repository;
    EventBus bus;
    HistoryManager history;
    AuditObserver audit;
    WorkflowEngine engine;
    WorkloadBasedAssignment workload;
    RoleBasedAssignment byRole;
    RuleBasedAssignment senior{"level >= 2", [](const WorkItem&, const Participant& person) { return person.getLevel() >= 2; }};

    std::vector<std::unique_ptr<Participant>> people;
    std::map<std::string, std::vector<std::string>> rolesOf;
    std::vector<Participant*> everyone;
    std::map<std::string, AssignmentStrategy*> strategies;
    std::map<std::string, Process> processes;
    std::vector<std::string> processOrder;
    std::vector<std::unique_ptr<Case>> cases;
    std::map<std::string, std::function<WorkflowAction*(WorkItem*, Participant*)>> commands;
    std::map<std::string, std::function<void(Case&, WorkItem*, const std::string&)>> afterCommand;

    static std::string param(const Params& params, const std::string& name) {
        auto found = params.find(name);
        return found == params.end() ? "" : found->second;
    }

    static void say(Case& c, const std::string& kind, const std::string& text, const std::string& pattern) {
        c.feed.push_back({kind, text, pattern});
    }

    Case& find(const std::string& id) {
        for (auto& c : cases) {
            if (c->id == id) return *c;
        }
        throw std::runtime_error("There is no case " + id);
    }

    Participant* person(const std::string& id) {
        for (Participant* candidate : everyone) {
            if (candidate->getParticipantId() == id) return candidate;
        }
        return nullptr;
    }

    void addPerson(const std::string& id, const std::string& name, const std::vector<std::string>& roles, int level, int alreadyBusy = 0) {
        people.emplace_back(new Participant(id, name, roles, level));
        for (int i = 0; i < alreadyBusy; ++i) people.back()->addOpenItem();
        rolesOf[id] = roles;
        everyone.push_back(people.back().get());
    }

    void addPeople() {
        addPerson("c1", "Sam", {"technician"}, 1, 2);
        addPerson("c2", "Alice", {"technician"}, 1);
        addPerson("c3", "Bob", {"administrator"}, 2);
        addPerson("c0", "Student", {"guest"}, 0);
        addPerson("p1", "Deon", {"proposer"}, 0);
        addPerson("p2", "Lerato", {"product_manager"}, 1);
        addPerson("p3", "Sipho", {"head_engineer"}, 2, 2);
        addPerson("p4", "Thandi", {"head_engineer"}, 2);
        addPerson("p5", "Ayesha", {"finance"}, 2);
        addPerson("p6", "Naledi", {"executive"}, 3);
        addPerson("p7", "Mandla", {"executive"}, 4);
        addPerson("p8", "Zanele", {"analyst"}, 1);
    }

    void store(WorkflowBlueprint blueprint) {
        ConcreteWorkflowBuilder builder;
        WorkflowDirector director;
        director.construct(builder, blueprint);
        repository.save(new WorkflowDefinition(builder.build()));
    }

    void add(const Process& process) {
        processes[process.id] = process;
        processOrder.push_back(process.id);
    }

    WorkflowBlueprint projectApproval(int version) {
        std::vector<Step> steps = {{"submit", "Submit Proposal", "proposer", "", ""},
                                   {"productReview", "Product Review", "product_manager", "Review Stage", ""},
                                   {"techReview", "Technical Review", "head_engineer", "Review Stage", ""}};
        std::vector<std::pair<std::string, std::string>> links = {{"submit", "productReview"}, {"productReview", "techReview"}};
        if (version == 2) {
            steps.push_back({"securityReview", "Security Review", "head_engineer", "Review Stage", ""});
            links.push_back({"techReview", "securityReview"});
            links.push_back({"securityReview", "budgetAudit"});
        } else {
            links.push_back({"techReview", "budgetAudit"});
        }
        steps.push_back({"budgetAudit", "Budget Audit", "", "Budget Stage", "budget-audit"});
        steps.push_back({"financeReview", "Budget Review", "finance", "Budget Stage", ""});
        steps.push_back({"execApproval", "Executive Approval", "executive", "Budget Stage", ""});
        steps.push_back({"kickoff", "Project Kickoff", "proposer", "", ""});
        links.push_back({"budgetAudit", "financeReview"});
        links.push_back({"execApproval", "kickoff"});
        WorkflowBlueprint blueprint = makeBlueprint("project-approval", "Project Approval", version, steps, links);
        ApprovalChainDefinition chain("finance-executive-chain");
        chain.addStep(ApprovalStepDefinition("finance", 86400, "escalate"));
        chain.addStep(ApprovalStepDefinition("executive", 259200, "escalate"));
        blueprint.approvalChains.push_back(chain);
        return blueprint;
    }

    void defineProcesses() {
        store(makeBlueprint("campus-recovery", "Campus portal recovery", 1,
            {{"diagnose", "Diagnose the outage", "technician", "", ""},
             {"repair", "Restore the portal configuration", "administrator", "", ""},
             {"verify", "Test student access", "technician", "", ""},
             {"close", "Close the incident", "administrator", "", ""}},
            {{"diagnose", "repair"}, {"repair", "verify"}}));
        repository.save(repository.load("campus-recovery", 1)->cloneWithVersion(2));   // Prototype
        store(makeBlueprint("budget-audit", "Budget Audit", 1,
            {{"compareProjects", "Compare past projects", "analyst", "", ""},
             {"summaryReport", "Write summary report", "analyst", "", ""}},
            {{"compareProjects", "summaryReport"}}));
        store(projectApproval(1));
        store(projectApproval(2));

        Process campus;
        campus.id = "campus-recovery";
        campus.name = "Campus portal recovery";
        campus.firstStep = "diagnose";
        campus.versions = {1, 2};
        campus.guarded = true;
        campus.approvalRole = "administrator";
        campus.approvalSteps = {"repair"};
        Decision verify;
        verify.options = {"passed", "failed"};
        campus.decisions["verify"] = verify;
        campus.routers["verify"] = std::make_shared<ConditionalRouting>("passed", "close", "repair");
        campus.branches = {{"verify", "close", "passed"}, {"verify", "repair", "failed"}};
        add(campus);

        Process project;
        project.id = "project-approval";
        project.name = "Project approval";
        project.firstStep = "submit";
        project.versions = {1, 2};
        project.hasBudget = true;
        project.approvalRole = "finance";
        project.approvalSteps = {"financeReview"};
        project.deadlineStep = "execApproval";
        Decision budget;
        budget.rule = [](const std::string& value) { return std::stoi(value) > EXECUTIVE_LIMIT ? "over_limit" : "within_limit"; };
        budget.skips["within_limit"] = {"execApproval"};
        project.decisions["financeReview"] = budget;
        project.routers["financeReview"] = std::make_shared<ConditionalRouting>("over_limit", "execApproval", "kickoff");
        project.branches = {{"financeReview", "execApproval", "over_limit"}, {"financeReview", "kickoff", "within_limit"}};
        add(project);

        Process audit;   // only ever runs as a sub-process, so it is not offered on the start form
        audit.id = "budget-audit";
        audit.name = "Budget Audit";
        audit.firstStep = "compareProjects";
        processes[audit.id] = audit;
    }

    std::function<RuntimeComponent*(RuntimeComponent*)> guardWrapper(Case& c) {
        return [&c](RuntimeComponent* tree) -> RuntimeComponent* {
            c.guard = new SecurityRuntimeDecorator(
                new AllowedStatesRuntimeDecorator(new LoggingRuntimeDecorator(tree), {"IDLE", "RUNNING", "COMPLETED"}),
                "administrator");
            return c.guard;
        };
    }

    /** @brief The chain comes from the definition when it has one, otherwise from the process's approver. */
    Chain approvalChain(const Process& process, Case& c) {
        WorkflowDefinition* definition = repository.load(process.id, c.version);
        WorkflowInstance* instance = engine.getInstance(c.id);
        if (!process.deadlineStep.empty()) {
            return buildApprovalChain(*definition->getApprovalChainById("finance-executive-chain"),
                                      instance->findWorkItem(process.deadlineStep), &engine);
        }
        Chain chain;
        chain.emplace_back(new ValidationApprovalHandler());
        if (!process.approvalRole.empty()) chain.emplace_back(new RoleApprovalHandler(process.approvalRole));
        for (size_t i = 0; i + 1 < chain.size(); ++i) chain[i]->setNext(chain[i + 1].get());
        return chain;
    }

    /** @brief Works out the outcome and runs any sign-off. @return false if completing must stop here. */
    bool prepareCompletion(Case& c, WorkItem* item, std::string& outcome) {
        const Process& process = processes.at(c.processId);
        auto decision = process.decisions.find(item->getId());
        if (decision != process.decisions.end() && decision->second.rule) outcome = decision->second.rule(c.budget);
        if (decision != process.decisions.end() && !decision->second.rule && outcome.empty()) {
            say(c, "refused", "Refused: choose an outcome for " + item->getName(), "Strategy (routing)");
            return false;
        }
        const std::vector<std::string>& signOff = process.approvalSteps;
        if (std::find(signOff.begin(), signOff.end(), item->getId()) == signOff.end()) return true;

        Request request("REQ-" + item->getId(), "APPROVE", item->getId(), c.id, process.approvalRole,
                        item->getAssignedParticipant() ? item->getAssignedParticipant()->getParticipantId() : "nobody",
                        item->getName(), item->getState(), Data(c.budget.empty() ? c.title : c.budget));
        bool refused = c.chain[0]->authorise(request) == HandlerResult::Rejected;
        say(c, refused ? "refused" : "approval", std::string("Approval chain ") + (refused ? "rejected " : "accepted ") +
            item->getName(), "Chain of Responsibility");
        bus.notify(WorkFlowEvent("APPROVAL_DECISION", c.id, item->getId(), refused ? "rejected" : "approved " + outcome));
        return !refused;
    }

    /** @brief Asks the step's routing strategy what comes next and moves the case there. */
    void route(Case& c, const std::string& stepId, const std::string& outcome) {
        const Process& process = processes.at(c.processId);
        const WorkflowDefinition* definition = repository.load(process.id, c.version);
        const WorkflowComponent* node = findNode(*definition, stepId);

        TreeRouting sequential;
        auto special = process.routers.find(stepId);
        RoutingStrategy* router = special != process.routers.end() ? special->second.get() : &sequential;
        std::vector<const WorkflowComponent*> next = router->next(*node, *definition, outcome);

        auto decision = process.decisions.find(stepId);
        if (decision != process.decisions.end()) {
            auto skipped = decision->second.skips.find(outcome);
            if (skipped != decision->second.skips.end()) {
                for (const std::string& id : skipped->second) cancelSkipped(c, id);
            }
        }
        c.active.erase(std::remove(c.active.begin(), c.active.end(), stepId), c.active.end());

        std::string names;
        for (const WorkflowComponent* target : next) {
            names += (names.empty() ? "" : ", ") + target->getId();
            c.taken.push_back(stepId + ">" + target->getId());
        }
        say(c, "route", router->getName() + " routing from " + stepId + (outcome.empty() ? "" : " (" + outcome + ")") + " to " +
            (names.empty() ? "the end of the process" : names), "Strategy (routing)");
        for (const WorkflowComponent* target : next) enter(c, target->getId());
        if (c.active.empty()) finish(c);
    }

    void cancelSkipped(Case& c, const std::string& id) {
        WorkItem* item = engine.getInstance(c.id)->findWorkItem(id);
        const std::vector<std::string> before = c.active;
        if (item && engine.submit(new CancelCommand(item))) {
            c.undoStack.push_back(before);
            say(c, "action", item->getName() + " is skipped on this branch, so nobody keeps holding it", "Command + State");
        }
    }

    /** @brief Makes a step active. A loop back rolls the work since then back first. */
    void enter(Case& c, const std::string& stepId) {
        WorkItem* item = engine.getInstance(c.id)->findWorkItem(stepId);
        if (!item) {
            startSubProcess(c, stepId);
            c.active.push_back(stepId);
            return;
        }
        bool looped = false;
        while ((item->getState() == "Completed" || item->getState() == "In_Progress") && !c.undoStack.empty()) {
            engine.undoLast(c.id);
            c.undoStack.pop_back();
            looped = true;
        }
        if (looped) say(c, "undo", "Loop back to " + stepId + ": undo rolled that work back, a Completed item has no way forward",
                        "Command + Memento");
        c.active.push_back(stepId);
    }

    /** @brief The sub-process runs as its own case, and the parent waits for it. */
    void startSubProcess(Case& c, const std::string& stepId) {
        engine.setAssignment(c.strategy, everyone);
        engine.advance(c.id);
        const std::string childId = c.id + "-" + stepId;
        WorkflowInstance* instance = engine.getInstance(childId);
        const Process& process = processes.at(instance->getDefinitionId());

        std::unique_ptr<Case> child(new Case());
        child->id = childId;
        child->title = process.name + " (sub-process of " + c.id + ")";
        child->processId = process.id;
        child->version = instance->getDefinitionVersion();
        child->parent = &c;
        child->callStep = stepId;
        child->strategy = c.strategy;
        child->active.push_back(process.firstStep);
        say(*child, "info", "Started by " + c.id + ", which waits here until this case finishes", "Composite");
        say(c, "route", "Waiting for sub-process " + childId + ", a workflow of its own", "Composite");
        c.undoStack.clear();   // undo stops at the boundary: the child has its own history
        cases.push_back(std::move(child));
    }

    void finish(Case& c) {
        engine.getInstance(c.id)->setState("COMPLETED");
        c.finished = true;
        bus.notify(WorkFlowEvent("INSTANCE_COMPLETED", c.id, "-", "COMPLETED"));
        say(c, "info", "Case finished", "Observer");
        if (c.parent) {
            c.undoStack.clear();
            engine.advance(c.parent->id);
            say(*c.parent, "route", "Sub-process " + c.id + " finished, so this case resumes", "Composite");
            route(*c.parent, c.callStep, "approved");
        }
    }

    void stop(Case& c) {
        engine.getInstance(c.id)->setState("CANCELLED");
        c.finished = true;
        c.active.clear();
        say(c, "info", "Step cancelled, so this case stops here. Undo brings it back", "State");
    }

    std::string processesJson() {
        std::vector<std::string> out;
        for (const std::string& id : processOrder) {
            const Process& process = processes.at(id);
            std::vector<std::string> versions;
            for (int version : process.versions) versions.push_back(std::to_string(version));
            out.push_back("{\"id\":" + quote(id) + ",\"name\":" + quote(process.name) +
                          ",\"budget\":" + (process.hasBudget ? "true" : "false") + ",\"versions\":" + quoteAll(versions) + "}");
        }
        return array(out);
    }

    std::string rulesJson() {
        std::vector<std::string> names;
        for (const auto& strategy : strategies) names.push_back(strategy.first);
        return quoteAll(names);
    }

    std::string peopleJson() {
        std::vector<std::string> out;
        for (Participant* person : everyone) {
            out.push_back("{\"id\":" + quote(person->getParticipantId()) + ",\"name\":" + quote(person->getName()) +
                          ",\"roles\":" + quoteAll(rolesOf.at(person->getParticipantId())) +
                          ",\"open\":" + std::to_string(person->getOpenItems()) + "}");
        }
        return array(out);
    }

    std::string casesJson() {
        std::vector<std::string> out;
        for (auto& c : cases) {
            ProgressReportVisitor progress;
            engine.applyVisitor(c->id, progress);
            out.push_back("{\"id\":" + quote(c->id) + ",\"title\":" + quote(c->title) +
                          ",\"state\":" + quote(engine.getInstance(c->id)->getState()) +
                          ",\"child\":" + (c->parent ? "true" : "false") +
                          ",\"percent\":" + std::to_string(progress.getPercentComplete()) + "}");
        }
        return array(out);
    }

    /** @brief The work people can pick up right now, across every case. */
    std::string workJson() {
        std::vector<std::string> out;
        for (auto& c : cases) {
            for (const std::string& id : c->active) {
                WorkItem* item = engine.getInstance(c->id)->findWorkItem(id);
                if (!item) continue;
                Participant* who = item->getAssignedParticipant();
                out.push_back("{\"case\":" + quote(c->id) + ",\"item\":" + quote(id) + ",\"name\":" + quote(item->getName()) +
                              ",\"state\":" + quote(item->getState()) + ",\"who\":" + quote(who ? who->getName() : "") +
                              ",\"role\":" + quote(item->getRequiredRoleId()) + "}");
            }
        }
        return array(out);
    }

    /** @brief Extra fields for one work item: whether routing has reached it and what its choices are. */
    std::string itemExtras(const std::string& caseId, WorkItem& item) {
        Case& c = find(caseId);
        const Process& process = processes.at(c.processId);
        bool active = std::find(c.active.begin(), c.active.end(), item.getId()) != c.active.end();
        auto decision = process.decisions.find(item.getId());
        std::vector<std::string> options;
        if (decision != process.decisions.end()) options = decision->second.options;
        return ",\"active\":" + std::string(active ? "true" : "false") + ",\"role\":" + quote(item.getRequiredRoleId()) +
               ",\"options\":" + quoteAll(options) + ",\"deadline\":" + (item.getId() == process.deadlineStep ? "true" : "false");
    }

    std::string caseJson(Case& c) {
        using namespace std::placeholders;
        TreeReader tree(c.id, std::bind(&Session::itemExtras, this, _1, _2));
        engine.applyVisitor(c.id, tree);
        ProgressReportVisitor progress;
        WorkloadVisitor people;
        engine.applyVisitor(c.id, progress);
        engine.applyVisitor(c.id, people);

        std::vector<std::string> feed, commandLog, edges;
        for (const FeedLine& line : c.feed) {
            feed.push_back("{\"kind\":" + quote(line.kind) + ",\"text\":" + quote(line.text) + ",\"pattern\":" + quote(line.pattern) + "}");
        }
        for (WorkflowAction* action : engine.getHistory(c.id)) commandLog.push_back(action->getDescription());

        const Process& process = processes.at(c.processId);
        const WorkflowDefinition* definition = repository.load(process.id, c.version);
        auto edge = [&](const std::string& from, const std::string& to, const std::string& label) {
            bool taken = std::find(c.taken.begin(), c.taken.end(), from + ">" + to) != c.taken.end();
            edges.push_back("{\"from\":" + quote(from) + ",\"to\":" + quote(to) + ",\"label\":" + quote(label) +
                            ",\"taken\":" + (taken ? "true" : "false") + "}");
        };
        for (const TransitionDefinition& link : definition->getTransitions()) edge(link.getFrom(), link.getTo(), "");
        for (const Branch& branch : process.branches) edge(branch.from, branch.to, branch.label);

        return "{\"id\":" + quote(c.id) + ",\"title\":" + quote(c.title) + ",\"budget\":" + quote(c.budget) +
               ",\"state\":" + quote(engine.getInstance(c.id)->getState()) +
               ",\"parent\":" + quote(c.parent ? c.parent->id : "") +
               ",\"guarded\":" + (c.guard ? "true" : "false") +
               ",\"canUndo\":" + (c.undoStack.empty() ? "false" : "true") +
               ",\"done\":" + std::to_string(progress.getCompletedItems()) +
               ",\"total\":" + std::to_string(progress.getTotalItems()) +
               ",\"percent\":" + std::to_string(progress.getPercentComplete()) +
               ",\"tree\":" + array(tree.getNodes()) + ",\"feed\":" + array(feed) + ",\"edges\":" + array(edges) +
               ",\"commands\":" + quoteAll(commandLog) + ",\"audit\":" + quoteAll(engine.getAuditTrail(c.id)) +
               ",\"workload\":" + quoteAll(people.getSummary()) + "}";
    }
};

std::string decode(const std::string& text) {
    std::string out;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '+') out += ' ';
        else if (text[i] == '%' && i + 2 < text.size()) {
            out += static_cast<char>(std::strtol(text.substr(i + 1, 2).c_str(), nullptr, 16));
            i += 2;
        } else out += text[i];
    }
    return out;
}

Params parseQuery(const std::string& query) {
    Params params;
    std::stringstream pairs(query);
    std::string pair;
    while (std::getline(pairs, pair, '&')) {
        size_t split = pair.find('=');
        if (split != std::string::npos) params[decode(pair.substr(0, split))] = decode(pair.substr(split + 1));
    }
    return params;
}

void reply(int client, const std::string& status, const std::string& type, const std::string& body) {
    std::string response = "HTTP/1.1 " + status + "\r\nContent-Type: " + type + "\r\nContent-Length: " +
                           std::to_string(body.size()) + "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n" + body;
    size_t sent = 0;
    while (sent < response.size()) {
        ssize_t written = send(client, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
        if (written <= 0) return;
        sent += static_cast<size_t>(written);
    }
}

/** @brief Answers one request: the page itself, or one call on the session. */
void serve(int client, std::unique_ptr<Session>& session) {
    char buffer[8192];
    ssize_t received = recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) return;
    std::istringstream request(std::string(buffer, static_cast<size_t>(received)));
    std::string method, target;
    request >> method >> target;

    size_t mark = target.find('?');
    const std::string path = target.substr(0, mark);
    const Params params = parseQuery(mark == std::string::npos ? "" : target.substr(mark + 1));

    static const std::map<std::string, std::pair<std::string, std::string>> pages = {
        {"/", {"gui/live.html", "text/html; charset=utf-8"}},
        {"/replay", {"gui/index.html", "text/html; charset=utf-8"}},
        {"/trace.js", {"gui/trace.js", "text/javascript"}},
    };
    auto page = pages.find(path);
    if (page != pages.end()) {
        std::ifstream file(page->second.first);
        std::stringstream content;
        content << file.rdbuf();
        if (file) reply(client, "200 OK", page->second.second, content.str());
        else reply(client, "500 Error", "text/plain", page->second.first + " not found: run this from the repo folder");
        return;
    }

    static const std::map<std::string, std::function<std::string(Session&, const Params&)>> routes = {
        {"/api/state", [](Session&, const Params& p) { auto found = p.find("case"); return found == p.end() ? std::string() : found->second; }},
        {"/api/start", [](Session& s, const Params& p) { return s.start(p); }},
        {"/api/act", [](Session& s, const Params& p) { s.act(p); return p.at("case"); }},
        {"/api/undo", [](Session& s, const Params& p) { s.undo(p); return p.at("case"); }},
        {"/api/deadline", [](Session& s, const Params& p) { s.missDeadline(p); return p.at("case"); }},
        {"/api/guard", [](Session& s, const Params& p) { s.guardStage(p); return p.at("case"); }},
    };

    if (path == "/api/reset") {
        session.reset(new Session());
        reply(client, "200 OK", "application/json", session->state(""));
        return;
    }
    auto route = routes.find(path);
    if (route == routes.end()) {
        reply(client, "404 Not Found", "text/plain", "Not found");
        return;
    }
    try {
        const std::string selected = route->second(*session, params);
        std::string body = session->state(selected);
        body.insert(1, "\"focus\":" + quote(selected) + ",");
        reply(client, "200 OK", "application/json", body);
    } catch (const std::exception& error) {
        reply(client, "400 Bad Request", "application/json", "{\"error\":" + quote(error.what()) + "}");
    }
}

} // namespace

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(PORT);
    if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || listen(listener, 16) != 0) {
        std::cerr << "Could not listen on port " << PORT << ". Is the console already running?\n";
        return 1;
    }

    std::unique_ptr<Session> session(new Session());
    std::cout << "TIVIDY live console: open http://localhost:" << PORT << "\nPress Ctrl+C to stop.\n" << std::flush;
    while (true) {
        int client = accept(listener, nullptr, nullptr);
        if (client < 0) continue;
        serve(client, session);
        close(client);
    }
}
