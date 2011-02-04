/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>
  (C) 2002  David Woodhouse <dwmw2@infradead.org>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  linux/lib/rbtree.c
*/

#ifndef LHI_DISABLE_RBTREE
#include "privlibhashish.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#include "rbtree.h"
#include "threads.h"


struct lhi_rb_entry {
	struct rb_node node;
	uint32_t keylen;
	const void *key;
	const void *data;
};


static struct lhi_rb_entry* lhi_rb_entry_new(const void *k, const void *d, uint32_t keylen)
{
	struct lhi_rb_entry *node_new = malloc(sizeof(*node_new));
	if (!node_new)
		return NULL;

	node_new->keylen = keylen;
	node_new->key = k;
	node_new->data = d;

	return node_new;
}


static void __rb_rotate_left(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *right = node->rb_right;
	struct rb_node *parent = rb_parent(node);

	if ((node->rb_right = right->rb_left))
		rb_set_parent(right->rb_left, node);
	right->rb_left = node;

	rb_set_parent(right, parent);

	if (parent)
	{
		if (node == parent->rb_left)
			parent->rb_left = right;
		else
			parent->rb_right = right;
	}
	else
		root->rb_node = right;
	rb_set_parent(node, right);
}

static void __rb_rotate_right(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *left = node->rb_left;
	struct rb_node *parent = rb_parent(node);

	if ((node->rb_left = left->rb_right))
		rb_set_parent(left->rb_right, node);
	left->rb_right = node;

	rb_set_parent(left, parent);

	if (parent)
	{
		if (node == parent->rb_right)
			parent->rb_right = left;
		else
			parent->rb_left = left;
	}
	else
		root->rb_node = left;
	rb_set_parent(node, left);
}


