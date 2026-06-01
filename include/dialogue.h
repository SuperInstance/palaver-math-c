#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stddef.h>

#define DIALOGUE_MAX_CHILDREN 64
#define DIALOGUE_MAX_NODES 256

typedef struct DialogueNode {
    int id;
    char statement[256];
    int speaker_id;
    int response_ids[DIALOGUE_MAX_CHILDREN];
    int num_responses;
    int is_consensus;  /* 1 if this node represents agreement */
    struct DialogueNode *parent;
} DialogueNode;

typedef struct {
    DialogueNode nodes[DIALOGUE_MAX_NODES];
    int num_nodes;
} DialogueTree;

/* Initialize a dialogue tree. */
void dialogue_tree_init(DialogueTree *tree);

/* Add a flat statement (returns node id). */
int dialogue_add_statement(DialogueTree *tree, const char *statement,
                           int speaker_id, int is_consensus);

/* Set parent-child relationship. child_id responds to parent_id. */
int dialogue_add_response(DialogueTree *tree, int parent_id, int child_id);

/* Build a tree from flat arrays of statements, speaker_ids, parent_ids.
   parent_ids[0] should be -1 for root. */
int dialogue_build_tree(DialogueTree *tree, const char **statements,
                        const int *speaker_ids, const int *parent_ids,
                        const int *is_consensus, int n);

/* BFS to find shortest path to a consensus node. Returns node id and fills path.
   path_out must be sized DIALOGUE_MAX_NODES. Returns path length, or -1 if none. */
int find_consensus_path(DialogueTree *tree, int *path_out, int *path_len);

#endif /* DIALOGUE_H */
