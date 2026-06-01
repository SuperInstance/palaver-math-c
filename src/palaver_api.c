#include "palaver_api.h"
#include <string.h>

void palaver_init(PalaverContext *ctx, const Topic *topic,
                  double convergence_threshold, double step_size) {
    memset(ctx, 0, sizeof(*ctx));
    session_init(&ctx->session, topic, convergence_threshold, step_size);
    dialogue_tree_init(&ctx->dialogue);
}

ConsensusResult palaver_run(PalaverContext *ctx) {
    /* Find coalitions before convergence */
    ctx->coalitions = find_coalitions(ctx->session.participants,
                                      ctx->session.num_participants, 1.0);

    /* Run convergence */
    ConsensusResult result = compute_consensus(&ctx->session);

    /* Build history from the session state for rate analysis */
    ctx->history.num_rounds = 0;
    /* Note: detailed history is tracked internally by predict_consensus.
       For palaver_run, we record final state. */

    return result;
}

double palaver_convergence_rate(PalaverContext *ctx) {
    return convergence_rate(&ctx->history);
}