static void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *parent, *gparent;

	while ((parent = rb_parent(node)) && rb_is_red(parent)) {
		gparent = rb_parent(parent);

		if (parent == gparent->rb_left)
		{
			{
				register struct rb_node *uncle = gparent->rb_right;
				if (uncle && rb_is_red(uncle))
				{
					rb_set_black(uncle);
					rb_set_black(parent);
					rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			if (parent->rb_right == node)
			{
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
				if (uncle && rb_is_red(uncle))
				{
					rb_set_black(uncle);
					rb_set_black(parent);
					rb_set_red(gparent);
					node = gparent;
					continue;
				}
			}

			if (parent->rb_left == node)
			{
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

static void __rb_erase_color(struct rb_node *node, struct rb_node *parent,
			     struct rb_root *root)
{
	struct rb_node *other;

	while ((!node || rb_is_black(node)) && node != root->rb_node)
	{
		if (parent->rb_left == node)
		{
			other = parent->rb_right;
			if (rb_is_red(other))
			{
				rb_set_black(other);
				rb_set_red(parent);
				__rb_rotate_left(parent, root);
				other = parent->rb_right;
			}
			if ((!other->rb_left || rb_is_black(other->rb_left)) &&
			    (!other->rb_right || rb_is_black(other->rb_right)))
			{
				rb_set_red(other);
				node = parent;
				parent = rb_parent(node);
			}
			else
			{
				if (!other->rb_right || rb_is_black(other->rb_right))
				{
					struct rb_node *o_left;
					if ((o_left = other->rb_left))
						rb_set_black(o_left);
					rb_set_red(other);
					__rb_rotate_right(other, root);
					other = parent->rb_right;
				}
				rb_set_color(other, rb_color(parent));
				rb_set_black(parent);
				if (other->rb_right)
					rb_set_black(other->rb_right);
				__rb_rotate_left(parent, root);
				node = root->rb_node;
				break;
			}
		}
		else
		{
			other = parent->rb_left;
			if (rb_is_red(other))
			{
				rb_set_black(other);
				rb_set_red(parent);
				__rb_rotate_right(parent, root);
				other = parent->rb_left;
			}
			if ((!other->rb_left || rb_is_black(other->rb_left)) &&
			    (!other->rb_right || rb_is_black(other->rb_right)))
			{
				rb_set_red(other);
				node = parent;
				parent = rb_parent(node);
			}
			else
			{
				if (!other->rb_left || rb_is_black(other->rb_left))
				{
					register struct rb_node *o_right;
					if ((o_right = other->rb_right))
						rb_set_black(o_right);
					rb_set_red(other);
					__rb_rotate_left(other, root);
					other = parent->rb_left;
				}
				rb_set_color(other, rb_color(parent));
				rb_set_black(parent);
				if (other->rb_left)
					rb_set_black(other->rb_left);
				__rb_rotate_right(parent, root);
				node = root->rb_node;
				break;
			}
		}
	}
	if (node)
		rb_set_black(node);
}


static void rb_erase(struct rb_node *node, struct rb_root *root)
{
	struct rb_node *child, *parent;
	int color;

	if (!node->rb_left)
		child = node->rb_right;
	else if (!node->rb_right)
		child = node->rb_left;
	else
	{
		struct rb_node *old = node, *left;

		node = node->rb_right;
		while ((left = node->rb_left) != NULL)
			node = left;
		child = node->rb_right;
		parent = rb_parent(node);
		color = rb_color(node);

		if (child)
			rb_set_parent(child, parent);
		if (parent == old) {
			parent->rb_right = child;
			parent = node;
		} else
			parent->rb_left = child;

		node->rb_parent_color = old->rb_parent_color;
		node->rb_right = old->rb_right;
		node->rb_left = old->rb_left;

		if (rb_parent(old))
		{
			if (rb_parent(old)->rb_left == old)
				rb_parent(old)->rb_left = node;
			else
				rb_parent(old)->rb_right = node;
		} else
			root->rb_node = node;

		rb_set_parent(old->rb_left, node);
		if (old->rb_right)
			rb_set_parent(old->rb_right, node);
		goto color;
	}

	parent = rb_parent(node);
	color = rb_color(node);

	if (child)
		rb_set_parent(child, parent);
	if (parent)
	{
		if (parent->rb_left == node)
			parent->rb_left = child;
		else
			parent->rb_right = child;
	}
	else
		root->rb_node = child;

 color:
	if (color == RB_BLACK)
		__rb_erase_color(child, parent, root);
}


/*
 * lhi_get_rbtree search for a given key and return SUCCESS
 * when found in the hash and FAILURE if not found.
 *
 * @arg hi_handle the hashish handle
 * @arg key the pointer to the key
 * @arg keylen the len of the key in bytes
 * @arg res contains pointer to record if retval is SUCCESS
 * @return SUCCESS if found or FAILURE when not found
 */
int lhi_get_rbtree(const hi_handle_t *hi_handle,
		const void *key, uint32_t keylen, void **res)
{
	uint32_t tree = hi_handle->hash_func(key, keylen) % hi_handle->table_size;
	struct rb_node *tmp_node = hi_handle->eng_rbtree.trees[tree].root.rb_node;
	struct rb_node **rbnode = &tmp_node;

	lhi_pthread_rwlock_rdlock(hi_handle->eng_rbtree.trees[tree].rwlock);
	while (*rbnode) {
		int diff;
		struct lhi_rb_entry *lhi_entry;
		struct rb_node *parent = *rbnode;
                lhi_entry = rb_entry(parent, struct lhi_rb_entry, node);

		diff = hi_handle->key_cmp(key, lhi_entry->key);
		if (diff == 0) {
			*res = (void *) lhi_entry->data;
			lhi_pthread_rwlock_unlock(hi_handle->eng_rbtree.trees[tree].rwlock);
			return SUCCESS;
		}

		if (diff < 0)
			rbnode = &parent->rb_right;
		else
			rbnode = &parent->rb_left;
	}
	lhi_pthread_rwlock_unlock(hi_handle->eng_rbtree.trees[tree].rwlock);
	return HI_ERR_NOKEY;
}


static size_t hi_rbtree_traverse(struct rb_node *rbnode, size_t pos,  struct lhi_bucket_array *a)
{
	struct lhi_rb_entry *lhi_entry;
	struct rb_node *left, *right;
	size_t p = pos;

	left = rbnode->rb_left;
	pos++;
	if (left)
		pos = hi_rbtree_traverse(left, pos, a);
	right = rbnode->rb_right;
	if (right)
		pos = hi_rbtree_traverse(right, pos, a);
	lhi_entry = rb_entry(rbnode, struct lhi_rb_entry, node);
	//printf("save key %s at %u\n", lhi_entry->key, p);
	if (p < a->nmemb) {
		a->data[p] = (void *) lhi_entry->data;
		a->keys[p] = (void *) lhi_entry->key;
		a->keys_length[p] = lhi_entry->keylen;
	}
	return pos;
}


int lhi_rbtree_bucket_to_array(const hi_handle_t *hi_handle, size_t tree, struct lhi_bucket_array *a)
{
	struct rb_node *rbnode;
	size_t max;
	int ret = HI_ERR_NODATA;

	if (hi_handle->table_size < tree)
		return HI_ERR_RANGE;
	rbnode = hi_handle->eng_rbtree.trees[tree].root.rb_node;
	if (!rbnode)
		return HI_ERR_NODATA;

	lhi_pthread_rwlock_rdlock(hi_handle->eng_rbtree.trees[tree].rwlock);
	max = hi_handle->bucket_size[tree];
	if (!max)
		goto out;

	ret = lhi_bucket_array_alloc(a, max);
	if (ret)
		goto out;
	hi_rbtree_traverse(rbnode, 0, a);
	ret = 0;
 out:
	lhi_pthread_rwlock_unlock(hi_handle->eng_rbtree.trees[tree].rwlock);
	return ret;
}


/* like get, but remove from tree */
int lhi_remove_rbtree(hi_handle_t *hi_handle,
		const void *key, uint32_t keylen, void **res)
{
	uint32_t tree = hi_handle->hash_func(key, keylen) % hi_handle->table_size;
	struct rb_root *root;
	struct rb_node **rbnode;

	root = (struct rb_root*) &hi_handle->eng_rbtree.trees[tree].root;
	rbnode = &root->rb_node;
	if (!rbnode)
		return HI_ERR_NOKEY;

	lhi_pthread_rwlock_wrlock(hi_handle->eng_rbtree.trees[tree].rwlock);
	while (*rbnode) {
		int diff;
		struct lhi_rb_entry *lhi_entry;
		struct rb_node *parent = *rbnode;
                lhi_entry = rb_entry(parent, struct lhi_rb_entry, node);

		diff = hi_handle->key_cmp(key, lhi_entry->key);
		if (diff == 0) {
			*res = (void *) lhi_entry->data;
			rb_erase(parent, root);
			free(lhi_entry);
			--hi_handle->bucket_size[tree];
			lhi_pthread_rwlock_unlock(hi_handle->eng_rbtree.trees[tree].rwlock);

			lhi_pthread_mutex_lock(hi_handle->mutex_lock);
			hi_handle->no_objects--;
			lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
			return SUCCESS;
		}

		if (diff < 0)
			rbnode = &parent->rb_right;
		else
			rbnode = &parent->rb_left;
	}
	lhi_pthread_rwlock_unlock(hi_handle->eng_rbtree.trees[tree].rwlock);
	return HI_ERR_NOKEY;
}


/* lhi_insert_array insert a key/data pair into our hashhandle
 *
 * @arg hi_handle the hashish handle
 * @return SUCCESS or a negativ return values in the case of an error
 */
int lhi_insert_rbtree(hi_handle_t *hi_handle, const void *key,
		uint32_t keylen, const void *data)
{
	uint32_t tree = hi_handle->hash_func(key, keylen) % hi_handle->table_size;
	struct rb_root *root = (struct rb_root*)  &hi_handle->eng_rbtree.trees[tree].root;
	struct lhi_rb_entry *node_new;
	struct rb_node **rbnode, *parent = NULL;
	int ret = HI_ERR_DUPKEY;

	rbnode = &root->rb_node;
	if (!rbnode)
		return HI_ERR_INTERNAL;

	lhi_pthread_rwlock_wrlock(hi_handle->eng_rbtree.trees[tree].rwlock);
	while (*rbnode) {
		int diff;
		struct lhi_rb_entry *lhi_entry;
		parent = *rbnode;
                lhi_entry = rb_entry(parent, struct lhi_rb_entry, node);

		diff = hi_handle->key_cmp(key, lhi_entry->key);
		if (diff == 0)
			goto out;
		if (diff < 0)
			rbnode = &parent->rb_right;
		else
			rbnode = &parent->rb_left;
	}

	ret = HI_ERR_SYSTEM;
	node_new = lhi_rb_entry_new(key, data, keylen);
	if (!node_new)
		goto out;

	rb_link_node(&node_new->node, parent, rbnode);
	rb_insert_color(&node_new->node, root);
	hi_handle->bucket_size[tree]++;
	ret = SUCCESS;
 out:
	lhi_pthread_rwlock_unlock(hi_handle->eng_rbtree.trees[tree].rwlock);
	if (ret == SUCCESS) { /* silly, but what can we do? 8-/ */
		lhi_pthread_mutex_lock(hi_handle->mutex_lock);
		hi_handle->no_objects++;
		lhi_pthread_mutex_unlock(hi_handle->mutex_lock);
	}
	return ret;
}


int lhi_fini_rbtree(hi_handle_t *hi_handle)
{
	unsigned int i, size;

	size = hi_handle->table_size;
	hi_handle->table_size = 0;

	for (i=0; i < size ; i++) {
		lhi_pthread_rwlock_wrlock(hi_handle->eng_rbtree.trees[i].rwlock);
		/* make sure noone accesses this */
		lhi_pthread_rwlock_unlock(hi_handle->eng_rbtree.trees[i].rwlock);
		lhi_pthread_rwlock_destroy(hi_handle->eng_rbtree.trees[i].rwlock);
	}
	free(hi_handle->eng_rbtree.trees);
	hi_handle->eng_rbtree.trees = NULL;
	return SUCCESS;
}
#endif /* LHI_DISABLE_RBTREE */
