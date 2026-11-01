/**
 * @file DemoTrace.h
 * @brief Records a demo run so the GUI (gui/index.html) can replay what the engine really did.
 *
 * DemoTrace is one more **Observer** on the EventBus. The engine does not know
 * it exists. Each beat stores the narration, the events seen since the last
 * beat, and a snapshot of the live tree taken by a ProgressReportVisitor.
 */
#ifndef DEMO_TRACE_H
#define DEMO_TRACE_H

#include <fstream>
#include <string>
#include <vector>
#include "ProgressReportVisitor.h"
#include "WorkFlowObserver.h"
#include "WorkflowDefinition.h"
#include "WorkflowEngine.h"

/** @brief One line of the same-or-different table. */
struct Comparison {
    std::string topic;     ///< What is being compared.
    std::string scenario1; ///< What Scenario 1 does.
    std::string scenario2; ///< What Scenario 2 does.
    bool same;             ///< true if both scenarios use the same code here.
};

/** @brief An extra arrow for the GUI: a routing branch that is not a fixed transition. */
struct Branch {
    std::string from;  ///< Activity the branch leaves.
    std::string to;    ///< Activity the branch enters.
    std::string label; ///< Outcome that takes this branch.
};

/**
 * @class DemoTrace
 * @brief Observer that collects the run as JSON and writes it to a script the GUI loads.
 */
class DemoTrace : public WorkFlowObserver {
private:
    std::string cases;            ///< Finished cases as JSON objects.
    std::string caseHeader;       ///< Opening JSON of the current case.
    std::string beats;            ///< Beats of the current case as JSON objects.
    std::vector<std::string> events; ///< Events since the last beat.

    /** @brief Quotes a string as a JSON value. */
    static std::string quote(const std::string& text) {
        std::string out = "\"";
        for (char c : text) {
            if (c == '"' || c == '\\') out += std::string("\\") + c;
            else if (c == '\n') out += "\\n";
            else out += c;
        }
        return out + "\"";
    }

    /** @brief Writes a list of strings as a JSON array. */
    static std::string quoteAll(const std::vector<std::string>& lines) {
        std::string out = "[";
        for (size_t i = 0; i < lines.size(); ++i) out += (i ? "," : "") + quote(lines[i]);
        return out + "]";
    }

public:
    /** @brief Keeps each event until the next beat. */
    void update(const WorkFlowEvent& event) override {
        events.push_back("#" + std::to_string(event.getSequence()) + " " + event.getEventName() + " " +
                         event.getWorkItemId() + " " + event.getPayload());
    }

    /**
     * @brief Starts recording a new case.
     * @param scenario   1 or 2.
     * @param title      Name of the case.
     * @param definition Definition the case uses (for the activities and arrows).
     * @param branches   Routing branches to draw as well as the fixed transitions.
     * @param called     Definition run as a sub-process, so its steps can be named (may be null).
     */
    void beginCase(int scenario, const std::string& title, const WorkflowDefinition& definition,
                   const std::vector<Branch>& branches, const WorkflowDefinition* called = nullptr) {
        std::string nodes = "[", edges = "[";
        std::vector<ActivityDefinition> activities = definition.getActivities();
        if (called) activities.insert(activities.end(), called->getActivities().begin(), called->getActivities().end());
        for (const ActivityDefinition& activity : activities) {
            nodes += (nodes.size() > 1 ? "," : "");
            nodes += "{\"id\":" + quote(activity.getId()) + ",\"name\":" + quote(activity.getName()) + "}";
        }
        for (const TransitionDefinition& transition : definition.getTransitions()) {
            edges += (edges.size() > 1 ? "," : "");
            edges += "{\"from\":" + quote(transition.getFrom()) + ",\"to\":" + quote(transition.getTo()) + ",\"label\":\"\"}";
        }
        for (const Branch& branch : branches) {
            edges += (edges.size() > 1 ? "," : "");
            edges += "{\"from\":" + quote(branch.from) + ",\"to\":" + quote(branch.to) +
                     ",\"label\":" + quote(branch.label) + "}";
        }
        caseHeader = "{\"scenario\":" + std::to_string(scenario) + ",\"title\":" + quote(title) +
                     ",\"version\":" + std::to_string(definition.getVersion()) +
                     ",\"nodes\":" + nodes + "],\"edges\":" + edges + "],\"beats\":[";
    }

    /**
     * @brief Records one moment of the story.
     * @param engine     Engine to read the live tree from.
     * @param instanceId Case to snapshot (empty before the case has started).
     * @param phase      define, start, run or report.
     * @param differs    true if only this scenario does this.
     * @param current    Activity id being worked on (may be empty).
     * @param title      Short heading.
     * @param detail     What happened.
     * @param pattern    Design pattern doing the work (may be empty).
     * @param call       The engine call behind this moment (may be empty).
     */
    void beat(WorkflowEngine& engine, const std::string& instanceId, const std::string& phase, bool differs,
              const std::string& current, const std::string& title, const std::string& detail,
              const std::string& pattern, const std::string& call) {
        ProgressReportVisitor progress;
        engine.applyVisitor(instanceId, progress);
        beats += (beats.empty() ? "" : ",\n");
        beats += "{\"phase\":" + quote(phase) + ",\"differs\":" + (differs ? "true" : "false") +
                 ",\"current\":" + quote(current) + ",\"title\":" + quote(title) + ",\"detail\":" + quote(detail) +
                 ",\"pattern\":" + quote(pattern) + ",\"call\":" + quote(call) +
                 ",\"done\":" + std::to_string(progress.getCompletedItems()) +
                 ",\"total\":" + std::to_string(progress.getTotalItems()) +
                 ",\"tree\":" + quoteAll(progress.getReport()) + ",\"events\":" + quoteAll(events) + "}";
        events.clear();
    }

    /** @brief Finishes the current case. */
    void endCase() {
        cases += (cases.empty() ? "" : ",\n") + caseHeader + beats + "]}";
        beats.clear();
    }

    /**
     * @brief Writes everything to a JavaScript file that the GUI loads.
     * @param path       File to write.
     * @param comparison The same-or-different table.
     * @return true if the file was written.
     */
    bool write(const std::string& path, const std::vector<Comparison>& comparison) const {
        std::ofstream file(path);
        if (!file) return false;
        file << "// Written by ./main. Do not edit: run the demo again to refresh it.\n"
             << "window.TIVIDY_TRACE = {\"cases\":[" << cases << "],\n\"comparison\":[";
        for (size_t i = 0; i < comparison.size(); ++i) {
            file << (i ? "," : "") << "{\"topic\":" << quote(comparison[i].topic)
                 << ",\"s1\":" << quote(comparison[i].scenario1) << ",\"s2\":" << quote(comparison[i].scenario2)
                 << ",\"same\":" << (comparison[i].same ? "true" : "false") << "}\n";
        }
        file << "]};\n";
        return true;
    }
};

#endif // DEMO_TRACE_H
