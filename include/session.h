#ifndef SESSION_H
#define SESSION_H

#include "palaver.h"

#define SESSION_MAX_PARTICIPANTS 64
#define SESSION_MAX_ROUNDS 128

typedef struct {
    double position[PALAVER_MAX_DIMS];
    double confidence; /* 0..1 */
    int rounds;
} ConsensusResult;

typedef struct {
    int id;
    double position[PALAVER_MAX_DIMS];
    int votes;
} Proposal;

typedef struct {
    Topic topic;
    Participant participants[SESSION_MAX_PARTICIPANTS];
    int num_participants;
    Proposal proposals[SESSION_MAX_PARTICIPANTS];
    int num_proposals;
    int current_round;
    double convergence_threshold;
    double step_size; /* how much participants move toward center per round */
} PalaverSession;

void session_init(PalaverSession *s, const Topic *topic, double threshold, double step_size);
int session_add_participant(PalaverSession *s, const Participant *p);
int session_add_proposal(PalaverSession *s, const double *position);
int session_vote(PalaverSession *s, int participant_idx, int proposal_idx);
ConsensusResult compute_consensus(PalaverSession *s);

#endif /* SESSION_H */
