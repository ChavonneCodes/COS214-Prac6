/**
 * @file Participant.cpp
 * @brief Implementation of Participant (used by the Strategy pattern).
 */
#include "Participant.h"
#include <algorithm>

Participant::Participant(const std::string& participantId, const std::string& name,
                         const std::vector<std::string>& roleIds, int level)
    : participantId(participantId), name(name), roleIds(roleIds),
      level(level), openItems(0) {}

std::string Participant::getParticipantId() const { return participantId; }
std::string Participant::getName() const { return name; }
int Participant::getLevel() const { return level; }

bool Participant::hasRole(const std::string& roleId) const {
    return std::find(roleIds.begin(), roleIds.end(), roleId) != roleIds.end();
}

int Participant::getOpenItems() const { return openItems; }
void Participant::addOpenItem() { ++openItems; }
void Participant::closeOpenItem() { if (openItems > 0) --openItems; }
