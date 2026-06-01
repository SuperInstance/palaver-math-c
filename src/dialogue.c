#include "dialogue.h"
#include <string.h>

void dialogue_tree_init(DialogueTree *tree) {
    memset(tree, 0, sizeof(*tree));
}

int dialogue_add_statement(DialogueTree *tree, const char *statement,
                           int speaker_id, int is_consensus) {
    if (tree->num_nodes >= DIALOGUE_MAX_NODES) return -1;
    DialogueNode *node = &tree->nodes[tree->num_nodes];
    node->id = tree->num_nodes;
    strncpy(node->statement, statement, sizeof(node->statement) - 1);
    node->statement[sizeof(node->statement) - 1] = '\0';
    node->speaker_id = speaker_id;
    node->is_consensus = is_consensus;
    node->parent = NULL;
    node->num_responses = 0;
    tree->num_nodes++;
    return node->id;
}

int dialogue_add_response(DialogueTree *tree, int parent_id, int child_id) {
    if (parent_id < 0 || parent_id >= tree->num_nodes) return -1;
    if (child_id < 0 || child_id >= tree->num_nodes) return -1;
    DialogueNode *p = &tree->nodes[parent_id];
    if (p->num_responses >= DIALOGUE_MAX_CHILDREN) return -1;
    p->response_ids[p->num_responses++] = child_id;
    tree->nodes[child_id].parent = p;
    return 0;
}

int dialogue_build_tree(DialogueTree *tree, const char **statements,
                        const int *speaker_ids, const int *parent_ids,
                        const int *is_consensus_arr, int n) {
    dialogue_tree_init(tree);
    for (int i = 0; i < n; i++) {
        int id = dialogue_add_statement(tree, statements[i], speaker_ids[i],
                                        is_consensus_arr[i]);
        if (id < 0) return -1;
        if (parent_ids[i] >= 0) {
            dialogue_add_response(tree, parent_ids[i], id);
        }
    }
    return tree->num_nodes;
}

int find_consensus_path(DialogueTree *tree, int *path_out, int *path_len) {
    if (tree->num_nodes == 0) { *path_len = 0; return -1; }

    /* BFS from root (node 0) */
    int queue[DIALOGUE_MAX_NODES];
    int visited[DIALOGUE_MAX_NODES];
    int prev[DIALOGUE_MAX_NODES];
    memset(visited, 0, sizeof(visited));
    memset(prev, -1, sizeof(prev));

    int front = 0, back = 0;
    queue[back++] = 0;
    visited[0] = 1;

    while (front < back) {
        int cur = queue[front++];
        DialogueNode *node = &tree->nodes[cur];

        if (node->is_consensus) {
            /* Reconstruct path */
            int path[DIALOGUE_MAX_NODES];
            int len = 0;
            int p = cur;
            while (p >= 0) {
                path[len++] = p;
                p = prev[p];
            }
            /* Reverse into path_out */
            for (int i = 0; i < len; i++) {
                path_out[i] = path[len - 1 - i];
            }
            *path_len = len;
            return cur;
        }

        for (int i = 0; i < node->num_responses; i++) {
            int child = node->response_ids[i];
            if (!visited[child]) {
                visited[child] = 1;
                prev[child] = cur;
                queue[back++] = child;
            }
        }
    }

    *path_len = 0;
    return -1; /* no consensus node found */
}
