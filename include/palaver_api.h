#ifndef PALAVER_API_H
#define PALAVER_API_H

#include "palaver.h"
#include "session.h"
#include "coalition.h"
#include "convergence.h"
#include "dialogue.h"

/* Unified API for Palaver consensus computation.
   All functionality is accessible through the individual headers above.
   This header provides convenience wrappers. */

typedef struct {
    PalaverSession session;
    CoalitionResult coalitions;
    DialogueTree dialogue;
    SessionHistory history;
} PalaverContext;

/* Initialize a full palaver context. */
void palaver_init(PalaverContext *ctx, const Topic *topic,
                  double convergence_threshold, double step_size);

/* Run full consensus: iterate convergence, track coalitions, return result. */
ConsensusResult palaver_run(PalaverContext *ctx);

/* Analyze convergence rate from current context. */
double palaver_convergence_rate(PalaverContext *ctx);

#endif /* PALAVER_API_H */
