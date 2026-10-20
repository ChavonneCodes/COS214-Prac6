#include <iostream>
#include "SequentialRouting.h"
#include "ConditionalRouting.h"

static int passed = 0, failed = 0;
static void check(const std::string& what, bool ok) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << std::endl;
    if (ok) ++passed; else ++failed;
}

int main() {
    WorkflowDefinition def("wf1", "Leave request", 1);
    ActivityDefinition a("A", "Submit", "r1");
    ActivityDefinition b("B", "Review", "r2");
    ActivityDefinition c("C", "Approve", "r3");
    ActivityDefinition d("D", "Reject", "r3");
    def.addActivity(a); def.addActivity(b); def.addActivity(c); def.addActivity(d);
    def.addTransition(TransitionDefinition("A", "B", "always"));
    def.addTransition(TransitionDefinition("B", "C", "passed"));

    SequentialRouting seq;
    ConditionalRouting cond("passed", "C", "D");
    RoutingStrategy* s = &seq;

    check("Sequential name", s->getName() == "Sequential");
    std::vector<const ActivityDefinition*> r = s->next(a, def, "");
    check("Sequential: A -> one activity", r.size() == 1);
    check("Sequential: A -> B", !r.empty() && r[0]->getId() == "B");
    check("Sequential ignores outcome", s->next(a, def, "failed").size() == 1);
    check("Sequential: B has no 'always' transition", s->next(b, def, "").empty());
    check("Sequential: D is a dead end", s->next(d, def, "").empty());

    s = &cond;
    check("Conditional name", s->getName() == "Conditional(passed)");
    r = s->next(b, def, "passed");
    check("Conditional: passed -> C", r.size() == 1 && r[0]->getId() == "C");
    r = s->next(b, def, "failed");
    check("Conditional: failed -> D", r.size() == 1 && r[0]->getId() == "D");
    ConditionalRouting bad("passed", "X", "Y");
    check("Conditional: unknown target -> empty", bad.next(b, def, "passed").empty());

    std::cout << "=== RESULT: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed == 0 ? 1 - 1 : 1;
}
