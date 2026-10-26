/**
 * @file WorkflowDirector.h
 * @brief Defines the WorkflowDirector class.
 * 
 * Part of the Builder pattern (role: Director). It is responsible for 
 * executing the building steps in a specific sequence.
 */

#ifndef WORKFLOW_DIRECTOR_H
#define WORKFLOW_DIRECTOR_H

#include "WorkflowBuilder.h"
#include "WorkflowBlueprint.h"

/**
 * @class WorkflowDirector
 * @brief Orchestrates the construction of a workflow using a Builder.
 * 
 * The Director knows *how* to construct a workflow (the sequence of steps), 
 * but it knows nothing about the specific details of the workflow or how 
 * it is represented internally. It takes a user-provided blueprint and 
 * feeds it to the abstract builder step-by-step.
 * 
 * By accepting the abstract WorkflowBuilder interface, the Director can 
 * construct any type of workflow using any concrete builder implementation.
 */
class WorkflowDirector {
public:
    /**
     * @brief Constructs a workflow by directing a builder through the assembly steps.
     * 
     * This method iterates through the raw data contained in the blueprint 
     * (roles, activities, transitions, etc.) and calls the corresponding 
     * methods on the builder to assemble the final product.
     * 
     * @param builder The abstract Builder interface to use for construction.
     * @param blueprint The raw data blueprint provided by the user/client.
     */
    void construct(WorkflowBuilder& builder, WorkflowBlueprint& blueprint);
};

#endif // WORKFLOW_DIRECTOR_H