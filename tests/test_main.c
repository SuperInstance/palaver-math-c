#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "palaver.h"
#include "session.h"
#include "coalition.h"
#include "convergence.h"
#include "dialogue.h"
#include "palaver_api.h"
#include "arena.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_EQ_DBL(a, b, tol) do { \
    double _a = (a), _b = (b), _tol = (tol); \
    if (fabs(_a - _b) > _tol) { \
        printf("  FAIL line %d: %.6f != %.6f (tol %.6f)\n", __LINE__, _a, _b, _tol); \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

#define ASSERT_EQ_INT(a, b) do { \
    int _a = (a), _b = (b); \
    if (_a != _b) { \
        printf("  FAIL line %d: %d != %d\n", __LINE__, _a, _b); \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("  FAIL line %d: %s\n", __LINE__, #cond); \
        tests_failed++; \
    } else { tests_passed++; } \
} while(0)

#define TEST(name) printf("  %s\n", name)

/* ===================== ARENA TESTS ===================== */
static void test_arena_basic(void) {
    TEST("arena basic alloc");
    Arena a;
    arena_init(&a);
    int *p = (int *)arena_alloc(&a, sizeof(int) * 10);
    ASSERT_TRUE(p != NULL);
    for (int i = 0; i < 10; i++) p[i] = i;
    ASSERT_EQ_INT(p[5], 5);
    arena_free(&a);
}

static void test_arena_multiple(void) {
    TEST("arena multiple allocs");
    Arena a;
    arena_init(&a);
    void *p1 = arena_alloc(&a, 100);
    void *p2 = arena_alloc(&a, 200);
    ASSERT_TRUE(p1 != NULL);
    ASSERT_TRUE(p2 != NULL);
    ASSERT_TRUE(p1 != p2);
    arena_free(&a);
}

static void test_arena_large(void) {
    TEST("arena large alloc");
    Arena a;
    arena_init(&a);
    void *p = arena_alloc(&a, 100000);
    ASSERT_TRUE(p != NULL);
    arena_free(&a);
}

/* ===================== PALAVER TESTS ===================== */
static Participant make_participant(int id, int dims, double pos[], double inf) {
    Participant p;
    p.id = id;
    p.dims = dims;
    p.influence = inf;
    for (int i = 0; i < dims; i++) p.position[i] = pos[i];
    return p;
}

static void test_compute_center_single(void) {
    TEST("compute_center single participant");
    double pos[] = {1.0, 2.0};
    Participant p = make_participant(1, 2, pos, 1.0);
    double center[PALAVER_MAX_DIMS];
    compute_center(&p, 1, center);
    ASSERT_EQ_DBL(center[0], 1.0, 1e-9);
    ASSERT_EQ_DBL(center[1], 2.0, 1e-9);
}

static void test_compute_center_two_equal(void) {
    TEST("compute_center two equal influence");
    double pos1[] = {0.0, 0.0};
    double pos2[] = {4.0, 6.0};
    Participant ps[2];
    ps[0] = make_participant(1, 2, pos1, 1.0);
    ps[1] = make_participant(2, 2, pos2, 1.0);
    double center[PALAVER_MAX_DIMS];
    compute_center(ps, 2, center);
    ASSERT_EQ_DBL(center[0], 2.0, 1e-9);
    ASSERT_EQ_DBL(center[1], 3.0, 1e-9);
}

static void test_compute_center_weighted(void) {
    TEST("compute_center weighted by influence");
    double pos1[] = {0.0};
    double pos2[] = {10.0};
    Participant ps[2];
    ps[0] = make_participant(1, 1, pos1, 3.0);
    ps[1] = make_participant(2, 1, pos2, 1.0);
    double center[PALAVER_MAX_DIMS];
    compute_center(ps, 2, center);
    ASSERT_EQ_DBL(center[0], 2.5, 1e-9);
}

static void test_consensus_distance_zero(void) {
    TEST("consensus_distance single participant = 0");
    double pos[] = {5.0};
    Participant p = make_participant(1, 1, pos, 1.0);
    ASSERT_EQ_DBL(consensus_distance(&p, 1), 0.0, 1e-9);
}

static void test_consensus_distance_two(void) {
    TEST("consensus_distance two participants");
    double p1[] = {0.0}, p2[] = {4.0};
    Participant ps[2];
    ps[0] = make_participant(1, 1, p1, 1.0);
    ps[1] = make_participant(2, 1, p2, 1.0);
    ASSERT_EQ_DBL(consensus_distance(ps, 2), 4.0, 1e-9);
}

static void test_euclidean_basic(void) {
    TEST("euclidean_distance basic");
    double a[] = {0.0, 0.0};
    double b[] = {3.0, 4.0};
    ASSERT_EQ_DBL(euclidean_distance(a, b, 2), 5.0, 1e-9);
}

static void test_euclidean_zero(void) {
    TEST("euclidean_distance same point");
    double a[] = {1.0, 2.0, 3.0};
    ASSERT_EQ_DBL(euclidean_distance(a, a, 3), 0.0, 1e-9);
}

static void test_consensus_distance_three(void) {
    TEST("consensus_distance three participants");
    double p1[] = {0.0}, p2[] = {2.0}, p3[] = {4.0};
    Participant ps[3];
    ps[0] = make_participant(1, 1, p1, 1.0);
    ps[1] = make_participant(2, 1, p2, 1.0);
    ps[2] = make_participant(3, 1, p3, 1.0);
    /* avg of (0,2)=2, (0,4)=4, (2,4)=2 => avg = 8/3 */
    ASSERT_EQ_DBL(consensus_distance(ps, 3), 8.0 / 3.0, 1e-9);
}

/* ===================== SESSION TESTS ===================== */
static void test_session_init(void) {
    TEST("session init");
    Topic t = {1, 2, {0.0, 0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.1, 0.5);
    ASSERT_EQ_INT(s.num_participants, 0);
    ASSERT_EQ_INT(s.topic.id, 1);
    ASSERT_EQ_DBL(s.convergence_threshold, 0.1, 1e-9);
}

static void test_session_add_participant(void) {
    TEST("session add participant");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    double pos[] = {1.0};
    Participant p = make_participant(1, 1, pos, 1.0);
    int idx = session_add_participant(&s, &p);
    ASSERT_EQ_INT(idx, 0);
    ASSERT_EQ_INT(s.num_participants, 1);
}

static void test_session_add_proposal(void) {
    TEST("session add proposal");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    double pos[] = {5.0};
    int idx = session_add_proposal(&s, pos);
    ASSERT_EQ_INT(idx, 0);
    ASSERT_EQ_INT(s.num_proposals, 1);
    ASSERT_EQ_DBL(s.proposals[0].position[0], 5.0, 1e-9);
}

static void test_session_vote(void) {
    TEST("session vote");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    double pos[] = {1.0};
    Participant p = make_participant(1, 1, pos, 1.0);
    session_add_participant(&s, &p);
    double prop[] = {0.5};
    session_add_proposal(&s, prop);
    int rc = session_vote(&s, 0, 0);
    ASSERT_EQ_INT(rc, 0);
    ASSERT_EQ_INT(s.proposals[0].votes, 1);
}

static void test_session_convergence_two(void) {
    TEST("session convergence two participants");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    double p1[] = {0.0}, p2[] = {2.0};
    Participant pa = make_participant(1, 1, p1, 1.0);
    Participant pb = make_participant(2, 1, p2, 1.0);
    session_add_participant(&s, &pa);
    session_add_participant(&s, &pb);
    ConsensusResult r = compute_consensus(&s);
    ASSERT_TRUE(r.rounds > 0);
    ASSERT_EQ_DBL(r.position[0], 1.0, 0.1); /* should converge near midpoint */
    ASSERT_TRUE(r.confidence > 0.5);
}

static void test_session_convergence_already_consensus(void) {
    TEST("session already at consensus");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    double p1[] = {1.0}, p2[] = {1.0};
    Participant pa = make_participant(1, 1, p1, 1.0);
    Participant pb = make_participant(2, 1, p2, 1.0);
    session_add_participant(&s, &pa);
    session_add_participant(&s, &pb);
    ConsensusResult r = compute_consensus(&s);
    ASSERT_EQ_INT(r.rounds, 1);
    ASSERT_EQ_DBL(r.position[0], 1.0, 1e-9);
}

static void test_session_empty(void) {
    TEST("session empty returns zero");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    ConsensusResult r = compute_consensus(&s);
    ASSERT_EQ_INT(r.rounds, 0);
    ASSERT_EQ_DBL(r.confidence, 0.0, 1e-9);
}

static void test_session_multidim(void) {
    TEST("session 3D convergence");
    Topic t = {1, 3, {0.0, 0.0, 0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.1, 0.3);
    double p1[] = {0.0, 0.0, 0.0}, p2[] = {6.0, 6.0, 6.0};
    Participant pa = make_participant(1, 3, p1, 1.0);
    Participant pb = make_participant(2, 3, p2, 1.0);
    session_add_participant(&s, &pa);
    session_add_participant(&s, &pb);
    ConsensusResult r = compute_consensus(&s);
    ASSERT_TRUE(r.rounds > 0);
    ASSERT_EQ_DBL(r.position[0], 3.0, 0.5);
}

/* ===================== COALITION TESTS ===================== */
static void test_coalition_one_group(void) {
    TEST("coalition all close -> one group");
    double p1[] = {0.0}, p2[] = {0.5}, p3[] = {0.3};
    Participant ps[3];
    ps[0] = make_participant(1, 1, p1, 1.0);
    ps[1] = make_participant(2, 1, p2, 1.0);
    ps[2] = make_participant(3, 1, p3, 1.0);
    CoalitionResult cr = find_coalitions(ps, 3, 2.0);
    ASSERT_EQ_INT(cr.num_groups, 1);
    ASSERT_EQ_INT(cr.groups[0].num_members, 3);
}

static void test_coalition_two_groups(void) {
    TEST("coalition two separate groups");
    double p1[] = {0.0}, p2[] = {0.5}, p3[] = {10.0}, p4[] = {10.5};
    Participant ps[4];
    ps[0] = make_participant(1, 1, p1, 1.0);
    ps[1] = make_participant(2, 1, p2, 1.0);
    ps[2] = make_participant(3, 1, p3, 1.0);
    ps[3] = make_participant(4, 1, p4, 1.0);
    CoalitionResult cr = find_coalitions(ps, 4, 2.0);
    ASSERT_EQ_INT(cr.num_groups, 2);
}

static void test_coalition_empty(void) {
    TEST("coalition empty participants");
    CoalitionResult cr = find_coalitions(NULL, 0, 1.0);
    ASSERT_EQ_INT(cr.num_groups, 0);
}

static void test_coalition_strength(void) {
    TEST("coalition strength computed");
    double p1[] = {0.0}, p2[] = {0.1};
    Participant ps[2];
    ps[0] = make_participant(1, 1, p1, 1.0);
    ps[1] = make_participant(2, 1, p2, 1.0);
    CoalitionResult cr = find_coalitions(ps, 2, 2.0);
    ASSERT_TRUE(cr.groups[0].strength > 0.0);
}

static void test_coalition_merge(void) {
    TEST("coalition merge adjacent");
    double p1[] = {0.0}, p2[] = {5.0}, p3[] = {10.0};
    Participant ps[3];
    ps[0] = make_participant(1, 1, p1, 1.0);
    ps[1] = make_participant(2, 1, p2, 1.0);
    ps[2] = make_participant(3, 1, p3, 1.0);
    CoalitionResult cr = find_coalitions(ps, 3, 2.0);
    /* 3 separate groups */
    ASSERT_EQ_INT(cr.num_groups, 3);
    /* Merge with threshold 6.0 — should merge all */
    CoalitionResult merged = merge_coalitions(ps, 3, &cr, 6.0);
    ASSERT_TRUE(merged.num_groups < 3);
}

static void test_coalition_center(void) {
    TEST("coalition center computed");
    double p1[] = {0.0}, p2[] = {2.0};
    Participant ps[2];
    ps[0] = make_participant(1, 1, p1, 1.0);
    ps[1] = make_participant(2, 1, p2, 1.0);
    CoalitionResult cr = find_coalitions(ps, 2, 5.0);
    ASSERT_EQ_DBL(cr.groups[0].center[0], 1.0, 1e-9);
}

/* ===================== CONVERGENCE TESTS ===================== */
static void test_convergence_rate_positive(void) {
    TEST("convergence rate positive for converging session");
    SessionHistory h;
    h.num_rounds = 5;
    for (int i = 0; i < 5; i++) {
        h.rounds[i].spread = 10.0 - 2.0 * i; /* 10, 8, 6, 4, 2 */
    }
    double rate = convergence_rate(&h);
    ASSERT_TRUE(rate > 0.0); /* positive convergence */
}

static void test_convergence_rate_zero_rounds(void) {
    TEST("convergence rate zero rounds = 0");
    SessionHistory h;
    h.num_rounds = 0;
    ASSERT_EQ_DBL(convergence_rate(&h), 0.0, 1e-9);
}

static void test_convergence_rate_one_round(void) {
    TEST("convergence rate one round = 0");
    SessionHistory h;
    h.num_rounds = 1;
    h.rounds[0].spread = 5.0;
    ASSERT_EQ_DBL(convergence_rate(&h), 0.0, 1e-9);
}

static void test_predict_consensus(void) {
    TEST("predict consensus");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.3);
    double p1[] = {0.0}, p2[] = {4.0};
    Participant pa = make_participant(1, 1, p1, 1.0);
    Participant pb = make_participant(2, 1, p2, 1.0);
    session_add_participant(&s, &pa);
    session_add_participant(&s, &pb);
    ConsensusResult r = predict_consensus(&s, 200);
    ASSERT_TRUE(r.rounds > 0);
    ASSERT_EQ_DBL(r.position[0], 2.0, 0.5);
}

static void test_predict_consensus_fast(void) {
    TEST("predict consensus converges quickly when close");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    double p1[] = {1.0}, p2[] = {1.1};
    Participant pa = make_participant(1, 1, p1, 1.0);
    Participant pb = make_participant(2, 1, p2, 1.0);
    session_add_participant(&s, &pa);
    session_add_participant(&s, &pb);
    ConsensusResult r = predict_consensus(&s, 200);
    ASSERT_TRUE(r.rounds <= 5);
}

/* ===================== DIALOGUE TESTS ===================== */
static void test_dialogue_init(void) {
    TEST("dialogue tree init");
    DialogueTree tree;
    dialogue_tree_init(&tree);
    ASSERT_EQ_INT(tree.num_nodes, 0);
}

static void test_dialogue_add_statement(void) {
    TEST("dialogue add statement");
    DialogueTree tree;
    dialogue_tree_init(&tree);
    int id = dialogue_add_statement(&tree, "Hello", 1, 0);
    ASSERT_EQ_INT(id, 0);
    ASSERT_EQ_INT(tree.num_nodes, 1);
    ASSERT_EQ_INT(strcmp(tree.nodes[0].statement, "Hello"), 0);
}

static void test_dialogue_add_response(void) {
    TEST("dialogue add response");
    DialogueTree tree;
    dialogue_tree_init(&tree);
    int root = dialogue_add_statement(&tree, "Topic", 1, 0);
    int child = dialogue_add_statement(&tree, "Reply", 2, 0);
    int rc = dialogue_add_response(&tree, root, child);
    ASSERT_EQ_INT(rc, 0);
    ASSERT_EQ_INT(tree.nodes[root].num_responses, 1);
    ASSERT_EQ_INT(tree.nodes[root].response_ids[0], child);
}

static void test_dialogue_build_tree(void) {
    TEST("dialogue build tree from flat");
    DialogueTree tree;
    const char *stmts[] = {"Root", "Reply A", "Reply B", "Consensus"};
    int speakers[] = {1, 2, 3, 4};
    int parents[] = {-1, 0, 0, 1};
    int cons[] = {0, 0, 0, 1};
    int n = dialogue_build_tree(&tree, stmts, speakers, parents, cons, 4);
    ASSERT_EQ_INT(n, 4);
    ASSERT_EQ_INT(tree.nodes[0].num_responses, 2);
}

static void test_dialogue_find_consensus(void) {
    TEST("dialogue find consensus path (BFS)");
    DialogueTree tree;
    const char *stmts[] = {"Root", "A", "B", "C", "Agreement!"};
    int speakers[] = {1, 2, 3, 4, 5};
    int parents[] = {-1, 0, 0, 1, 2};
    int cons[] = {0, 0, 0, 0, 1};
    dialogue_build_tree(&tree, stmts, speakers, parents, cons, 5);
    int path[DIALOGUE_MAX_NODES];
    int path_len = 0;
    int result = find_consensus_path(&tree, path, &path_len);
    ASSERT_TRUE(result >= 0);
    ASSERT_EQ_INT(path_len, 3); /* 0 -> 2 -> 4 */
    ASSERT_EQ_INT(path[0], 0);
    ASSERT_EQ_INT(path[2], 4);
}

static void test_dialogue_no_consensus(void) {
    TEST("dialogue no consensus returns -1");
    DialogueTree tree;
    const char *stmts[] = {"Root", "A", "B"};
    int speakers[] = {1, 2, 3};
    int parents[] = {-1, 0, 0};
    int cons[] = {0, 0, 0};
    dialogue_build_tree(&tree, stmts, speakers, parents, cons, 3);
    int path[DIALOGUE_MAX_NODES];
    int path_len = 0;
    int result = find_consensus_path(&tree, path, &path_len);
    ASSERT_EQ_INT(result, -1);
}

static void test_dialogue_root_is_consensus(void) {
    TEST("dialogue root is consensus");
    DialogueTree tree;
    const char *stmts[] = {"We agree!"};
    int speakers[] = {1};
    int parents[] = {-1};
    int cons[] = {1};
    dialogue_build_tree(&tree, stmts, speakers, parents, cons, 1);
    int path[DIALOGUE_MAX_NODES];
    int path_len = 0;
    int result = find_consensus_path(&tree, path, &path_len);
    ASSERT_EQ_INT(result, 0);
    ASSERT_EQ_INT(path_len, 1);
}

/* ===================== API TESTS ===================== */
static void test_palaver_api_init(void) {
    TEST("palaver API init");
    PalaverContext ctx;
    Topic t = {1, 2, {0.0, 0.0}};
    palaver_init(&ctx, &t, 0.1, 0.5);
    ASSERT_EQ_INT(ctx.session.num_participants, 0);
}

static void test_palaver_api_run(void) {
    TEST("palaver API run full consensus");
    PalaverContext ctx;
    Topic t = {1, 1, {0.0}};
    palaver_init(&ctx, &t, 0.01, 0.3);
    double p1[] = {0.0}, p2[] = {2.0};
    Participant pa = make_participant(1, 1, p1, 1.0);
    Participant pb = make_participant(2, 1, p2, 1.0);
    session_add_participant(&ctx.session, &pa);
    session_add_participant(&ctx.session, &pb);
    ConsensusResult r = palaver_run(&ctx);
    ASSERT_TRUE(r.rounds > 0);
    ASSERT_EQ_DBL(r.position[0], 1.0, 0.5);
}

static void test_palaver_api_coalitions(void) {
    TEST("palaver API finds coalitions");
    PalaverContext ctx;
    Topic t = {1, 1, {0.0}};
    palaver_init(&ctx, &t, 0.01, 0.3);
    double p1[] = {0.0}, p2[] = {0.1}, p3[] = {10.0};
    Participant pa = make_participant(1, 1, p1, 1.0);
    Participant pb = make_participant(2, 1, p2, 1.0);
    Participant pc = make_participant(3, 1, p3, 1.0);
    session_add_participant(&ctx.session, &pa);
    session_add_participant(&ctx.session, &pb);
    session_add_participant(&ctx.session, &pc);
    palaver_run(&ctx);
    ASSERT_TRUE(ctx.coalitions.num_groups >= 1);
}

static void test_session_vote_invalid(void) {
    TEST("session vote invalid indices");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    ASSERT_EQ_INT(session_vote(&s, -1, 0), -1);
    ASSERT_EQ_INT(session_vote(&s, 0, -1), -1);
}

static void test_session_max_participants(void) {
    TEST("session max participants overflow");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.5);
    double pos[] = {0.0};
    int added = 0;
    for (int i = 0; i < SESSION_MAX_PARTICIPANTS + 5; i++) {
        Participant p = make_participant(i, 1, pos, 1.0);
        if (session_add_participant(&s, &p) >= 0) added++;
    }
    ASSERT_EQ_INT(added, SESSION_MAX_PARTICIPANTS);
}

static void test_euclidean_1d(void) {
    TEST("euclidean 1D");
    double a[] = {3.0};
    double b[] = {7.0};
    ASSERT_EQ_DBL(euclidean_distance(a, b, 1), 4.0, 1e-9);
}

static void test_compute_center_empty(void) {
    TEST("compute_center empty doesn't crash");
    double center[PALAVER_MAX_DIMS] = {0};
    compute_center(NULL, 0, center);
    /* Just shouldn't crash */
    ASSERT_TRUE(1);
}

static void test_dialogue_max_nodes(void) {
    TEST("dialogue respects max nodes");
    DialogueTree tree;
    dialogue_tree_init(&tree);
    int added = 0;
    for (int i = 0; i < DIALOGUE_MAX_NODES + 10; i++) {
        if (dialogue_add_statement(&tree, "x", 1, 0) >= 0) added++;
    }
    ASSERT_EQ_INT(added, DIALOGUE_MAX_NODES);
}

static void test_convergence_rate_flat(void) {
    TEST("convergence rate flat (no change)");
    SessionHistory h;
    h.num_rounds = 4;
    for (int i = 0; i < 4; i++) h.rounds[i].spread = 5.0;
    double rate = convergence_rate(&h);
    ASSERT_EQ_DBL(rate, 0.0, 1e-9);
}

static void test_coalition_single_participant(void) {
    TEST("coalition single participant");
    double p1[] = {5.0};
    Participant ps[1];
    ps[0] = make_participant(1, 1, p1, 1.0);
    CoalitionResult cr = find_coalitions(ps, 1, 1.0);
    ASSERT_EQ_INT(cr.num_groups, 1);
    ASSERT_EQ_INT(cr.groups[0].num_members, 1);
}

static void test_session_convergence_weighted(void) {
    TEST("session convergence with unequal influence");
    Topic t = {1, 1, {0.0}};
    PalaverSession s;
    session_init(&s, &t, 0.01, 0.3);
    double p1[] = {0.0}, p2[] = {10.0};
    Participant pa = make_participant(1, 1, p1, 3.0); /* heavy */
    Participant pb = make_participant(2, 1, p2, 1.0); /* light */
    session_add_participant(&s, &pa);
    session_add_participant(&s, &pb);
    ConsensusResult r = compute_consensus(&s);
    /* Should converge closer to the heavy participant (near 2.5) */
    ASSERT_TRUE(r.position[0] < 5.0);
}

static void test_dialogue_deep_chain(void) {
    TEST("dialogue deep chain finds consensus");
    DialogueTree tree;
    dialogue_tree_init(&tree);
    char buf[16];
    /* Build chain: 0 -> 1 -> 2 -> 3 -> 4 (consensus) */
    for (int i = 0; i < 5; i++) {
        sprintf(buf, "Node %d", i);
        dialogue_add_statement(&tree, buf, i + 1, i == 4);
        if (i > 0) dialogue_add_response(&tree, i - 1, i);
    }
    int path[DIALOGUE_MAX_NODES];
    int path_len = 0;
    int result = find_consensus_path(&tree, path, &path_len);
    ASSERT_EQ_INT(result, 4);
    ASSERT_EQ_INT(path_len, 5);
    ASSERT_EQ_INT(path[0], 0);
    ASSERT_EQ_INT(path[4], 4);
}

static void test_coalition_merge_noop(void) {
    TEST("coalition merge with tiny threshold = no merge");
    double p1[] = {0.0}, p2[] = {100.0};
    Participant ps[2];
    ps[0] = make_participant(1, 1, p1, 1.0);
    ps[1] = make_participant(2, 1, p2, 1.0);
    CoalitionResult cr = find_coalitions(ps, 2, 1.0);
    ASSERT_EQ_INT(cr.num_groups, 2);
    CoalitionResult merged = merge_coalitions(ps, 2, &cr, 0.01);
    ASSERT_EQ_INT(merged.num_groups, 2);
}

int main(void) {
    printf("=== Palaver Math C Test Suite ===\n\n");

    printf("[Arena]\n");
    test_arena_basic();
    test_arena_multiple();
    test_arena_large();

    printf("\n[Palaver Core]\n");
    test_compute_center_single();
    test_compute_center_two_equal();
    test_compute_center_weighted();
    test_compute_center_empty();
    test_consensus_distance_zero();
    test_consensus_distance_two();
    test_consensus_distance_three();
    test_euclidean_basic();
    test_euclidean_zero();
    test_euclidean_1d();

    printf("\n[Session]\n");
    test_session_init();
    test_session_add_participant();
    test_session_add_proposal();
    test_session_vote();
    test_session_vote_invalid();
    test_session_convergence_two();
    test_session_convergence_already_consensus();
    test_session_empty();
    test_session_multidim();
    test_session_max_participants();
    test_session_convergence_weighted();

    printf("\n[Coalition]\n");
    test_coalition_one_group();
    test_coalition_two_groups();
    test_coalition_empty();
    test_coalition_strength();
    test_coalition_merge();
    test_coalition_center();
    test_coalition_single_participant();
    test_coalition_merge_noop();

    printf("\n[Convergence]\n");
    test_convergence_rate_positive();
    test_convergence_rate_zero_rounds();
    test_convergence_rate_one_round();
    test_convergence_rate_flat();
    test_predict_consensus();
    test_predict_consensus_fast();

    printf("\n[Dialogue]\n");
    test_dialogue_init();
    test_dialogue_add_statement();
    test_dialogue_add_response();
    test_dialogue_build_tree();
    test_dialogue_find_consensus();
    test_dialogue_no_consensus();
    test_dialogue_root_is_consensus();
    test_dialogue_max_nodes();
    test_dialogue_deep_chain();

    printf("\n[Palaver API]\n");
    test_palaver_api_init();
    test_palaver_api_run();
    test_palaver_api_coalitions();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
