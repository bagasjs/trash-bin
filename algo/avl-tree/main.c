#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <raylib.h>
#include <raymath.h>

typedef struct Node Node;

Node *node_lhs(Node *);
Node *node_rhs(Node *);
int node_height(Node *n);
const char *node_display(Node *node);

#define WHEEL_SENSITIVITY 4
#define NODE_RADIUS 20
#define EDGE_X_MULT 20
#define EDGE_Y_MULT 8

void draw_node_rec_inner(Node *root, Vector2 pos, int level) 
{
    Vector2 offset = (Vector2){ 
        .x = NODE_RADIUS * EDGE_X_MULT * 1.0f/(float)(level * 2), 
        .y = NODE_RADIUS * EDGE_Y_MULT 
    };
    if(node_lhs(root)) {
        Vector2 lhs_pos = Vector2Add(pos, Vector2Multiply(offset, (Vector2){ -1, 1 }));
        DrawLineV(pos, lhs_pos, WHITE);
        draw_node_rec_inner(node_lhs(root), lhs_pos, level + 1);
    }

    if(node_rhs(root)) {
        Vector2 rhs_pos = Vector2Add(pos, offset);
        DrawLineV(pos, rhs_pos, WHITE);
        draw_node_rec_inner(node_rhs(root), rhs_pos, level + 1);
    }

    DrawCircleV(pos, NODE_RADIUS, RED);
    DrawText(node_display(root), pos.x + NODE_RADIUS + 5, pos.y - NODE_RADIUS, NODE_RADIUS, WHITE);
}

void draw_node_rec(Node *root, Vector2 pos)
{
    draw_node_rec_inner(root, pos, 1);
}

void display_tree_in_window(Node *root)
{
    // SetConfigFlags(FLAG_WINDOW_TOPMOST);
    InitWindow(800, 600, "Visualizing Tree");

    Camera2D camera = {0};
    camera.target   = (Vector2){ .x = -100, .y = -100 };
    camera.zoom     = 1.0f;
    camera.rotation = 0.0f;

    Vector2 drag_start = {0};
    bool is_dragging = false;
    Vector2 camera_offset_on_drag_start = {0};

    while(!WindowShouldClose()) {
        float wheel = GetMouseWheelMove();
        Vector2 cursor = GetMousePosition();

        if(!is_dragging && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            is_dragging = true;
            drag_start  = cursor;
            camera_offset_on_drag_start = camera.offset;
        }

        if(is_dragging) {
            Vector2 drag_offset = Vector2Subtract(cursor, drag_start);
            camera.offset = Vector2Add(camera_offset_on_drag_start, drag_offset);

            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                is_dragging = false;
            }
        }
#if 0
        if(IsKeyDown(KEY_LEFT_CONTROL)) {
            camera.zoom += wheel/10;
        } else if(IsKeyDown(KEY_LEFT_SHIFT)) {
            camera.offset.x += wheel * 10 * WHEEL_SENSITIVITY;
        } else {
            camera.offset.y += wheel * 10 * WHEEL_SENSITIVITY;
        }
#else
        camera.zoom += wheel/10;
#endif

        BeginDrawing();
        {
            ClearBackground(BLACK);
            BeginMode2D(camera);
            {
                draw_node_rec(root, (Vector2){ 0, 0 });
            }
            EndMode2D();
        }
        EndDrawing();
    }
    CloseWindow();
}

struct Node {
    Node *lhs;
    Node *rhs;

    int key;
    int height;
};

int node_height(Node *n)
{
    return n ? n->height : 0;
}

int node_balance(Node *n)
{
    return node_height(n->rhs) - node_height(n->lhs);
}

Node *node_lhs(Node *n)
{
    return n->lhs;
}

Node *node_rhs(Node *n)
{
    return n->rhs;
}

const char *node_display(Node *n)
{
    return TextFormat("%d [h=%d] [b=%d]", n->key, node_height(n), node_balance(n));
}

#define NODE_POOL_CAP 1024
typedef struct {
    Node     pool[NODE_POOL_CAP];
    uint32_t alloted;

    Node *root;
} Tree;

Node *tree_new_node(Tree *tree, int key)
{
    assert(tree->alloted < NODE_POOL_CAP);
    Node *node = &tree->pool[tree->alloted++];
    node->key = key;
    node->height = 0;
    node->lhs = 0;
    node->rhs = 0;
    node->height = 0;
    return node;
}

Node *tree_find(Tree *tree, uint64_t value)
{
    Node *curr = tree->root;
    for(;;) {
        if(value < curr->key) {
            if(!curr->lhs) break;
            curr = curr->lhs;
        } else if(value > curr->key) {
            if(!curr->rhs) break;
            curr = curr->rhs;
        } else {
            // equal
            return curr;
        }
    }
    return NULL;
}

int node_update_height(Node *node)
{
    if(!node) return 0;
    int lh = node_update_height(node->lhs);
    int rh = node_update_height(node->rhs);
    node->height = 1 + (lh > rh ? lh : rh);
    return node->height;
}

Node *tree_insert_inner(Tree *tree, Node *parent, Node *child)
{
    if(child->key < parent->key) {
        if(parent->lhs) {
            parent->lhs = tree_insert_inner(tree, parent->lhs, child);
        } else {
            parent->lhs = child;
        }
    } else if(child->key > parent->key) {
        if(parent->rhs) {
            parent->rhs = tree_insert_inner(tree, parent->rhs, child);
        } else {
            parent->rhs = child;
        }
    } else {
        assert(child->key != parent->key);
        assert(0 && "unreachable wtf");
    }
    int lh = node_height(parent->lhs);
    int rh = node_height(parent->rhs);
    parent->height = 1 + (lh > rh ? lh : rh);

    Node *a = parent;
    int a_bal = node_balance(a);
    if(a_bal > 1) {
        Node *b = a->rhs;
        assert(b);

        int b_bal = node_balance(b);
        if(b_bal > 0) {
            parent = b;
            a->rhs = b->lhs;
            b->lhs = a;
        } else if(b_bal < 0) {
            Node *c = b->lhs;
            assert(c);
            parent = c;
            a->rhs = c->lhs;
            b->lhs = c->rhs;
            c->lhs = a;
            c->rhs = b;
        } else {
            assert(0 && "unreachable wtf");
        }
        node_update_height(parent);
    } else if (a_bal < -1) {
        Node *b = a->lhs;
        assert(b);

        int b_bal = node_balance(b);
        if(b_bal < 0) {
            parent = b;
            a->lhs = b->rhs;
            b->rhs = a;
        } else if(b_bal > 0) {
            Node *c = b->rhs;
            assert(c);
            parent = c;
            b->rhs = c->lhs;
            a->lhs = c->rhs;
            c->lhs = b;
            c->rhs = a;
        } else {
            assert(0 && "unreachable wtf");
        }
        node_update_height(parent);
    }
    return parent;
}

void tree_insert(Tree *tree, Node *node)
{
    if(tree->root == NULL) {
        tree->root = node;
        return;
    }
    node->height = 1;
    tree->root   = tree_insert_inner(tree, tree->root, node);
}

int main(void)
{
    Tree tree = {0};

    for(int i = 0; i < 20; i += 2) {
        tree_insert(&tree, tree_new_node(&tree, i));
    }
    for(int i = 19; i >= 1; i -= 2) {
        tree_insert(&tree, tree_new_node(&tree, i));
    }

    display_tree_in_window(tree.root);
    return 0;
}
