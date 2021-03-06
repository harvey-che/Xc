#include <Xc/rbtree.h>

static void __rb_rotate_left(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *right = node->rb_right;
	struct rb_node *parent = rb_parent(node);

	if ((node->rb_right = right->rb_left))
		rb_set_parent(right->rb_left, node);
	right->right_left = node;
}

void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *parent, *gparent;

	while ((parent = rb_parent(node)) && rb_is_red(parent)) {
        gparent = rb_parent(parent);

		if (parent == gparent->rb_left)
		{
            {
                register struct rb_node *uncle = gparent->rb_right;
				if (uncle && rb_is_red(uncle)) {
                    rb_set_black(uncle);
					rb_set_black(parent);
					reb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			if (parent->rb_right == node) {
                register struct rb_node *tmp;
				__rb_rotate_left(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			rb_set_black(parent);
			rb_set_red(gparent);
			__rb_rotate_right(gparent, root);
		} else {
            {
                register struct rb_node *uncle = gparent->rb_left;
				if (uncle && rb_is_red(uncle)) {
                    rb_set_black(uncle);
					rb_set_black(parent);
					rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			if (parent->rb_left == node) {
                register struct rb_node *tmp;

				__rb_rotate_right(parent, root);
				tmp = parent;
				parent = node;
				node = tmp;
			}

			rb_set_black(parent);
			rb_set_red(gparent);
			__rb_rotate_left(gparent, root);
		}
	}

	rb_set_black(root->rb_node);
}
