#include <iostream>
#include <string>
#include <vector>
#include "RuntimeVisitor.h"
#include "ProgressReportVisitor.h"
#include "WorkloadVisitor.h"
#include "StageInstance.h"
#include "SubProcessInstance.h"
#include "WorkItem.h"
#include "LabelledRuntimeDecorator.h"
#include "WorkflowEngine.h"

static int passed = 0, failed = 0;
static void check(const std::string& what, bool ok) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << std::endl;
    if (ok) ++passed; else ++failed;
}

// Records every call so the test can check double dispatch and traversal order.
class RecordingVisitor : public RuntimeVisitor {
public:
    std::vector<std::string> calls;
    void visitWorkItem(WorkItem& item) override { calls.push_back("item:" + item.getId()); }
    void visitStage(StageInstance& stage) override { calls.push_back("enter:" + stage.getId()); }
    void visitSubProcess(SubProcessInstance& sub) override { calls.push_back("sub:" + sub.getId()); }
    void leaveStage(StageInstance& stage) override { calls.push_back("leave:" + stage.getId()); }
};

int main() {
    // Definitions (kept separate so the runtime tree does not own them)
    StageDefinition rootDef("root", "Approval");
    StageDefinition reviewDef("review", "Review Stage");
    ActivityDefinition submitDef("submit", "Submit", "proposer");
    ActivityDefinition checkDef("check", "Check", "reviewer");
    ActivityDefinition approveDef("approve", "Approve", "manager");
    SubProcessDefinition creditDef("credit", "credit-check", 1);

    Participant ann("p1", "Ann", {"proposer"});
    Participant bob("p2", "Bob", {"reviewer"});

    // Runtime tree: Approval -> [Submit, Review Stage -> [Check, credit], Approve]
    StageInstance* root = new StageInstance("case-1", &rootDef);
    WorkItem* submit = new WorkItem("case-1", &submitDef);
    StageInstance* review = new StageInstance("case-1", &reviewDef);
    WorkItem* checkItem = new WorkItem("case-1", &checkDef);
    WorkItem* approve = new WorkItem("case-1", &approveDef);
    root->addChild(submit);
    review->addChild(checkItem);
    review->addChild(new SubProcessInstance("case-1", &creditDef));
    root->addChild(review);
    root->addChild(approve);

    submit->assignTo(&ann);   // Created -> Available -> Assigned
    submit->start();
    submit->complete();       // Completed
    checkItem->assignTo(&bob);
    checkItem->start();       // In_Progress
                              // approve stays Created and unassigned

    // --- Double dispatch and traversal order ---
    RecordingVisitor recorder;
    root->accept(recorder);
    std::vector<std::string> expected = {"enter:root", "item:submit", "enter:review", "item:check",
                                         "sub:credit", "leave:review", "item:approve", "leave:root"};
    check("each node calls its own visit method, in tree order", recorder.calls == expected);

    // --- Decorators pass the visitor through ---
    RuntimeComponent* labelled = new LabelledRuntimeDecorator(root, "Case 1"); // now owns root
    RecordingVisitor throughDecorator;
    labelled->accept(throughDecorator);
    check("a decorated tree is visited exactly like the plain tree", throughDecorator.calls == expected);

    // --- ProgressReportVisitor ---
    ProgressReportVisitor progress;
    labelled->accept(progress);
    check("progress counts every work item", progress.getTotalItems() == 3);
    check("progress counts completed items", progress.getCompletedItems() == 1);
    check("progress percentage is rounded down", progress.getPercentComplete() == 33);
    check("unfinished workflow is not complete", !progress.isComplete());
    std::map<std::string, int> states = progress.getStateCounts();
    check("state counts per state", states["Completed"] == 1 && states["In_Progress"] == 1 &&
                                    states["Created"] == 1);
    std::vector<std::string> report = progress.getReport();
    check("report has one line per node plus a summary", report.size() == 7);
    check("report indents nested stages",
          report.size() == 7 && report[3] == "    [In_Progress] Check (Bob)");
    check("report shows unassigned items", report.size() == 7 && report[5] == "  [Created] Approve (unassigned)");
    check("report ends with the summary", report.back() == "1/3 work items completed (33%)");

    checkItem->complete();
    approve->assignTo(&ann);
    approve->start();
    approve->complete();
    ProgressReportVisitor finished;
    labelled->accept(finished);
    check("finished workflow is complete", finished.isComplete() && finished.getPercentComplete() == 100);

    ProgressReportVisitor empty;
    check("no work items gives 0% and not complete", empty.getPercentComplete() == 0 && !empty.isComplete());

    // --- WorkloadVisitor ---
    WorkloadVisitor workload;
    labelled->accept(workload);
    check("participants listed in the order first seen",
          workload.getParticipants() == std::vector<std::string>({"Ann", "Bob"}));
    check("items grouped by participant",
          workload.getItemsFor("Ann") == std::vector<std::string>({"Submit", "Approve"}) &&
          workload.getItemsFor("Bob") == std::vector<std::string>({"Check"}));
    check("completed items are not open", workload.getOpenCount("Ann") == 0 && workload.getOpenCount("Bob") == 0);
    check("unknown participant has no items", workload.getItemsFor("Zed").empty() && workload.getOpenCount("Zed") == 0);

    // --- Facade ---
    WorkflowEngine engine;
    ProgressReportVisitor none;
    check("engine refuses an unknown instance", !engine.applyVisitor("missing", none) && none.getTotalItems() == 0);

    delete labelled;
    std::cout << "=== RESULT: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed == 0 ? 0 : 1;
}
